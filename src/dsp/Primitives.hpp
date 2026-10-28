#pragma once
// Private DSP primitives. Non-platform, allocation-free processing interfaces.
#include <algorithm>
#include <cmath>
#include <cstddef>
#include <vector>

namespace arverb::detail {
constexpr float kPi = 3.14159265358979323846f;
inline float clamp(float x, float lo, float hi) noexcept {
    return std::max(lo, std::min(hi, x));
}

inline float killDenormal(float x) noexcept {
    return (std::fabs(x) < 1.0e-20f) ? 0.0f : x;
}

inline float lowPassCoeff(float cutoff, float sampleRate) noexcept {
    // One pole: avoids tan() in hot path; cutoff constrained below Nyquist.
    cutoff = clamp(cutoff, 10.0f, sampleRate * 0.44f);
    return 1.0f - std::exp(-2.0f * kPi * cutoff / sampleRate);
}

struct Delay {
    std::vector<float> data;
    std::size_t mask = 0;
    std::size_t write = 0;

    void prepare(std::size_t maxDelaySamples) {
        std::size_t n = 8;
        while (n < maxDelaySamples + 8) n <<= 1;
        data.assign(n, 0.0f);
        mask = n - 1;
        write = 0;
    }
    void reset() noexcept {
        std::fill(data.begin(), data.end(), 0.0f);
        write = 0;
    }
    float read(float samples) const noexcept {
        // read-before-push: delay 1.0 means previous sample; min requested >=1.
        samples = clamp(samples, 1.0f, static_cast<float>(data.size() - 4));
        const auto integer = static_cast<std::size_t>(samples);
        const float fraction = samples - static_cast<float>(integer);
        const std::size_t a = (write - integer) & mask;
        const std::size_t b = (a - 1) & mask;
        return data[a] + (data[b] - data[a]) * fraction;
    }
    void push(float x) noexcept {
        data[write] = x;
        write = (write + 1) & mask;
    }
};

struct Allpass {
    Delay delay;
    float samples = 1.0f;
    void prepare(float ms, float rate) {
        samples = std::max(1.0f, ms * 0.001f * rate);
        delay.prepare(static_cast<std::size_t>(samples) + 2);
    }
    void reset() noexcept { delay.reset(); }
    float process(float x, float g) noexcept {
        const float d = delay.read(samples);
        const float y = d - g * x;
        delay.push(killDenormal(x + g * y));
        return y;
    }
};

struct LowPass {
    float z = 0.0f;
    float process(float x, float coeff) noexcept {
        z = killDenormal(z + coeff * (x - z));
        return z;
    }
    void reset() noexcept { z = 0.0f; }
};

struct HighPass {
    LowPass low;
    float process(float x, float coeff) noexcept {
        return x - low.process(x, coeff);
    }
    void reset() noexcept { low.reset(); }
};

struct Oscillator {
    float sinPhase = 0.0f;
    float cosPhase = 1.0f;
    float sinStep = 0.0f;
    float cosStep = 1.0f;
    unsigned count = 0;
    void setFrequency(float hertz, float rate) noexcept {
        const float omega = 2.0f * kPi * hertz / rate;
        sinStep = std::sin(omega);
        cosStep = std::cos(omega);
    }
    void reset(float phase) noexcept {
        sinPhase = std::sin(phase);
        cosPhase = std::cos(phase);
        count = 0;
    }
    void tick() noexcept {
        const float s = sinPhase * cosStep + cosPhase * sinStep;
        const float c = cosPhase * cosStep - sinPhase * sinStep;
        sinPhase = s;
        cosPhase = c;
        // Prevent recursive-oscillator amplitude drift. Rarely in the hot path.
        if ((++count & 8191u) == 0u) {
            const float norm = 1.0f / std::sqrt(s * s + c * c + 1.0e-20f);
            sinPhase *= norm;
            cosPhase *= norm;
        }
    }
};

}  // namespace arverb::detail
