# Pending GUI visual polish milestone

## 2026-10-30 premium compact brushed alloy GUI
- Original user screenshot feedback: flat stripes, washed-out coral, oversized default, full-precision JUCE tooltip.
- New default 842x394, min 800x370, max 1180x545.
- Multi-stop warm alloy gradient, deterministic irregular brushed texture, darker lower third, painted into cached JUCE Image on resize only.
- Native giant full-float popup removed. Compact short unit-correct value readout in header appears during knob drag; permanent Decay display remains.
- Compile-time dial/DSP order guard, host resized bounds tests, stable parameter-ID checks.
- All 13 automatable controls, 4 presets, bypass, reusable DSP core untouched; check Windows GUI build before merging.

---
# ArSonVerb GUI visual milestone — 2026-10-30 (VERIFIED)

- [PR #5](https://github.com/masarray/arsonverb/pull/5) merged into main, squash commit `465876fa0064f8b3857046c58f8dd947386ea614`.
- [GitHub Actions Windows JUCE run #41063910932](https://github.com/masarray/arsonverb/actions/runs/41063910932): **SUCCESS**. Windows MSVC VST3 + Standalone built, **6/6 CTest PASS** including real JUCE host contract and layout bounds checks at 830x390, 860x460, 1100x510, 1440x680. Raw unsigned artifact #12981532508.
- Fast PR core CI (Linux Release and sanitizer) also **SUCCESS**: https://github.com/masarray/arsonverb/actions/runs/41063910926
- GUI changed from 7x2 regular grid to original hardware-inspired copper/coral horizontal faceplate, dominant Decay, charcoal knobs, green numeric readout, and Tone/Shape/Space groups. **All 13 DSP parameter IDs** remain attached. Four presets and host bypass remain available.
- No modifications to realtime DSP, presets, core ABI, processor serialization, or parameter IDs. Native JUCE vector painting, no large bitmap, WebView, animation timers, or added library dependencies.
- **Still unverified:** actual screenshot / manual visual acceptance in a DAW, pluginval, high-DPI scaling on multiple displays. Do not say visually pixel-identical to Valhalla. Reference is for layout only; original ArSonVerb art.
- For future GUI changes see `docs/GUI_DESIGN.md`. Do not introduce fake controls or change automation IDs for purely visual alignment.
- Note: main may run additional Windows CI after PR merge; the successful **PR head** run above is authoritative evidence for this GUI source, while main status must be checked separately.

---

# ArSonVerb hardware-inspired GUI milestone — 2026-10-30

- Target image supplied by user: warm brushed hardware faceplate, large Decay knob, small tactile black controls, compact green readout, spatial Tone/Shape/Space groups.
- Implemented in `plugins/juce_vst3/PluginEditor.*` with native JUCE vector drawing, no bitmap resource or WebView. Original ArSonVerb colors and wordmark; no third-party brand assets.
- All **13 DSP controls**, 4 selectable presets, bypass, APVTS attachments and parameter automation are preserved. Knob captions correspond to actual DSP parameters. Decay display updates from the attached slider; user knob drag clears stale preset selection.
- Message-thread-only drawing and responsive layout; no changes to C++ DSP / audio callback. No new runtime dependency.
- JUCE host test extended to assert visible controls remain in bounds at 830x390, 860x460, 1100x510, 1440x680.
- Validation: review the corresponding GitHub Windows plugin workflow on PR before asserting a built binary exists. DAW visual QA and hardware remain an outstanding acceptance gate.
- This is an original hardware-inspired visual redesign, not a pixel-perfect or branded copy of Valhalla.

---

# ArSonVerb — verified integration status (2026-10-29)

**Canonical repo:** https://github.com/masarray/arsonverb  
**Main integration SHA:** `14d8c28e80e9838038692645b5c7f90f6f3b4cd8`  
**Product:** ArSonVerb 0.4.1, JUCE 9.0.4 + independent C++17 DSP core.  
**Status:** Windows VST3 **compiled** and real JUCE host-contract CTest **passed**. **Not** yet pluginval-, DAW-, or hardware-validated; **not** sound-quality parity with Valhalla.

## Verified evidence

- [PR #1](https://github.com/masarray/arsonverb/pull/1) merged, MSVC C2131 fixed (`constexpr juce::Colour` replaced by runtime `const`). New plugin name and bundle metadata, stable existing JUCE manufacturer/plugin numeric codes and parameter IDs.
- [PR #3](https://github.com/masarray/arsonverb/pull/3) merged: optional offline `scripts/sonic_qa.py` and `docs/SONIC_BASELINE.md`; Python/SciPy **not** in realtime DSP or plugin.
- [Windows JUCE PR build](https://github.com/masarray/arsonverb/actions/runs/40789813680): VST3, Standalone, host smoke tests **PASS**; six out of six CTest **PASS**; unsigned artifact **uploaded** (artifact #12862115128).
- [Core PR run](https://github.com/masarray/arsonverb/actions/runs/40789813630): Linux Release + ASan/UBSan **PASS**.
- [Core main SHA run](https://github.com/masarray/arsonverb/actions/runs/40790715579): Windows, macOS, Linux and sanitizer CI **PASS**.
- [Windows plugin main run](https://github.com/masarray/arsonverb/actions/runs/40790715515): still in progress at time of this evidence update; confirm final status before claiming main plugin CI green.
- [Unsigned Windows artifact download](https://github.com/masarray/arsonverb/actions/runs/40789813680/artifacts/12862115128): includes `ArSonVerb.vst3` bundle and `ArSonVerb.exe`. Filename in workflow still uses historical `ARStudioReverb` identifier; internal product binary is ArSonVerb.
- Checked bundled `moduleinfo.json`: correct `Name=ArSonVerb`, `Version=0.4.1` and 2 classes. It is **valid JSON5**, as required by Steinberg VST3; trailing commas are permitted, so do not mistakenly treat them as a packaging defect.

## Next unpassed gates

1. Actual pluginval/Steinberg validator result from an appropriately configured Windows/macOS environment.
2. DAW install, scan, preset/state save/restore, automation, mono/stereo behavior and long-term tail test on hardware.
3. Level-matched blind listening, mono-compatibility and per-band decay review before any premium-sonic claims (see [Issue #2](https://github.com/masarray/arsonverb/issues/2)). Four presets still use **one FDN topology**.
4. Resolve source and JUCE distribution licenses prior to a public licensed binary release. Artifact is an **unsigned test build**, not a production release.

---

# ArSonVerb — current GitHub integration handoff

Updated: 2026-10-29 | Base commit: `33f35872e6de82e2c3972bd2e713dc9be0a662f5` | Branch: `feat/arsonverb-brand-msvc-ci-gates`

- Canonical repository: https://github.com/masarray/arsonverb (public). The historical statement below that GitHub was not published is superseded.
- Core C++ CI passed on base commit: https://github.com/masarray/arsonverb/actions/runs/40789129891
- First Windows JUCE VST3 CI failed at GUI compilation: MSVC C2131 in `PluginEditor.cpp` lines 6–9, because `juce::Colour` was used in `constexpr` variables. Build/run: https://github.com/masarray/arsonverb/actions/runs/40789129900
- Proposed fix: file-scope `const juce::Colour` rather than `constexpr`; preserve all four color values. Rename user-visible product to **ArSonVerb** (JUCE plugin code and APVTS parameter IDs unchanged). No current release is established; freeze identifiers before first external public release.
- CI economy: PR core jobs run Linux + sanitizer; Windows/macOS core jobs run on main and manual dispatch. Plugin Windows PR runs only for plugin/CMake/workflow changes; core changes remain covered on main and manual plugin dispatch.
- **This note is not evidence of a Windows VST3 pass.** Read the newest PR workflow logs/artifacts. Gate progression on actual compile + CTest + VST3 artifact; then pluginval and real DAW test. Do not publish sonic parity claims with Valhalla without level-matched listening measurements.

---

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
