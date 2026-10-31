# P1 — Realtime Performance Baseline (2026-10-31)

Status: **merged and CI-verified; not yet physical-DAW performance-certified**. This milestone measures the existing PR #19 audio/GUI engine without modifying the reverb sound or premium hardware artwork.

## Ground rules and external engineering references

- Steinberg VST3 [Processing FAQ](https://steinbergmedia.github.io/vst3_dev_portal/pages/FAQ/Processing.html): no I/O, memory allocation, networking, blocking or UI calls in realtime process; bypass remains the plugin's responsibility.
- Steinberg [API threading model](https://steinbergmedia.github.io/vst3_dev_portal/pages/Technical%2BDocumentation/API%2BDocumentation/Index.html): processing may execute on the audio thread and block sizes can vary.
- Reported plugin latency **0 samples** means no algorithmic delay is added for the dry output and no PDC is required. Audio-interface buffers, host buffers, predelay on the *wet* path, scheduling jitter and actual underruns are entirely separate.

## What is newly measured

Run the existing native core target (no external benchmark framework and no new thread created by the plugin):

```sh
cmake -S . -B build-perf -DCMAKE_BUILD_TYPE=Release -DARVERB_BUILD_TESTS=ON
cmake --build build-perf --config Release --target arverb_benchmark
./build-perf/arverb_benchmark --seconds 2 > p1-my-computer.csv
```

Available flags: `--quick` (0.14 s of audio per case, CI smoke only), `--seconds 0.1..30`, `--help`. Always choose the same computer, CPU power state, Release compiler/configuration and block size for before/after comparisons. The profiler prints 54 CSV rows covering:

| Dimension | Values |
|---|---|
| Sample rate | 48k / 96k / 192k |
| Callback block | 32 / 64 / 256 samples |
| Serial independent instances | 1 / 8 / 16 |
| Workload | Steady state, or block-rate 5-parameter extreme automation |

Each row records actual measured callback mean, p50, p95, p99, max in microseconds, **callback deadline = block size ÷ sample rate**, mean/p99 percentage of that deadline, and how many **offline** callbacks exceeded the theoretical deadline. The latter **is not a measurement of actual DAW dropouts**. Absolute times depend on CPU, compiler, clock resolution, background tasks, thermal state and a shared runner; the sample-rate cases include different DSP workloads. Percentiles are observed callback durations, not extrapolated real-time guarantees. Run longer samples on a known, idle local Windows/ASIO machine for decisions.

The benchmark times a **serial stack of instances** per callback, not 16 concurrent CPU cores. Engines are created/prepared and warmed outside the timed loop. Input buffers are prefilled outside timing. Each callback includes parameter changes in the stress case, processing of all instances, and no expensive file operations in the timed region. Clock start/stop adds measurement overhead which becomes material for short buffers. The checksum discourages dead-code elimination; no output-audio numerical comparisons are used to declare better sound.

A future optimization can compare two machine-matched CSVs with:

```sh
python scripts/compare_perf.py baseline.csv candidate.csv
```

The comparator reports worst p99 regressions and best improvements, without imposing arbitrary noisy CI speed thresholds.

## GUI paint/resize probe (Windows JUCE host)

With a real JUCE build, set `ARVERB_GUI_PROFILE_CSV=p1-gui-profile.csv` before running `arverb_plugin_host_tests`. This runs the *existing* JUCE host/editor integration test and writes two rows for 842×394 and 1180×545. It measures a full editor paint including children (16 repetitions, after cache warmup), plus resize cost. The benchmark runs on the message thread of the standalone host test, **never** in the audio callback. Offscreen JUCE software painting is not evidence of actual GPU composition, HiDPI smoothness, host scheduling, FPS or mouse interaction performance. There are no CPU speed assertions.

## CI economy and deliverables

- Reuse existing Core C++ CI matrix. Only Linux Release runs `arverb_benchmark --quick` once, uploads `ArSonVerb-P1-Core-Callback-Profile-Linux` for seven days. Sanitizer, Windows and macOS jobs do not repeat a timing probe.
- Reuse existing JUCE Windows pipeline; now the VST3 test workflow also activates for **tests/**-only PR changes. Full Windows 6/6 CTest remains required. The host test writes `p1-gui-profile.csv` next to the unsigned Windows VST3+Standalone artifact.
- **Do not compare CI absolute microseconds to Windows field performance.** Do not tune DSP based on one random CI run. Prefer fixed-machine p99 plus regression spectra and level-matched listening before changing sonic math.
- No added allocations/locks/timers/FFT to Engine::process. No modification of the actual reverb engine, smoothing, presence ducking, 13 APVTS IDs, 32 programs, C ABI, 98/200 gold assets, or GUI thread model.

## P1 acceptance checklist

1. Confirm Linux/macOS/Windows core tests and Linux ASan/UBSan.
2. Confirm Windows JUCE VST3 and standalone build + all six CTests.
3. Inspect both CSV artifacts and flag unusually high p99/mean budget as a target for real hardware repeat tests, **not** proof of DAW underruns.
4. Run minimum 2-second/profile per scenario on a physical DAW workstation, record CPU model, Windows power policy, sample rate, ASIO buffer, number of instances, and host.
5. Compare GUI paint/resize with real 100/125/150% host screenshots. Audio quality A/B remains outside P1; preserve current tone.

**End state of P1:** an inspectable and repeatable performance baseline and profiling gates. Numeric CPU optimization (or SIMD) should be the *next* change only after this baseline identifies a bottleneck.
