# ArSonVerb — reusable studio reverb core

**ArSonVerb: original, portable C++17 algorithmic reverb foundation for vocals and music.**

Historical local baseline: **v0.4.0 source prototype — 5/5 GCC and Clang Release tests, 4/4 core sanitizer tests; JUCE VST3/Standalone wrapper and host smoke test are source-only and NOT yet Windows-built/DAW-validated**.
No code or private algorithms from Valhalla DSP have been copied. The implementation uses established, independently implementable DSP techniques and publicly documented design goals. It is **not** presented as a sonic match to any third-party commercial plugin.

## Intent

A vocal-oriented reverb should make the singer feel seated *inside* the music rather than putting a wash over the words. The first core provides:

- 8-channel Feedback Delay Network (FDN), using an **O(N) Householder reflection** instead of a dense feedback matrix.
- Four-stage, stereo **allpass diffusion** before the network.
- Six early-energy taps per channel and smoothly adjustable **pre-delay**.
- RT60-derived feedback control, **frequency-dependent damping**, and cascaded wet low/high cuts.
- Independent slow modulation oscillators for the feedback delays; `sin/cos` are not recomputed on every sample.
- Wet stereo width control, **input-envelope ducking**, and parameter smoothing.
- Separate **C ABI** for C#, Unity, other language runtimes, or a separate VST3 wrapper.
- Four parameter presets: Vocal Plate, Vocal Chamber, Music Hall, Airy Ballad. **They currently share one topology**, not four different algorithms.
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

## Current validation

`arverb_tests`: block partition invariance, mono-to-stereo wet tail, finite output, reset-to-silence, in-place equivalence, supported sample-rate range 8–192 kHz, parameter bounds and the C ABI.

`arverb_benchmark`: deterministic offline throughput at 48 kHz, stereo, 256-sample blocks on the machine where it is run. **An offline benchmark is not a measured DAW CPU percentage or a sonic-quality guarantee.**

## Next development gates

1. **Listening + measurements:** level-matched comparisons with trusted reverb references on real dry vocals, acoustic guitar, piano, full mix; waterfall / spectrogram, echo-density, RT60-vs-frequency, mono collapse.
2. **Topology design:** introduce genuinely separate Plate, Chamber and Hall processors with consistent parameter semantics, rather than treating preset bundles as distinct modes.
3. **DSP refinements:** better decorrelation / modulation, time-varying diffusion, improved early-to-late envelope, multi-band RT60, subjective calibration.
4. **Host:** JUCE 9 wrapper is now drafted; validation of VST3 build, state/preset serialization, host automation and smooth bypass remains required.
5. **Production:** Windows/macOS DAW tests, denormal/FPU behavior, sanitizers, CPU profiling, audio stress tests, end-user UI and installer.

See `docs/ARCHITECTURE.md` and `docs/RESEARCH.md` for signal flow and references.

## IP and attribution

This project is an original starter created for its user, not a copy of the Valhalla products or a fork of the linked open-source reverb libraries. Their references are used for public technical background, **not** source code. Before distributing a public repository, choose its copyright notice and license. Product naming and branding should remain distinct from other plugin developers.

## Architecture decision and contribution workflow

See [`docs/ARD.md`](docs/ARD.md) for the JUCE VST3 direction, DSP boundary and release requirements; [`AGENTS.md`](AGENTS.md) for coding/AI rules; [`docs/STATUS.md`](docs/STATUS.md) for verified implementation status; and [`.github/workflows/core-ci.yml`](.github/workflows/core-ci.yml) for **core-only** cross-platform CI (not VST3 validation).

## v0.2: JUCE 9 VST3 + standalone wrapper (unverified build)

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

## v0.4 reliability and C ABI (2026-10-29)

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