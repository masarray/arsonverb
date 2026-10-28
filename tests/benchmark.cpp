#include "arverb/Reverb.hpp"
#include <chrono>
#include <cmath>
#include <cstdio>

int main() {
    arverb::Engine verb;
    verb.prepare(48000.0);
    verb.loadPreset(arverb::Preset::MusicHall);
    constexpr int block = 256;
    constexpr int blocks = 48000 * 30 / block; // 30 seconds of audio, offline
    float inL[block]{}, inR[block]{}, outL[block]{}, outR[block]{};
    for (int i = 0; i < block; ++i) {
        inL[i] = 0.1f * std::sin(0.035f * i);
        inR[i] = 0.1f * std::cos(0.027f * i);
    }
    const auto start = std::chrono::steady_clock::now();
    for (int i = 0; i < blocks; ++i)
        verb.process(inL, inR, outL, outR, block);
    const auto stop = std::chrono::steady_clock::now();
    const double elapsed = std::chrono::duration<double>(stop - start).count();
    const double audioSeconds = static_cast<double>(blocks * block) / 48000.0;
    std::printf("30s input at 48kHz: %.4f s processing, %.1fx faster than realtime (this machine/build).\n",
                elapsed, audioSeconds / elapsed);
    return 0;
}
