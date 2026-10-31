# ArSonVerb — reusable studio reverb core

**ArSonVerb: original, portable C++17 algorithmic reverb foundation for vocals and music.**

**Public stable v1.0.0 (2026-11-01), Windows x64 VST3 + Standalone, free AGPL-3.0-only.** After P9 [PR #30](https://github.com/masarray/arsonverb/pull/30), the owner accepted the startup and preset-switch reliability improvements for stable distribution. P9 retains 4 reverb algorithms, 32 presets, 13 stable control IDs, the compact gold interface and clickable SonKuPik credit. [Windows JUCE 12/12 tests](https://github.com/masarray/arsonverb/actions/runs/41225265424) and [Linux 11/11 plus sanitizers](https://github.com/masarray/arsonverb/actions/runs/41225267456) passed. The publication pipeline rebuilds and retests the exact tagged commit. **Published download:** [ArSonVerb v1.0.0 Windows x64 VST3 + Standalone](https://github.com/masarray/arsonverb/releases/download/v1.0.0/ArSonVerb-Windows-x64-v1.0.0.zip) · [official release page](https://github.com/masarray/arsonverb/releases/tag/v1.0.0) · [v1.0.0 release notes](docs/RELEASE_V1.0.0.md) · [AGPL licence](LICENSE). Hardware/software driver latency remains dependent on interface/host settings; no universal xrun-free claim.

**Current P8 v0.7.0 DEVELOPMENT AUDITION:** [PR #28](https://github.com/masarray/arsonverb/pull/28) merged with real-JUCE host preset priming before audio start, 128-frame coalesced DSP coefficient automation, and bounded wet-only 7.8 kHz top air. [Windows JUCE validation](https://github.com/masarray/arsonverb/actions/runs/41219722058) passed 11/11 CTests and uploaded unsigned VST3 + Standalone artifact `13049931087` and an eight-WAV audition pack `13049981061`. [Linux Release + sanitizers](https://github.com/masarray/arsonverb/actions/runs/41219722061) passed 10/10. **These are candidate test builds, not a hardware-verified crackle cure, commercial sonic parity claim or licensed public release.** Details and physical acceptance: [P8 realtime and top air](docs/P8_REALTIME_STARTUP_AIR.md).

**Historical P6 milestone (2026-10-31): v0.5.0 DEVELOPMENT PREVIEW.** [PR #26](https://github.com/masarray/arsonverb/pull/26) merged with native JUCE **mono→mono, mono→stereo, stereo→stereo** routing, actual four-mode mono host regression, and eight reproducible raw 48kHz impulse WAVs plus measured CSV and SHA-256 provenance. [Windows JUCE PR CI run 41208990180](https://github.com/masarray/arsonverb/actions/runs/41208990180) passed **10/10** VST3/Standalone host tests and uploaded the unsigned Windows binary artifact **13045452835** and physical audition pack **13045597649**. [Core PR run 41208990172](https://github.com/masarray/arsonverb/actions/runs/41208990172) passed Linux release/sanitizer. P5's original engine, four feedback matrices, 13 scalar IDs, 32 factory values, gold GUI and C ABI remain unchanged. **These are unsigned test artifacts, not a licensed public release or validated sonic equivalence.** Real DAW/ASIO listening, xrun and pluginval are still pending. See [P6 physical audition](docs/P6_PHYSICAL_AUDITION.md).

Historical v0.2–v0.4 notes later in this README are retained for traceability; any at-the-time "not yet Windows-built" or "one algorithm" statements there are superseded by the current P6 baseline.
No code or private algorithms from Valhalla DSP have been copied. The implementation uses established, independently implementable DSP techniques and publicly documented design goals. It is **not** presented as a sonic match to any third-party commercial plugin.

## Current P7 integration (2026-10-31)

**P7 v0.6.0 DSP is merged**, with RT60-aware high-frequency tail feedback calibration while retaining all 13 original scalar host automation IDs, four modes, the approved premium gold GUI, P6 mono/stereo routing, 32 factory programs and portable C ABI. [PR #27](https://github.com/masarray/arsonverb/pull/27) / commit `b1e7c6f` passed [Windows JUCE VST3/Standalone 11/11 CTests](https://github.com/masarray/arsonverb/actions/runs/41212540202) and [Linux Release/ASan+UBSan 10/10 core CTests](https://github.com/masarray/arsonverb/actions/runs/41212540154). The **unsigned v0.6.0 Windows test build** is artifact `13046572769`; the **P7 reverb-engine eight-WAV audition pack** is artifact `13046897126` (the shared CI name contains P6 but its provenance is P7). Artifact retention is seven days. This is NOT a licensed public release, verified auditory parity, or proof of real-device zero latency. Real workstation audio auditions, pluginval, ASIO/WASAPI p99/xruns and distribution licensing remain open. See [P7 sonic audit & physical test](docs/P7_RT60_AWARE_HF.md).

## Intent

A vocal-oriented reverb should make the singer feel seated *inside* the music rather than putting a wash over the words. The first core provides:

- 8-line Feedback Delay Network (FDN) with four selectable orthogonal feedback matrices: Classic (Householder), Plate (Hadamard), Chamber (Givens rotations) and Hall (alternate Householder); one efficient tank, not four separately running processors.
- Four-stage, stereo **allpass diffusion** before the network.
- Six early-energy taps per channel and smoothly adjustable **pre-delay**.
- RT60-derived feedback control, **frequency-dependent damping**, adjustable wet low/high cuts, and P5 bounded high-frequency feedback damping + wet-air shelf.
- Independent slow modulation oscillators for the feedback delays; `sin/cos` are not recomputed on every sample.
- Wet stereo width control, **input-envelope ducking**, and parameter smoothing.
- Separate **C ABI** for C#, Unity, other language runtimes, or a separate VST3 wrapper.
- **32 factory programs** (including the original four) mapped to mode and parameter settings in the JUCE UI; four selectable algorithm families have distinct feedback matrices, but global commercial sound-quality parity is unproven.
- No heap allocations, logging, blocking calls or mutexes in `process()`.

## Build (Windows / Visual Studio 2022)

Requirements: CMake >= 3.18 and a C++17-capable compiler. No plugin SDK needed to build the core.

```powershell
cmake -S . -B build -G "Visual Studio 17 2022" -A x64 -DARVERB_BUILD_SHARED=ON
cmake --build build --config Release
ctest --test-dir build -C Release --output-on-failure
```

`arverb_core` produces a static library for direct C++ use. `arverb_shared` produces a C ABI DLL usable from C# using P/Invoke. `arverb_demo` writes a 5-second, 48-kHz stereo floating-point WAV impulse response.

Linux/macOS:

```bash
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release -DARVERB_BUILD_SHARED=ON
cmake --build build -j
ctest --test-dir build --output-on-failure
./build/arverb_demo impulse.wav
./build/arverb_benchmark
./build/arverb_demo vocal_plate_impulse.wav plate
./build/arverb_demo music_hall_impulse.wav hall
```

## Core usage

```cpp
#include "arverb/Reverb.hpp"

arverb::Engine reverb;
reverb.loadPreset(arverb::Preset::VocalPlate);
reverb.prepare(48000.0);  // do this only while audio processing is stopped

// Called by the real-time audio thread; planar float buffers, in-place supported:
reverb.process(inLeft, inRight, outLeft, outRight, frameCount);

// Thread-safe automation targets; read at the next audio block:
reverb.setParameter(arverb::Parameter::DecaySeconds, 2.4f);
```

For a DAW **send/aux**, set `Mix=1` and use the host send return fader. For a direct **insert**, `Mix` normally stays lower (start near 0.15–0.25). `DuckAmount` can help preserve a lead vocal while phrases are being sung.

### Threading contract

- `prepare()` allocates and resets delay state; call only with audio suspended.
- `reset()` mutates delay state and must not race with `process()`.
- `process()` belongs to a single audio thread. **Do not use one engine concurrently in two process calls.**
- `setParameter()` uses atomic float targets; host/UI thread changes are picked up on the next process call. A multi-parameter preset is not an atomic transaction.
- Planar audio buffers must be valid for `frames` samples. Same-channel in-place input/output is supported; arbitrary cross-channel aliasing is not.

## Current validation (P5, 2026-10-31)

- **Core:** Linux/Windows/macOS release tests plus Linux ASan/UBSan passed on the post-merge `main` source [run 41205502461](https://github.com/masarray/arsonverb/actions/runs/41205502461).
- **Plugin:** JUCE 9.0.4 Windows VST3 and Standalone succeeded with 10/10 CTests, including genuine JUCE host and Mix/Air/mono regressions [run 41205502513](https://github.com/masarray/arsonverb/actions/runs/41205502513). Unsigned Windows test artifact ID `13044660087`, subject to the workflow's seven-day retention.
- **P5 audio:** equal-power gain targets and bounded wet-tail air improvements passed automated signal checks. This proves neither subjective Valhalla-class quality nor absence of all phase cancellation in a real DAW routing setup.
- **Realtime:** reported additional algorithmic plugin latency is 0 samples, **not** device or DAW zero latency. Shared-runner offline profiling does not establish physical ASIO/WASAPI xrun-free operation.
- **Distribution:** no outbound license has been approved; see [LICENSE-STATUS.md](LICENSE-STATUS.md). Do not package as a public ArSonMix Factory FX release yet.

## Next meaningful gates

1. **Physical musical acceptance:** dry vocal, acoustic guitar, piano and full-mix, P4↔P5 and commercial reference level-matched A/B, wet 0/25/50/75/100% and DUCK 0% diagnostic.
2. **Measure the actual sound:** stereo/mono correlation, decay RT60 per octave, echo-density buildup, metallic ringing, high-frequency tail retention, and true phase cancellation under real insert and aux/send routing.
3. **Real device:** ASIO/WASAPI 32/64-sample p95/p99 CPU, xruns, host scan/state recall, pluginval or a second host, mono/stereo cases and DPI rendering.
4. **Release gate:** verify distribution license/JUCE obligations, provenance, signed/versioned packaging and download workflow before external distribution. Implement another DSP milestone only against reproducible failed sonic gates.

See [docs/STATUS.md](docs/STATUS.md), [P5 engineering audit](docs/P5_MUSICAL_ENGINE_AUDIT.md) and [docs/PERFORMANCE_P1.md](docs/PERFORMANCE_P1.md).

## Historical engineering notes (superseded by P5 current state above)

## Historical v0.2: JUCE 9 VST3 + standalone wrapper (unverified at that time)

The plugin uses **JUCE 9.0.4**, C++17, CMake and **the same JUCE-free `arverb_core`**. No third-party audio DSP source is vendored.

Windows 2022 / Visual Studio 2022 / CMake **3.22+** (internet access for JUCE checkout):

```powershell
cmake -S . -B build-plugin -G "Visual Studio 17 2022" -A x64 -DARVERB_BUILD_PLUGIN=ON -DARVERB_BUILD_TESTS=ON
cmake --build build-plugin --config Release --target ARStudioReverb_VST3 ARStudioReverb_Standalone arverb_tests arverb_contract_tests
ctest --test-dir build-plugin -C Release --output-on-failure
```

For offline/controlled builds, download JUCE 9.0.4 yourself and configure with `-DARVERB_JUCE_SOURCE_DIR="C:/path/to/JUCE"`. Building the core needs no JUCE and no network.

The plugin wrapper exposes stable symbolic APVTS parameter IDs (`mix`, `predelay`, `decay`, `size`, `diffusion`, `damping`, `lowcut`, `highcut`, `moddepth`, `modrate`, `width`, `early`, `duck`) plus `bypass`. Parameter values are passed into the DSP through lock-free target updates. Stereo processing is in-place with no temporary audio buffers. Bypass smoothly takes the wet mix to zero while the reverb tail state keeps running. State XML includes schema version 1.

**Known limitations:** JUCE was not installed in the local environment that built v0.2; plugin syntax, host state round-trip, automation, host bypass, VST3 compatibility and editor scaling must be tested in Windows CI and DAW before any release. Four preset names still share one FDN topology, not four unique algorithms.

See [`docs/STATUS.md`](docs/STATUS.md) for evidence and the next-stage build blockers. CI `.github/workflows/plugin-ci.yml` is prepared, not executed.

## v0.3 reliability + host integration tests (source milestone, 2026-10-28)

- DSP precomputes immutable early-reflection and FDN delay sample coefficients in `prepare()`; tested median processing wall-time improved **~13%** compared with the archived v0.2 source on the same Linux test environment (3 runs, 30 s of audio at 48 kHz). This is not a DAW CPU measurement.
- A new 5-second impulse/feedback regression suite checks all four presets, wet stereo response, delayed onset and finite behavior in the presence of malformed host samples.
- Release and ASan/UBSan core tests: **3/3 passing** on Linux in this milestone.
- New `tests/test_plugin_host.cpp` implements an actual JUCE `AudioProcessor` test harness for state, automation and bypass. Plugin-enabled Windows CI compiles and runs this harness in addition to VST3/Standalone builds. **The JUCE SDK was unavailable locally, so this harness and the VST3 build have not yet been executed.**

To run core QA locally: `cmake -S . -B build -DCMAKE_BUILD_TYPE=Release -DARVERB_BUILD_TESTS=ON && cmake --build build --parallel && ctest --test-dir build --output-on-failure`.

To build the real plugin on Windows with Visual Studio 2022:

```powershell
cmake -S . -B build-plugin -G "Visual Studio 17 2022" -A x64 -DARVERB_BUILD_PLUGIN=ON -DARVERB_BUILD_TESTS=ON
cmake --build build-plugin --config Release --parallel 2 --target ARStudioReverb_VST3 ARStudioReverb_Standalone arverb_plugin_host_tests
ctest --test-dir build-plugin -C Release --output-on-failure
```

For complete validation gates and real status, use [`docs/STATUS.md`](docs/STATUS.md). Existing `v0.2` headings above describe the historical milestone rather than the current state.

## Historical v0.4 reliability and C ABI (2026-10-29)

- Product version now originates from `project(VERSION)` in CMake and is exported by `arverb_version()`; this eliminates stale library version strings.
- API compatibility has independent `ARVERB_ABI_VERSION` and parameter-count checks. New public C functions `arverb_abi_version()` and `arverb_parameter_count()` are additive; existing ABI function signatures are unchanged.
- New ISO-C test executable links against the *actual shared library*, ensuring symbols export and C clients can call the engine without C++/JUCE.
- Automation soak regression runs 1,500 deterministic processing blocks over five sample rates and frame sizes 0–2048, with random parameter changes and invalid input injection. Audio processing is checked for allocation, finite output and runaway feedback.
- JUCE host test *source* now initializes JUCE's GUI lifecycle, tests editor construction and resizing, and sends irregular host block sizes. **This source has not yet been built using JUCE**.
- Local evidence: GCC 14 Release **5/5**, Clang 17 Release **5/5**, GCC ASan/UBSan core **4/4**. Detailed scope: [`docs/QA_V0.4.md`](docs/QA_V0.4.md).
- Windows one-command build/test entry: `pwsh -File scripts/build-windows.ps1`. It requires Visual Studio 2022 and JUCE source or internet access.

**Important:** no Windows `.vst3` or DAW scan is claimed, and the four named presets are still parameter variations on one FDN. This milestone improves engineering safety and future reusability, not yet sonic parity with Valhalla.
#   a r s o n v e r b  
 