// ArSonVerb P1: reproducible *offline* callback scheduling probe.
// Measures the core Engine, not a DAW, driver, GPU or end-to-end latency.
// No clocks, I/O, allocations or profiler instrumentation in Engine::process.
#include <arverb/Reverb.hpp>
#include <algorithm>
#include <array>
#include <chrono>
#include <cmath>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <memory>
#include <numeric>
#include <thread>
#include <vector>

namespace {
using Clock = std::chrono::steady_clock;
constexpr std::array<int, 3> sampleRates {{48000, 96000, 192000}};
constexpr std::array<int, 3> blockSizes {{32, 64, 256}};
constexpr std::array<int, 3> instanceCounts {{1, 8, 16}};

double micros (Clock::time_point start, Clock::time_point end) {
    return std::chrono::duration<double, std::micro>(end - start).count();
}
double percentile (const std::vector<double>& sorted, double proportion) {
    const auto at = static_cast<std::size_t>(
        std::ceil(proportion * static_cast<double>(sorted.size())));
    return sorted[std::min(sorted.size() - 1, std::max<std::size_t>(1, at) - 1)];
}
void updateAutomation (arverb::Engine& engine, int iteration) noexcept {
    // Change 5 controls per *host block*, intentionally stressing cached
    // exp/sin/cos paths. Values are legal and fixed for each callback.
    // This is a conservative cost envelope, not typical human automation.
    const bool alternate = (iteration & 1) != 0;
    engine.setParameter(arverb::Parameter::DecaySeconds, alternate ? 1.6f : 4.5f);
    engine.setParameter(arverb::Parameter::ModRateHz, alternate ? 0.29f : 1.07f);
    engine.setParameter(arverb::Parameter::DampingHz, alternate ? 3800.0f : 9000.0f);
    engine.setParameter(arverb::Parameter::LowCutHz, alternate ? 180.0f : 400.0f);
    engine.setParameter(arverb::Parameter::HighCutHz, alternate ? 7000.0f : 12000.0f);
}
void runCase(int rate, int block, int count, bool automation, double seconds) {
    const auto callbacks = std::max<std::size_t>(
        80, static_cast<std::size_t>(std::ceil(seconds * rate / block)));
    const auto warmup = std::max<std::size_t>(
        16, static_cast<std::size_t>(std::ceil(0.05 * rate / block)));
    std::vector<std::unique_ptr<arverb::Engine>> engines;
    engines.reserve(static_cast<std::size_t>(count));
    for (int n = 0; n < count; ++n) {
        auto e = std::make_unique<arverb::Engine>();
        e->loadPreset(arverb::Preset::MusicHall);
        e->prepare(rate);
        engines.push_back(std::move(e));
    }
    std::vector<float> inL(static_cast<std::size_t>(block));
    std::vector<float> inR(static_cast<std::size_t>(block));
    std::vector<float> outL(static_cast<std::size_t>(block));
    std::vector<float> outR(static_cast<std::size_t>(block));
    for (int i = 0; i < block; ++i) {
        // Fixed, finite test signal; no I/O or synthesis in timed region.
        inL[static_cast<std::size_t>(i)] = 0.20f * std::sin(0.051f * i);
        inR[static_cast<std::size_t>(i)] = 0.20f * std::cos(0.037f * i);
    }
    const auto invoke = [&](std::size_t callback) {
        for (int n = 0; n < count; ++n) {
            if (automation) updateAutomation(*engines[static_cast<std::size_t>(n)],
                                              static_cast<int>(callback));
            engines[static_cast<std::size_t>(n)]->process(
                inL.data(), inR.data(), outL.data(), outR.data(),
                static_cast<std::size_t>(block));
        }
    };
    for (std::size_t i = 0; i < warmup; ++i) invoke(i);
    std::vector<double> durations;
    durations.reserve(callbacks);
    double checksum = 0.0;
    for (std::size_t i = 0; i < callbacks; ++i) {
        const auto begin = Clock::now();
        invoke(i + warmup);
        const auto end = Clock::now();
        durations.push_back(micros(begin, end));
        checksum += outL[i % static_cast<std::size_t>(block)] +
                    outR[(i * 7u) % static_cast<std::size_t>(block)];
    }
    if (!std::isfinite(checksum)) {
        std::fputs("Non-finite output during performance probe\n", stderr);
        std::exit(3);
    }
    const double total = std::accumulate(durations.begin(), durations.end(), 0.0);
    std::sort(durations.begin(), durations.end());
    const double deadline = static_cast<double>(block) * 1.0e6 / rate;
    const auto missed = static_cast<std::size_t>(std::count_if(
        durations.begin(), durations.end(),
        [deadline](double d) { return d > deadline; }));
    const double mean = total / static_cast<double>(durations.size());
    const double p99 = percentile(durations, 0.99);
    std::printf("%d,%d,%d,%s,%zu,%.4f,%.4f,%.4f,%.4f,%.4f,"
                "%.4f,%.3f,%.3f,%zu,%.9g\n",
                rate, block, count, automation ? "block_automation" : "steady",
                callbacks, mean, percentile(durations, 0.50),
                percentile(durations, 0.95), p99, durations.back(),
                deadline, 100.0 * mean / deadline,
                100.0 * p99 / deadline, missed, checksum);
}
} // namespace

int main(int argc, char** argv) {
    double seconds = 1.25; // Longer profiling by default, opt-in command.
    for (int i = 1; i < argc; ++i) {
        if (std::strcmp(argv[i], "--quick") == 0) {
            seconds = 0.14; // CI smoke only; never a reliable CPU ranking.
        } else if (std::strcmp(argv[i], "--seconds") == 0 && i + 1 < argc) {
            char* end = nullptr;
            const double requested = std::strtod(argv[++i], &end);
            if (!end || *end != '\0' || !std::isfinite(requested) ||
                requested < 0.1 || requested > 30.0) {
                std::fputs("Invalid --seconds; use 0.1..30\n", stderr);
                return 2;
            }
            seconds = requested;
        } else if (std::strcmp(argv[i], "--help") == 0) {
            std::puts("arverb_benchmark [--quick | --seconds N]\n"
                      "CSV: all 54 cases, no realtime scheduling, CPU jitter possible.");
            return 0;
        } else {
            std::fputs("Unknown argument; see --help\n", stderr);
            return 2;
        }
    }
    std::printf("# ArSonVerb P1 offline core benchmark; clock=steady_clock,"
                " logical_cores=%u,seconds_per_case=%.3f\n",
                std::thread::hardware_concurrency(), seconds);
    std::puts("# Do NOT infer DAW glitches, audio-device latency or head-to-head plugin superiority.");
    std::puts("sample_rate,block_size,instances,mode,callbacks,mean_us,p50_us,"
              "p95_us,p99_us,max_us,deadline_us,mean_budget_pct,p99_budget_pct,"
              "offline_over_deadline_callbacks,checksum");
    for (int rate : sampleRates)
        for (int block : blockSizes)
            for (int count : instanceCounts)
                for (bool automation : {false, true})
                    runCase(rate, block, count, automation, seconds);
    return 0;
}
