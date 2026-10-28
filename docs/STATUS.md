# AR Studio Reverb — cross-thread status

Updated: **2026-10-29** | Source milestone: **v0.4.0** | GitHub repo/CI: **NOT PUBLISHED / NOT RUN** | Validated Windows VST3: **NO**

## Latest verified work

- Source of truth: `AR-Studio-Reverb-v0.4.0-Engineering-Milestone.zip`, unpacked source. Base was v0.3 ZIP; no unrelated repositories modified.
- Local GCC 14 Release shared build + CTest **5/5**; Clang 17 Release shared build + CTest **5/5**; GCC 14 Debug ASan + UBSan core **4/4**.
- New C ABI version/parameter-count probes; product version generated from CMake project version; true shared-library ISO-C consumer test.
- Stress test: 1,500 blocks, randomized values of 13 parameters, 0/1/3/17/64/255/513/2048 frames, 8/44.1/48/96/192 kHz, NaN/Inf/FLT_MAX, finite feedback bounds, and heap-allocation interception on process().
- JUCE host test improved for GUI lifecycle/editor sizing; **JUCE is not installed in the local environment**, so plugin compile and DAW proof remain missing.
- Plugin Windows GitHub Actions updated to build all six test executables (five core/C ABI plus plugin host) and package VST3/Standalone **if it passes**. This workflow has **not run**.
- Added `scripts/build-windows.ps1` and `docs/QA_V0.4.md` for reproducible handoff.

## P1 exit blockers (unchanged)

1. Select/create dedicated AR Studio Reverb repository and publish exact source. GitHub `masarray` listings inspected, no dedicated project found; do not push into another project.
2. Run Windows MSVC GitHub Action and fix any **observed** build errors; capture SHA/logs/artifact.
3. Run JUCE host test, pluginval/Steinberg validator and at least one DAW scan/state reload.
4. Complete level-matched listening and spectral QA before premium-sound claims; still a single FDN topology.

## Evidence

See `docs/QA_V0.4.md`. Do not infer JUCE build/test success from core CTest success.

---

## Historical v0.3 handoff (archived below)

# AR Studio Reverb — canonical cross-thread status

Updated: 2026-10-28 | Source milestone: **v0.3.0** | Public release: **NONE**

## Authority / baseline

This package was based on `AR-Studio-Reverb-v0.2.0-JUCE9-Source-Milestone.zip`.
No dedicated AR Studio Reverb repository was found in the accessible `masarray` GitHub listings during this task. This is a **local source artifact**, not a GitHub commit. Do not assume CI has run or that a Windows VST3 binary exists.

## Verified local evidence (Linux, source v0.3)

- GCC 14 Release: CMake/Ninja core + C ABI shared library compiled; `ctest` **3/3 passed**.
- GCC AddressSanitizer + UBSan Debug: `ctest` **3/3 passed**.
- New deterministic `arverb_audio_regression_tests`: 4 preset wet impulse responses, 10 ms pre-delay gating, stereo tails, 5-second no-runaway window and recovery from NaN, Inf and FLT_MAX host input.
- DSP optimizations: immutable early-reflection taps, fixed FDN delay coefficients and ms->samples conversion are precomputed in `prepare()`. Clamp absurd host inputs to +/-32 before recursive feedback; preserves considerable above-0dBFS normal headroom.
- Offline 48 kHz / 30 s / 256-frame benchmark, same runner and release build, 3 repetitions:
  - v0.2: **0.3149, 0.3089, 0.3148 s**, median **0.3148 s**.
  - v0.3: **0.2853, 0.2729, 0.2740 s**, median **0.2740 s**.
  - Median wall-time decrease **12.96%** in this environment. Not a Windows/DAW CPU claim; audio output may differ slightly due to floating-point operation ordering.
- Existing C ABI, parameter ID, preset SSOT, no-heap processing and block-partition tests remain in the suite.

## Implemented source, NOT YET proven using the JUCE SDK

- JUCE 9.0.4 VST3/Standalone plugin wrapper remains source-only; no `.vst3` or `.exe` validated.
- Added `tests/test_plugin_host.cpp` using real `ReverbProcessor`: stereo bus, 14 host parameters (13 DSP + bypass), preset and automation changes, variable block lengths, host bypass callback, user bypass, APVTS XML state save/restore, corrupt state rejection and sample-rate reprepare.
- Updated `plugin-ci.yml`: build both plugin formats **and** host smoke target, run all CTest tests, upload unsigned artifact. This workflow has NOT run on GitHub.
- JUCE 9 SDK not present locally; cannot confirm compilation/API correctness until CI or a Windows developer workstation builds the project. Host test is source-level only today.

## Release blockers

1. Publish this exact source tree to a deliberately selected GitHub repository. Never overwrite another project's repository.
2. Run Windows 2022 MSVC JUCE 9.0.4 plugin workflow; fix any compiler errors from observed logs and attach successful VST3/Standalone unsigned test artifacts.
3. Exercise plugin in REAPER/Cubase or another actual DAW: scan, automated parameters, bypass, state roundtrip, editor resizing and shutdown/rescan.
4. Run Steinberg validator and/or pluginval as a separate release gate; verify actual results, including denormal and dynamic block sizes.
5. Real CPU benchmarking and level-matched vocal/music listening evaluation; four presets STILL share one FDN topology, not four algorithms.
6. Decide source-code license and review JUCE license before redistributing source or binaries. Freeze manufacturer/plugin IDs before first external release.

## Next meaningful milestone — P1 exit gate

The P1 milestone is **not complete** until a real GitHub Actions Windows build produces an installable VST3, host tests pass, and at least one DAW scan/session restore is confirmed. Avoid GUI embellishment and algorithm rewrites until this is demonstrated.

## Handoff

Read `AGENTS.md`, `docs/ARD.md`, `CMakeLists.txt`, and this file. Continue **from current source**, not memory. The next agent should inspect GitHub repository/PR/CI status, push this package only to the intended dedicated project, run plugin CI, resolve actual observed failures, record evidence here, and only then mark plugin compilation validated.
