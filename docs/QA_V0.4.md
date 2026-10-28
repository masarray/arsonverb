# QA evidence — v0.4.0 (2026-10-29)

## Baseline

Source: local archive `AR-Studio-Reverb-v0.3.0-DSP-QA-JUCE-Host.zip`. No dedicated GitHub repository was found from the authenticated `masarray` repository listings (first 100 repos and next page). These results are from an isolated local Linux container, not from Windows, a DAW, or GitHub Actions.

## Changes in v0.4

- Product version comes from the CMake `project(VERSION)` single source of truth, returned by `arverb_version()` to non-C++ consumers. Separate additive ABI compatibility version `ARVERB_ABI_VERSION = 1` and stable 13-parameter count.
- New true ISO-C shared-library integration test, including ABI/version probes and invalid parameter/handle inputs. C and C++ are deliberately separate to catch name mangling and DLL export/import errors in a future Windows build.
- Deterministic stress suite: randomized 13-parameter automation over 1,500 blocks at 8/44.1/48/96/192 kHz, frame sizes 0/1/3/17/64/255/513/2048, periodic resets, NaN/Inf/FLT_MAX injection and no `new`/`new[]` in `process()`.
- JUCE host test source improved with `ScopedJuceInitialiser_GUI`, editor creation/resizing and varied block sizes. **Still uncompiled until the JUCE SDK is available.**

## Verified in this local environment

- GCC 14.2 Release / Ninja / shared C ABI: CTest 5/5.
- Clang 17 Release / Ninja / shared C ABI: CTest 5/5.
- GCC 14.2 Debug ASan + UBSan, core-only: CTest 4/4.
- Offline realtime-safety tests check heap allocations; this is not a hard real-time scheduling proof, DPC measurement, CPU % in a DAW, or audio-quality verification.

## Unverified release gates

- Windows MSVC VST3 + standalone compilation, genuine JUCE AudioProcessor host integration test, binary `.vst3` output and DAW scan.
- Pluginval/Steinberg validator and platform-specific installers.
- Objective audio RT60-vs-frequency, mono compatibility, AB level-matched listening on dry vocal with music.

**Next action:** Create/select the dedicated repository, publish exact source, run `.github/workflows/plugin-ci.yml` on Windows, collect build logs, and fix the errors that are actually observed. Do not declare P1 done based only on the source-level wrapper.
