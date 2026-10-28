#include <arverb/Reverb.hpp>
#include <array>
#include <cstdio>
#include <cstdlib>
#include <cmath>
#include <cstring>
#include <iostream>
#include <set>
#include <string>

static void require(bool okay, const char* why) {
    if (!okay) { std::fprintf(stderr, "FAIL: %s\n", why); std::abort(); }
}

int main() {
    std::set<std::string> unique;
    for (std::uint32_t i = 0; i < arverb::parameterCount; ++i) {
        const auto p = static_cast<arverb::Parameter>(i);
        const auto& info = arverb::parameterInfo(p);
        require(info.id && std::strlen(info.id) > 0, "missing ID");
        require(info.name && std::strlen(info.name) > 0, "missing name");
        require(unique.insert(info.id).second, "duplicate ID");
        require(info.minimum < info.maximum, "invalid range");
        require(info.defaultValue >= info.minimum && info.defaultValue <= info.maximum, "default bounds");
    }
    require(unique.find("mix") != unique.end(), "missing mix");
    require(unique.find("predelay") != unique.end(), "missing predelay");
    require(unique.find("decay") != unique.end(), "missing decay");
    // Automation ID stability: changing these strings breaks existing DAW sessions.
    constexpr std::array<const char*, 13> stableIds {
        "mix", "predelay", "decay", "size", "diffusion", "damping", "lowcut",
        "highcut", "moddepth", "modrate", "width", "early", "duck"
    };
    static_assert(stableIds.size() == arverb::parameterCount, "Append IDs; don't renumber");
    for (std::size_t i = 0; i < stableIds.size(); ++i)
        require(std::strcmp(stableIds[i], arverb::parameterInfo(static_cast<arverb::Parameter>(i)).id) == 0, "automation ID changed");
    for (const auto preset : {arverb::Preset::VocalPlate, arverb::Preset::VocalChamber,
                              arverb::Preset::MusicHall, arverb::Preset::AiryBallad}) {
        arverb::Engine engine;
        engine.loadPreset(preset);
        const auto expected = arverb::presetValues(preset);
        for (std::uint32_t i = 0; i < arverb::parameterCount; ++i) {
            const auto p = static_cast<arverb::Parameter>(i);
            const auto& info = arverb::parameterInfo(p);
            const float v = engine.getParameter(p);
            require(v == expected[i], "SSOT preset differs from Engine");
            require(std::isfinite(v), "nonfinite preset value");
            require(v >= info.minimum && v <= info.maximum, "preset range violation");
        }
        engine.prepare(48000.0);
        std::array<float, 128> left {}, right {};
        left[0] = 0.5f;
        right[0] = 0.5f;
        engine.process(left.data(), right.data(), left.data(), right.data(), left.size());
        for (const float v : left) require(std::isfinite(v), "nonfinite preset value");
        for (const float v : right) require(std::isfinite(v), "nonfinite preset value");
    }
    std::cout << "Parameter ID and preset contract OK\n";
}
