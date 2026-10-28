#include "arverb/Reverb.hpp"
#include <algorithm>
#include <array>
#include <cstdint>
#include <cstdio>
#include <cstring>
#include <vector>

// Minimal 32-bit float WAV IR exporter to audition with Audacity/Reaper.
static void u16(FILE* f, std::uint16_t x) {
    unsigned char b[2] = {static_cast<unsigned char>(x), static_cast<unsigned char>(x >> 8)};
    std::fwrite(b, 1, 2, f);
}
static void u32(FILE* f, std::uint32_t x) {
    unsigned char b[4] = {static_cast<unsigned char>(x), static_cast<unsigned char>(x >> 8),
                          static_cast<unsigned char>(x >> 16), static_cast<unsigned char>(x >> 24)};
    std::fwrite(b, 1, 4, f);
}
int main(int argc, char** argv) {
    const char* filename = (argc > 1) ? argv[1] : "arverb_impulse.wav";
    constexpr unsigned fs = 48000;
    constexpr std::size_t n = fs * 5;
    arverb::Engine engine;
    if (argc > 2) {
        if (std::strcmp(argv[2], "plate") == 0) engine.loadPreset(arverb::Preset::VocalPlate);
        if (std::strcmp(argv[2], "chamber") == 0) engine.loadPreset(arverb::Preset::VocalChamber);
        if (std::strcmp(argv[2], "hall") == 0) engine.loadPreset(arverb::Preset::MusicHall);
        if (std::strcmp(argv[2], "ballad") == 0) engine.loadPreset(arverb::Preset::AiryBallad);
    }
    engine.setParameter(arverb::Parameter::Mix, 1.0f);
    engine.setParameter(arverb::Parameter::DuckAmount, 0.0f);
    engine.prepare(fs);
    std::vector<float> inL(n, 0), inR(n, 0), outL(n), outR(n);
    inL[0] = 0.9f;
    inR[0] = 0.9f;
    for (std::size_t offset = 0; offset < n; offset += 256) {
        const auto chunk = std::min<std::size_t>(256, n - offset);
        engine.process(inL.data() + offset, inR.data() + offset,
                       outL.data() + offset, outR.data() + offset, chunk);
    }
    FILE* f = std::fopen(filename, "wb");
    if (!f) { std::perror(filename); return 1; }
    const std::uint32_t dataBytes = static_cast<std::uint32_t>(n * 2 * sizeof(float));
    std::fwrite("RIFF", 1, 4, f); u32(f, 36 + dataBytes);
    std::fwrite("WAVEfmt ", 1, 8, f); u32(f, 16);
    u16(f, 3); u16(f, 2); u32(f, fs); u32(f, fs * 8); u16(f, 8); u16(f, 32);
    std::fwrite("data", 1, 4, f); u32(f, dataBytes);
    for (std::size_t i = 0; i < n; ++i) {
        std::fwrite(&outL[i], sizeof(float), 1, f);
        std::fwrite(&outR[i], sizeof(float), 1, f);
    }
    std::fclose(f);
    std::printf("Wrote %s (5 sec, 48k stereo float WAV)\n", filename);
}
