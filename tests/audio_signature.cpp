// P2: deterministic stereo sample fingerprint; no IO in Engine::process.
// The exact same test is compiled against the parent P1 engine in Linux CI.
#include <arverb/Reverb.hpp>
#include <algorithm>
#include <cmath>
#include <cstdint>
#include <cstdio>
#include <cstring>
#include <vector>

static void feed(std::uint64_t& hash, float value) noexcept {
    std::uint32_t bits = 0;
    std::memcpy(&bits, &value, sizeof(bits));
    for (int b = 0; b < 4; ++b) {
        hash ^= (bits >> (b * 8)) & 0xffu;
        hash *= UINT64_C(1099511628211);
    }
}

int main() {
    for (int fs : {48000, 96000, 192000}) {
        for (auto preset : {arverb::Preset::VocalPlate,
                            arverb::Preset::MusicHall,
                            arverb::Preset::SlowCinematicWash}) {
            for (int block : {1, 32, 257}) {
                arverb::Engine engine;
                engine.loadPreset(preset);
                engine.prepare(fs);
                std::vector<float> inL(static_cast<std::size_t>(block));
                std::vector<float> inR(static_cast<std::size_t>(block));
                std::vector<float> outL(static_cast<std::size_t>(block));
                std::vector<float> outR(static_cast<std::size_t>(block));
                std::uint64_t hash = UINT64_C(14695981039346656037);
                constexpr int total = 8192;
                for (int offset = 0, cb = 0; offset < total; ++cb) {
                    const int frames = std::min(block, total - offset);
                    if (cb % 5 == 0) {
                        const bool a = (cb / 5) % 2 != 0;
                        engine.setParameter(arverb::Parameter::DecaySeconds, a ? 1.25f : 9.0f);
                        engine.setParameter(arverb::Parameter::ModRateHz, a ? 0.16f : 1.77f);
                        engine.setParameter(arverb::Parameter::DampingHz, a ? 1250.0f : 17000.0f);
                        engine.setParameter(arverb::Parameter::LowCutHz, a ? 40.0f : 900.0f);
                        engine.setParameter(arverb::Parameter::HighCutHz, a ? 1200.0f : 18000.0f);
                        engine.setParameter(arverb::Parameter::Mix, a ? 0.21f : 0.87f);
                        engine.setParameter(arverb::Parameter::DuckAmount, a ? 0.02f : 0.96f);
                    }
                    for (int i = 0; i < frames; ++i) {
                        const float t = static_cast<float>(offset + i);
                        inL[i] = 0.28f * std::sin(0.041f * t) + 0.07f * std::cos(0.193f * t);
                        inR[i] = 0.32f * std::sin(0.034f * t + 0.31f);
                    }
                    engine.process(inL.data(), inR.data(), outL.data(), outR.data(),
                                   static_cast<std::size_t>(frames));
                    for (int i = 0; i < frames; ++i) {
                        feed(hash, outL[i]);
                        feed(hash, outR[i]);
                    }
                    offset += frames;
                }
                std::printf("%d,%u,%d,%016llx\n", fs, static_cast<unsigned>(preset), block,
                            static_cast<unsigned long long>(hash));
            }
        }
    }
    return 0;
}
