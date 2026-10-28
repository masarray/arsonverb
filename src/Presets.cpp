#include "arverb/Reverb.hpp"

namespace arverb {
namespace {
// Order is the public stable Parameter enum order; validated by contract tests.
constexpr std::array<float, parameterCount> vocalPlate {
    0.18f, 43.0f, 1.7f, 0.85f, 0.85f, 6900.0f, 260.0f,
    9200.0f, 0.12f, 0.28f, 1.12f, 0.20f, 0.45f
};
constexpr std::array<float, parameterCount> vocalChamber {
    0.16f, 28.0f, 1.25f, 0.78f, 0.64f, 4800.0f, 250.0f,
    7800.0f, 0.09f, 0.24f, 1.0f, 0.40f, 0.40f
};
constexpr std::array<float, parameterCount> musicHall {
    0.19f, 34.0f, 2.5f, 1.3f, 0.72f, 6100.0f, 175.0f,
    10600.0f, 0.22f, 0.34f, 1.18f, 0.28f, 0.22f
};
constexpr std::array<float, parameterCount> airyBallad {
    0.23f, 64.0f, 3.7f, 1.64f, 0.80f, 5500.0f, 320.0f,
    8000.0f, 0.32f, 0.41f, 1.26f, 0.14f, 0.52f
};
} // namespace

std::array<float, parameterCount> presetValues(Preset preset) noexcept {
    switch (preset) {
        case Preset::VocalPlate:   return vocalPlate;
        case Preset::VocalChamber: return vocalChamber;
        case Preset::MusicHall:    return musicHall;
        case Preset::AiryBallad:   return airyBallad;
    }
    return vocalPlate;
}

void Engine::loadPreset(Preset preset) noexcept {
    const auto values = presetValues(preset);
    for (std::size_t i = 0; i < parameterCount; ++i)
        setParameter(static_cast<Parameter>(i), values[i]);
}

} // namespace arverb
