#pragma once

#include <array>
#include <atomic>
#include <cstddef>
#include <cstdint>
#include <memory>

namespace arverb {

static_assert(std::atomic<float>::is_always_lock_free,
              "This real-time engine requires lock-free float parameter atomics");

// Stable numeric IDs for host automation, presets and the C ABI.
enum class Parameter : std::uint32_t {
    Mix = 0,
    PreDelayMs,
    DecaySeconds,
    Size,
    Diffusion,
    DampingHz,
    LowCutHz,
    HighCutHz,
    ModDepthMs,
    ModRateHz,
    Width,
    EarlyLevel,
    DuckAmount,
    Count
};

struct ParameterInfo {
    const char* id;
    const char* name;
    const char* unit;
    float minimum;
    float maximum;
    float defaultValue;
};

constexpr std::size_t parameterCount = static_cast<std::size_t>(Parameter::Count);
const ParameterInfo& parameterInfo(Parameter param) noexcept;

enum class Preset : std::uint32_t {
    VocalPlate, VocalChamber, MusicHall, AiryBallad
};

// Single source of truth for fixed preset bundles. No allocation, no state mutation.
std::array<float, parameterCount> presetValues(Preset preset) noexcept;

// The C++ DSP core has no dependency on VST, JUCE, audio I/O, UI or operating system.
// A single Engine instance belongs to exactly one audio thread.
// prepare/reset must be called with audio processing suspended. SetParameter is thread-safe.
class Engine final {
public:
    Engine();
    ~Engine();
    Engine(const Engine&) = delete;
    Engine& operator=(const Engine&) = delete;

    // Allocation and configuration ONLY here, never inside process().
    // Supported sample rates: 8 kHz - 192 kHz. Throws std::invalid_argument for invalid rates.
    void prepare(double sampleRate);
    void reset() noexcept;
    bool prepared() const noexcept;

    // Thread-safe, lock-free target updates; parameters become active at next process() call.
    void setParameter(Parameter parameter, float value) noexcept;
    float getParameter(Parameter parameter) const noexcept;
    void loadPreset(Preset preset) noexcept;

    // Planar stereo f32. In-place per channel supported. Avoid cross-channel buffer aliases.
    // All input/output pointers must be valid when frames > 0. No allocation, locks or I/O.
    void process(const float* inputLeft, const float* inputRight,
                 float* outputLeft, float* outputRight, std::size_t frames) noexcept;

private:
    struct State;
    std::unique_ptr<State> state_;
    std::array<std::atomic<float>, parameterCount> parameters_;
};

}  // namespace arverb
