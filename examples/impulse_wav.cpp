#include "arverb/Reverb.hpp"
#include <algorithm>
#include <array>
#include <cmath>
#include <cstdint>
#include <cstdio>
#include <cstring>
#include <vector>

// Portable IEEE-754 stereo WAV renderer. OFFLINE ONLY: never linked into
// audio callback or JUCE frontend. Legacy CLI still works:
//   arverb_demo output.wav plate
// P6 QA CLI (fixed, reproducible reference preset):
//   arverb_demo output.wav clean classic|plate|chamber|hall mono|left|right
static void u16(FILE* f, std::uint16_t x) {
    const unsigned char b[2] = {static_cast<unsigned char>(x),
                                static_cast<unsigned char>(x >> 8)};
    std::fwrite(b, 1, 2, f);
}
static void u32(FILE* f, std::uint32_t x) {
    const unsigned char b[4] = {static_cast<unsigned char>(x),
                                static_cast<unsigned char>(x >> 8),
                                static_cast<unsigned char>(x >> 16),
                                static_cast<unsigned char>(x >> 24)};
    std::fwrite(b, 1, 4, f);
}
int main(int argc, char** argv) {
    const char* filename = argc > 1 ? argv[1] : "arverb_impulse.wav";
    constexpr unsigned fs = 48000;
    constexpr std::size_t frames = static_cast<std::size_t>(fs) * 6u;
    arverb::Engine engine;
    const char* preset = argc > 2 ? argv[2] : "plate";
    if (std::strcmp(preset, "clean") == 0)
        engine.loadPreset(arverb::Preset::VocalPlateClean);
    else if (std::strcmp(preset, "plate") == 0)
        engine.loadPreset(arverb::Preset::VocalPlate);
    else if (std::strcmp(preset, "chamber") == 0)
        engine.loadPreset(arverb::Preset::VocalChamber);
    else if (std::strcmp(preset, "hall") == 0)
        engine.loadPreset(arverb::Preset::MusicHall);
    else if (std::strcmp(preset, "ballad") == 0)
        engine.loadPreset(arverb::Preset::AiryBallad);
    else { std::fprintf(stderr, "Unknown preset: %s\n", preset); return 2; }

    // Explicit P6 mode makes each exported WAV exercise the actual selected
    // P4 matrix. Legacy two-argument invocations keep Classic for old tests.
    if (argc > 3) {
        const char* mode = argv[3];
        if (std::strcmp(mode, "classic") == 0)
            engine.setAlgorithm(arverb::Algorithm::Classic);
        else if (std::strcmp(mode, "plate") == 0)
            engine.setAlgorithm(arverb::Algorithm::Plate);
        else if (std::strcmp(mode, "chamber") == 0)
            engine.setAlgorithm(arverb::Algorithm::Chamber);
        else if (std::strcmp(mode, "hall") == 0)
            engine.setAlgorithm(arverb::Algorithm::Hall);
        else { std::fprintf(stderr, "Unknown algorithm: %s\n", mode); return 2; }
    }
    const char* excitation = argc > 4 ? argv[4] : "mono";
    if (std::strcmp(excitation, "mono") != 0 &&
        std::strcmp(excitation, "left") != 0 &&
        std::strcmp(excitation, "right") != 0) {
        std::fprintf(stderr, "Unknown excitation: %s\n", excitation);
        return 2;
    }
    engine.setParameter(arverb::Parameter::Mix, 1.0f);
    engine.setParameter(arverb::Parameter::DuckAmount, 0.0f);
    engine.prepare(fs);
    std::vector<float> inL(frames, 0.0f), inR(frames, 0.0f);
    std::vector<float> outL(frames), outR(frames);
    if (std::strcmp(excitation, "right") != 0) inL[0] = 0.6f;
    if (std::strcmp(excitation, "left") != 0) inR[0] = 0.6f;
    for (std::size_t offset = 0; offset < frames; offset += 256u) {
        const auto count = std::min<std::size_t>(256u, frames-offset);
        engine.process(inL.data()+offset, inR.data()+offset,
                       outL.data()+offset, outR.data()+offset, count);
    }
    FILE* f = std::fopen(filename, "wb");
    if (!f) { std::perror(filename); return 1; }
    constexpr std::size_t bytesPerFrame = sizeof(float)*2u;
    const auto dataBytes = static_cast<std::uint32_t>(frames*bytesPerFrame);
    std::fwrite("RIFF", 1, 4, f); u32(f, 36u + dataBytes);
    std::fwrite("WAVEfmt ", 1, 8, f); u32(f, 16);
    u16(f, 3); u16(f, 2); u32(f, fs); u32(f, fs*8); u16(f, 8); u16(f, 32);
    std::fwrite("data", 1, 4, f); u32(f, dataBytes);
    std::array<float, 512> interleaved {};
    bool finite = true;
    for (std::size_t offset=0; offset<frames; offset+=256u) {
        const auto count=std::min<std::size_t>(256u,frames-offset);
        for (std::size_t i=0;i<count;++i) {
            const auto l=outL[offset+i], r=outR[offset+i];
            finite = finite && std::isfinite(l) && std::isfinite(r);
            interleaved[i*2]=l;
            interleaved[i*2+1]=r;
        }
        if (!finite || std::fwrite(interleaved.data(), bytesPerFrame, count, f)!=count) {
            std::fclose(f);
            std::remove(filename);
            std::fprintf(stderr, "Nonfinite engine output or WAV write failure\n");
            return 1;
        }
    }
    if (std::fclose(f)!=0) { std::perror(filename); return 1; }
    std::printf("P6 WAV %s: %u Hz, %zu frames, 32-bit float, mode=%s, %s\n",
                filename, fs, frames, arverb::algorithmName(engine.getAlgorithm()),
                excitation);
    return 0;
}
