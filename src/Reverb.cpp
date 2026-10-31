#include "arverb/Reverb.hpp"
#include "dsp/Primitives.hpp"

#include <algorithm>
#include <array>
#include <cmath>
#include <limits>
#include <stdexcept>
#include <vector>

namespace arverb {
namespace {
constexpr float kLn001 = 6.907755278982137f;  // ln(1000), for RT60
constexpr std::size_t kLines = 8;

constexpr ParameterInfo kParameterInfo[parameterCount] = {
    {"mix",       "Mix",         "",    0.0f,    1.0f,    0.19f},
    {"predelay",  "Pre-delay",   "ms",  0.0f,  250.0f,   36.0f},
    {"decay",     "Decay RT60",  "s",   0.15f,  12.0f,    2.00f},
    {"size",      "Size",        "",    0.65f,   2.0f,    1.0f},
    {"diffusion", "Diffusion",   "",    0.0f,    1.0f,    0.66f},
    {"damping",   "HF damping",  "Hz", 800.0f,18000.0f, 6300.0f},
    {"lowcut",    "Wet low cut", "Hz",  30.0f, 1200.0f,  220.0f},
    {"highcut",   "Wet high cut","Hz",1000.0f,20000.0f,10000.0f},
    {"moddepth",  "Mod depth",   "ms",  0.0f,    1.2f,    0.18f},
    {"modrate",   "Mod rate",    "Hz",  0.05f,   2.0f,    0.32f},
    {"width",     "Wet width",   "",    0.0f,    1.6f,    1.08f},
    {"early",     "Early level", "",    0.0f,    1.0f,    0.30f},
    {"duck",      "Duck amount", "",    0.0f,    1.0f,    0.33f}
};

constexpr float kDelayMs[kLines] = {30.13f, 34.37f, 39.19f, 43.73f,
                                   47.53f, 52.87f, 59.17f, 66.41f};
constexpr float kLfoRatio[kLines] = {0.77f, 1.12f, 0.69f, 1.31f,
                                    0.94f, 1.49f, 0.83f, 1.24f};
// Fixed orthogonal-ish injection and readout patterns, distinct per channel.
constexpr float kInjectL[kLines] = {1, 1, 1, 1,-1,-1,-1,-1};
constexpr float kInjectR[kLines] = {1, 1,-1,-1, 1, 1,-1,-1};
constexpr float kReadL[kLines]   = {1,-1, 1, 1,-1, 1,-1,-1};
constexpr float kReadR[kLines]   = {1, 1,-1, 1,-1,-1, 1,-1};
constexpr float kReflect[kLines] = {1,-1, 1,-1, 1,-1, 1,-1};
constexpr float kDiffuseMs[2][4] = {
    {4.73f, 8.81f, 13.17f, 20.93f},
    {5.27f, 9.41f, 14.63f, 22.19f}
};
constexpr float kEarlyMs[6] = {14.9f, 23.3f, 37.7f, 55.1f, 83.3f, 126.1f};
constexpr float kEarlyGain[6] = {0.34f, 0.29f, 0.23f, 0.18f, 0.14f, 0.12f};

// Additional low-amplitude, irregularly spaced *early energy* taps. This is
// one original density layer, not a ray-traced room model or Valhalla code.
// The old 6-tap sound remains untouched below Diffusion=0.62. Delay storage
// stays inside the existing 132ms early buffers; no new allocations.
constexpr float kDenseEarlyMs[6]   = {18.7f, 31.1f, 46.9f, 68.3f, 95.7f, 117.3f};
constexpr float kDenseEarlyGain[6] = {0.15f,-0.12f, 0.10f,-0.08f, 0.06f,-0.05f};

constexpr float bassDecayCutoff(float lowCutHz) noexcept {
    // Subtle recirculating low-frequency absorption, NOT an extra wet HPF.
    return 85.0f + 0.14f * lowCutHz;
}
constexpr float bassDecayDepth(float lowCutHz) noexcept {
    // 3..10% low-band reduction per pass; small at normal vocal settings.
    return 0.03f + 0.07f * (lowCutHz / 1200.0f);
}

using detail::Delay;
using detail::Allpass;
using detail::LowPass;
using detail::HighPass;
using detail::Oscillator;
using detail::killDenormal;
using detail::clamp;
using detail::lowPassCoeff;

}  // namespace

const ParameterInfo& parameterInfo(Parameter parameter) noexcept {
    const auto i = static_cast<std::size_t>(parameter);
    return kParameterInfo[i < parameterCount ? i : 0];
}

struct Engine::State {
    float sampleRate = 0.0f;
    float smoother = 0.0f;
    float envelopeAttack = 0.0f;
    float envelopeRelease = 0.0f;
    Delay pre[2];
    Delay early[2];
    Allpass diffuser[2][4];
    Delay lines[kLines];
    // Immutable with respect to each prepare() call: avoid invariant arithmetic
    // in the sample loop without introducing per-parameter cache invalidation.
    float earlySampleL[6]{};
    float earlySampleR[6]{};
    float denseEarlySampleL[6]{};
    float denseEarlySampleR[6]{};
    float fdnBaseSamples[kLines]{};
    float millisecondsToSamples = 0.0f;
    Oscillator lfo[kLines];
    LowPass feedbackLP[kLines];
    detail::BassDecayContour feedbackBass[kLines];
    HighPass wetHP[2][2];
    LowPass wetLP[2][2];
    detail::PresenceDucker presence;
    std::array<float, parameterCount> smooth{};
    float gains[kLines]{};
    float targetGains[kLines]{};
    float cachedDecay = -1.0f;
    float cachedSize = -1.0f;
    float cachedRate = -1.0f;
    float cachedDamping = -1.0f;
    float cachedLowCut = -1.0f;
    float cachedHighCut = -1.0f;
    float targetHF = 0.0f;
    float targetLow = 0.0f;
    float targetHigh = 0.0f;
    float targetBassCoeff = 0.0f;
    float targetBassDepth = 0.0f;
    float bassCoeff = 0.0f;
    float bassDepth = 0.0f;
    float duckEnvelope = 0.0f;
    float hfFeedbackCoeff = 0.0f;
    float wetHighCoeff = 0.0f;
    float wetLowCoeff = 0.0f;

    void reset() noexcept {
        for (auto& p : pre) p.reset();
        for (auto& e : early) e.reset();
        for (auto& ch : diffuser) for (auto& d : ch) d.reset();
        for (auto& l : lines) l.reset();
        for (auto& f : feedbackLP) f.reset();
        for (auto& f : feedbackBass) f.reset();
        for (auto& ch : wetHP) for (auto& f : ch) f.reset();
        for (auto& ch : wetLP) for (auto& f : ch) f.reset();
        presence.reset();
        for (std::size_t i = 0; i < kLines; ++i) {
            lfo[i].reset(static_cast<float>(i) * 1.7718f + 0.39f);
        }
        duckEnvelope = 0.0f;
    }
};

Engine::Engine() : state_(std::make_unique<State>()) {
    for (std::size_t i = 0; i < parameterCount; ++i)
        parameters_[i].store(kParameterInfo[i].defaultValue, std::memory_order_relaxed);
}

Engine::~Engine() = default;

bool Engine::prepared() const noexcept { return state_->sampleRate > 0.0f; }

void Engine::prepare(double sampleRate) {
    if (!std::isfinite(sampleRate) || sampleRate < 8000.0 || sampleRate > 192000.0)
        throw std::invalid_argument("arverb sample rate must be 8000..192000 Hz");

    State& s = *state_;
    // Allocation is allowed here only, when the host has suspended processing.
    s.sampleRate = static_cast<float>(sampleRate);
    const float fs = s.sampleRate;
    s.millisecondsToSamples = fs * 0.001f;
    for (int t = 0; t < 6; ++t) {
        s.earlySampleL[t] = kEarlyMs[t] * s.millisecondsToSamples;
        s.earlySampleR[t] = s.earlySampleL[t] * (1.0f + 0.019f * (t % 3));
        s.denseEarlySampleL[t] = kDenseEarlyMs[t] * s.millisecondsToSamples;
        s.denseEarlySampleR[t] = s.denseEarlySampleL[t] * (1.0f + 0.014f * (t % 3));
    }
    for (std::size_t i = 0; i < kLines; ++i)
        s.fdnBaseSamples[i] = kDelayMs[i] * s.millisecondsToSamples;
    const auto forMs = [fs](float ms) -> std::size_t {
        return static_cast<std::size_t>(std::ceil(ms * 0.001f * fs)) + 8;
    };
    for (auto& p : s.pre) p.prepare(forMs(252.0f));
    for (auto& e : s.early) e.prepare(forMs(132.0f));
    for (int ch = 0; ch < 2; ++ch) {
        for (int i = 0; i < 4; ++i)
            s.diffuser[ch][i].prepare(kDiffuseMs[ch][i], fs);
    }
    for (std::size_t i = 0; i < kLines; ++i)
        s.lines[i].prepare(forMs(kDelayMs[i] * 2.0f + 3.0f));

    s.smoother = 1.0f - std::exp(-1.0f / (0.025f * fs));
    s.envelopeAttack = 1.0f - std::exp(-1.0f / (0.012f * fs));
    s.envelopeRelease = 1.0f - std::exp(-1.0f / (0.34f * fs));
    s.presence.prepare(fs);
    for (std::size_t i = 0; i < parameterCount; ++i)
        s.smooth[i] = parameters_[i].load(std::memory_order_relaxed);

    const float decay = s.smooth[static_cast<unsigned>(Parameter::DecaySeconds)];
    const float size = s.smooth[static_cast<unsigned>(Parameter::Size)];
    for (std::size_t i = 0; i < kLines; ++i) {
        const float lengthSeconds = kDelayMs[i] * 0.001f * size;
        s.gains[i] = std::exp(-kLn001 * lengthSeconds / decay);
    }
    s.cachedDecay = -1.0f;
    s.cachedSize = -1.0f;
    s.cachedRate = -1.0f;
    s.cachedDamping = -1.0f;
    s.cachedLowCut = -1.0f;
    s.cachedHighCut = -1.0f;
    s.hfFeedbackCoeff = lowPassCoeff(s.smooth[static_cast<unsigned>(Parameter::DampingHz)], fs);
    s.wetLowCoeff = lowPassCoeff(s.smooth[static_cast<unsigned>(Parameter::LowCutHz)], fs);
    s.wetHighCoeff = lowPassCoeff(s.smooth[static_cast<unsigned>(Parameter::HighCutHz)], fs);
    const float initialLowCut = s.smooth[static_cast<unsigned>(Parameter::LowCutHz)];
    s.bassCoeff = s.targetBassCoeff = lowPassCoeff(bassDecayCutoff(initialLowCut), fs);
    s.bassDepth = s.targetBassDepth = bassDecayDepth(initialLowCut);
    reset();
}

void Engine::reset() noexcept { state_->reset(); }

void Engine::setParameter(Parameter parameter, float value) noexcept {
    const auto i = static_cast<std::size_t>(parameter);
    if (i >= parameterCount || !std::isfinite(value)) return;
    const auto& info = kParameterInfo[i];
    parameters_[i].store(clamp(value, info.minimum, info.maximum), std::memory_order_relaxed);
}

float Engine::getParameter(Parameter parameter) const noexcept {
    const auto i = static_cast<std::size_t>(parameter);
    return i < parameterCount ? parameters_[i].load(std::memory_order_relaxed) : 0.0f;
}

void Engine::process(const float* inputLeft, const float* inputRight,
                     float* outputLeft, float* outputRight,
                     std::size_t frames) noexcept {
    State& s = *state_;
    if (!s.sampleRate || !frames || !inputLeft || !inputRight || !outputLeft || !outputRight) return;
    float target[parameterCount];
    for (std::size_t i = 0; i < parameterCount; ++i)
        target[i] = parameters_[i].load(std::memory_order_relaxed);
    const auto idx = [](Parameter p) -> unsigned { return static_cast<unsigned>(p); };

    // Constant work per block: expensive transcendental operations are never per sample.
    // Cache expensive math until the controlling parameter changes. Hosts are free to
    // call process() with tiny blocks; unchanged settings cost no transcendental ops.
    if (target[idx(Parameter::Size)] != s.cachedSize ||
        target[idx(Parameter::DecaySeconds)] != s.cachedDecay) {
        s.cachedSize = target[idx(Parameter::Size)];
        s.cachedDecay = target[idx(Parameter::DecaySeconds)];
        for (std::size_t i = 0; i < kLines; ++i) {
            const float delaySec = kDelayMs[i] * s.cachedSize * 0.001f;
            s.targetGains[i] = std::exp(-kLn001 * delaySec / s.cachedDecay);
        }
    }
    if (target[idx(Parameter::ModRateHz)] != s.cachedRate) {
        s.cachedRate = target[idx(Parameter::ModRateHz)];
        for (std::size_t i = 0; i < kLines; ++i)
            s.lfo[i].setFrequency(s.cachedRate * kLfoRatio[i], s.sampleRate);
    }
    if (target[idx(Parameter::DampingHz)] != s.cachedDamping) {
        s.cachedDamping = target[idx(Parameter::DampingHz)];
        s.targetHF = lowPassCoeff(s.cachedDamping, s.sampleRate);
    }
    if (target[idx(Parameter::LowCutHz)] != s.cachedLowCut) {
        s.cachedLowCut = target[idx(Parameter::LowCutHz)];
        s.targetLow = lowPassCoeff(s.cachedLowCut, s.sampleRate);
        s.targetBassCoeff = lowPassCoeff(bassDecayCutoff(s.cachedLowCut), s.sampleRate);
        s.targetBassDepth = bassDecayDepth(s.cachedLowCut);
    }
    if (target[idx(Parameter::HighCutHz)] != s.cachedHighCut) {
        s.cachedHighCut = target[idx(Parameter::HighCutHz)];
        s.targetHigh = lowPassCoeff(s.cachedHighCut, s.sampleRate);
    }

    // Eight smoothed scalars are actually consumed in process().
    // Decay, modulation rate and the three EQ frequency parameters instead
    // drive independent, correctly smoothed feedback/filter coefficients.
    // Integrating those five unused sample histories cost CPU but had no
    // influence on the audio output. Keep the original audible update order.
    constexpr std::array<unsigned, 8> kSmoothedAudioControls {{
        static_cast<unsigned>(Parameter::Mix),
        static_cast<unsigned>(Parameter::PreDelayMs),
        static_cast<unsigned>(Parameter::Size),
        static_cast<unsigned>(Parameter::Diffusion),
        static_cast<unsigned>(Parameter::ModDepthMs),
        static_cast<unsigned>(Parameter::Width),
        static_cast<unsigned>(Parameter::EarlyLevel),
        static_cast<unsigned>(Parameter::DuckAmount)
    }};
    for (std::size_t frame = 0; frame < frames; ++frame) {
        for (const unsigned p : kSmoothedAudioControls)
            s.smooth[p] += s.smoother * (target[p] - s.smooth[p]);
        const float mix = s.smooth[idx(Parameter::Mix)];
        const float preSamples = s.smooth[idx(Parameter::PreDelayMs)] * s.millisecondsToSamples;
        const float size = s.smooth[idx(Parameter::Size)];
        const float diffusion = s.smooth[idx(Parameter::Diffusion)];
        const float modDepth = s.smooth[idx(Parameter::ModDepthMs)] * s.millisecondsToSamples;
        const float width = s.smooth[idx(Parameter::Width)];
        const float earlyAmount = s.smooth[idx(Parameter::EarlyLevel)];
        const float duckAmount = s.smooth[idx(Parameter::DuckAmount)];
        const float g = 0.15f + diffusion * 0.60f;
        s.hfFeedbackCoeff += s.smoother * (s.targetHF - s.hfFeedbackCoeff);
        s.wetLowCoeff += s.smoother * (s.targetLow - s.wetLowCoeff);
        s.wetHighCoeff += s.smoother * (s.targetHigh - s.wetHighCoeff);
        s.bassCoeff += s.smoother * (s.targetBassCoeff - s.bassCoeff);
        s.bassDepth += s.smoother * (s.targetBassDepth - s.bassDepth);
        // Contain invalid upstream samples before they enter a recursive network.
        // Sanitise malformed host buffers, including finite values close to FLT_MAX.
        // +24 dBFS and beyond is valid; 32.0f preserves substantial headroom
        // while preventing the recursive matrix from reaching inf/NaN.
        const float inL = std::isfinite(inputLeft[frame])
            ? clamp(inputLeft[frame], -32.0f, 32.0f) : 0.0f;
        const float inR = std::isfinite(inputRight[frame])
            ? clamp(inputRight[frame], -32.0f, 32.0f) : 0.0f;
        float preL = inL;
        float preR = inR;
        // Zero-delay does not access an unwritten ring slot.
        if (preSamples >= 1.0f) {
            preL = s.pre[0].read(preSamples);
            preR = s.pre[1].read(preSamples);
        } else if (preSamples > 0.0f) {
            preL = inL * (1.0f - preSamples) + s.pre[0].read(1.0f) * preSamples;
            preR = inR * (1.0f - preSamples) + s.pre[1].read(1.0f) * preSamples;
        }
        s.pre[0].push(inL);
        s.pre[1].push(inR);

        float earlyL = 0.0f, earlyR = 0.0f;
        for (int t = 0; t < 6; ++t) {
            const float a = s.early[0].read(s.earlySampleL[t]);
            const float b = s.early[1].read(s.earlySampleR[t]);
            earlyL += kEarlyGain[t] * (a + 0.10f * b);
            earlyR += kEarlyGain[t] * (b + 0.10f * a);
        }
        // Extend early energy only at high Diffusion. The original first
        // six reflections remain exact for Diffusion <= 0.62. Density rises
        // continuously, avoiding a hard mode switch or added DSP latency.
        const float density = clamp((diffusion - 0.62f) / 0.38f, 0.0f, 1.0f);
        if (density > 0.0f) {
            float extraL = 0.0f, extraR = 0.0f;
            for (int t = 0; t < 6; ++t) {
                const float a = s.early[0].read(s.denseEarlySampleL[t]);
                const float b = s.early[1].read(s.denseEarlySampleR[t]);
                extraL += kDenseEarlyGain[t] * (a + 0.10f * b);
                extraR += kDenseEarlyGain[t] * (b + 0.10f * a);
            }
            earlyL += density * (0.23f * extraL - 0.10f * earlyL);
            earlyR += density * (0.23f * extraR - 0.10f * earlyR);
        }
        s.early[0].push(preL);
        s.early[1].push(preR);

        float diffL = preL * 0.72f + earlyL * 0.28f;
        float diffR = preR * 0.72f + earlyR * 0.28f;
        for (int i = 0; i < 4; ++i) {
            diffL = s.diffuser[0][i].process(diffL, g);
            diffR = s.diffuser[1][i].process(diffR, g);
        }

        float taps[kLines];
        float sumHouseholder = 0.0f;
        float tailL = 0.0f, tailR = 0.0f;
        for (std::size_t i = 0; i < kLines; ++i) {
            const float base = s.fdnBaseSamples[i] * size;
            taps[i] = s.lines[i].read(base + modDepth * s.lfo[i].sinPhase);
            sumHouseholder += kReflect[i] * taps[i];
            tailL += kReadL[i] * taps[i];
            tailR += kReadR[i] * taps[i];
            s.lfo[i].tick();
        }
        // H = I - 2vv'/8. Orthogonal Householder feedback: energy preserving
        // before per-line RT60 and damping; O(N), not an O(N^2) matrix multiply.
        const float householderProjection = 0.25f * sumHouseholder;
        for (std::size_t i = 0; i < kLines; ++i) {
            s.gains[i] += s.smoother * (s.targetGains[i] - s.gains[i]);
            const float mixed = taps[i] - householderProjection * kReflect[i];
            // Frequency-dependent *decay* rather than output-EQ only:
            // gently reduce very-low-frequency recurrence inside the tank,
            // leaving consonant/midband feedback and the dry path untouched.
            const float bassShaped = s.feedbackBass[i].process(
                mixed, s.bassCoeff, s.bassDepth);
            const float damped = s.feedbackLP[i].process(bassShaped, s.hfFeedbackCoeff);
            const float injected = 0.30f * (kInjectL[i] * diffL + kInjectR[i] * diffR);
            s.lines[i].push(killDenormal(s.gains[i] * damped + injected));
        }

        float wetL = (tailL * 0.24f) + earlyL * (0.38f * earlyAmount);
        float wetR = (tailR * 0.24f) + earlyR * (0.38f * earlyAmount);
        wetL = s.wetHP[0][1].process(s.wetHP[0][0].process(wetL, s.wetLowCoeff), s.wetLowCoeff);
        wetR = s.wetHP[1][1].process(s.wetHP[1][0].process(wetR, s.wetLowCoeff), s.wetLowCoeff);
        wetL = s.wetLP[0][1].process(s.wetLP[0][0].process(wetL, s.wetHighCoeff), s.wetHighCoeff);
        wetR = s.wetLP[1][1].process(s.wetLP[1][0].process(wetR, s.wetHighCoeff), s.wetHighCoeff);

        // Presence shaping is a wet-only complement to existing broadband
        // ducking; no lookahead, new parameter, or FDN topology change.
        s.presence.process(inL, inR, wetL, wetR, duckAmount);

        const float mid = 0.5f * (wetL + wetR);
        const float side = 0.5f * (wetL - wetR) * width;
        const float loudness = std::max(std::fabs(inL), std::fabs(inR));
        const float envCoef = loudness > s.duckEnvelope ? s.envelopeAttack : s.envelopeRelease;
        s.duckEnvelope += envCoef * (loudness - s.duckEnvelope);
        const float duckGain = 1.0f / (1.0f + 6.0f * duckAmount * s.duckEnvelope);
        outputLeft[frame] = (1.0f - mix) * inL + mix * duckGain * (mid + side);
        outputRight[frame] = (1.0f - mix) * inR + mix * duckGain * (mid - side);
    }
}

}  // namespace arverb
