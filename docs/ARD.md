# AR Studio Reverb — Architecture Requirements & Decisions (ARD)

**Document ID:** ARD-001  
**Revision:** 1.1 (architecture baseline + v0.2 source implementation note)  
**Date:** 2026-10-24  
**Status:** Recommended baseline / maintainer approval pending; v0.2 source wrapper authored but not JUCE-compiled  
**Product:** AR Studio Reverb; independent AR Reverb DSP Core  
**Audience:** C++ DSP, plugin, UI, test/release engineers and coding agents.

> This document is a technical contract for future development, **not proof that the described production features already exist**. For verified implementation status use `docs/STATUS.md`. This ARD is specific to the AR Studio Reverb repository; do not apply its conventions to unrelated projects.

## 1. Executive architecture decisions

| ID | Decision | Rationale / constraints |
|---|---|---|
| D-01 | **JUCE 9** for the first VST3 host wrapper and editor | Mature DAW integration, state/automation infrastructure, native UI; subject to licence compliance. |
| D-02 | **No JUCE or ImGui in `arverb_core`** | A single reusable DSP codebase for VST3, CLI, standalone, C ABI, future hosts. |
| D-03 | **Dear ImGui is not the customer-facing plugin UI** | It is excellent for diagnostics, but not a full host/plugin solution and has limited built-in accessibility. |
| D-04 | **C++17, CMake, explicit API boundaries** | Portable builds, straightforward MSVC/Clang/GCC toolchains, disciplined ownership. |
| D-05 | **GitHub is the source of truth; CI gates PR merges** | Reproducible changes, cross-thread continuity, independent verification. |
| D-06 | **No proprietary third-party DSP source copying** | Independently develop against published principles and measurable audio goals. |
| D-07 | **Public GitHub only when publishing source is intentional** | Public CI is efficient, but public code and history cannot reliably be made private again. |
| D-08 | **Select one plugin wrapper at a time** | Avoid building JUCE and iPlug2 in parallel; revisit iPlug2 only for a documented licensing or architectural trigger. |

**Licensing review required before external release.** JUCE 9 is dual-licensed under AGPLv3/commercial terms; its Starter, Indie and Pro tiers and revenue/funding rules must be evaluated for the intended licensee. VST3 SDK 3.8+ is MIT; iPlug2 has a permissive zlib-like licence. Do not infer that JUCE automatically obliges this entire project to be open-source if the proper commercial licence is obtained; do not infer that all external audio algorithms are licence-free.

## 2. Product definition and non-goals

**Primary use:** tasteful vocal and instrument/music mixing (pop, acoustic, ballad, singer-songwriter). The listener should perceive space and width without sacrificing intelligibility, transient clarity, or mono compatibility.

**Sound priorities, in order:**
1. Clean vocal intelligibility; tail should blend behind the direct signal rather than obscure it.
2. Smooth, non-metallic late energy with useful Plate, Chamber and Hall identities.
3. Controllable spectral decay (low-frequency cleanup, high-frequency absorption) and restrained modulation.
4. Useful early/late balance, predelay, width and optional musical ducking.
5. Predictability under automation, sample-rate and host block changes.

**Not in the initial release:** convolution IR library/browser, AI audio inference, cloud authentication, sample libraries, surround/Atmos, GPU DSP, extensive preset marketplace, copied commercial algorithms, mandatory online telemetry. Do not inflate the MVP with unnecessary engine frameworks or microservices.

## 3. Module boundaries and runtime data flow

```text
DAW / Host (VST3)
  │ stereo audio; sample rate, block size, transport, parameter changes
  ▼
plugins/juce_vst3/                      [SOURCE AUTHORED v0.2; BUILD UNVERIFIED]
  ├── Processor: layout negotiation, state, host lifecycle, bypass
  ├── ParameterBridge: stable IDs, value scaling, thread boundary
  └── Editor: custom JUCE Components, reusable design tokens
  │ AudioBuffer views + immutable / atomic parameter targets
  ▼
include/arverb/ + src/                 [EXISTS: v0.1]
  ├── Engine / Reverb modes             [ONE BASE TOPOLOGY EXISTS]
  ├── Delay, diffusion, ER, FDN, filters, envelope, modulation
  ├── Wet routing, smoothing, level control
  └── DSP state owned by exactly one processing thread
  │
  ├── examples/ offline renderer        [EXISTS]
  ├── tests/ core tests / benchmark     [EXISTS]
  └── C ABI -> other apps / C#          [EXISTS, provisional ABI]
```

Dependencies must flow inward: UI -> host wrapper -> public DSP API. The DSP core must never include JUCE, ImGui, Steinberg interfaces, UI controls, OS windowing, file pickers, DAW concepts or network dependencies.

### DSP public interface requirements

- `prepare(sampleRate, ...)` allocates capacity only when processing is suspended.
- `process(...) noexcept` is allocation-free, lock-free, synchronous and independent of GUI lifecycle.
- `reset()` is only safe while the processing thread is not concurrently using the engine.
- Parameter targets may be updated across the control/audio boundary using well-defined synchronization; **multi-parameter preset transactions are not atomic in v0.1**.
- Host automation uses stable symbolic parameter IDs. Do not reorder numeric IDs in released plugin sessions; add migration/version mapping instead.
- Presets use a versioned, documented representation, not raw `memcpy` of object layouts.
- Prefer a small C ABI layer for cross-language embedding. Freeze ABI only after explicit compatibility tests.
- GUI closure must not suspend or recreate the DSP engine.

### DSP processing design

**v0.1 actually implemented:** eight-line Householder FDN (O(N) orthogonal feedback), stereo pre-diffusion, simple early taps, filters, modulation, predelay, ducking, smoothed parameters, C ABI. The preset names are tuning configurations on **one** topology. No claims of true plate modeling or Valhalla parity.

**Target design** (subject to listening-test evidence):
- `PlateMode`: dense diffusion / smooth and compact timbral profile; distinctive topology versus other modes.
- `ChamberMode`: clear early reflections and moderate room character suited to intimate vocals.
- `HallMode`: spacious late field, adjustable early/late balance, restrained wandering modulation.
- Shared primitive library: delay, allpass, filters, envelope, modulation, orthogonal mixing, crossfade, denormal handling.
- Shared wet EQ and optional envelope-based ducking follow the mode's output unless listening tests suggest otherwise.

Do not instantiate a generic node-graph runtime until evidence shows it is necessary. Choose data-oriented, predictable hot loops and simple ownership. Use SIMD only after profiling; prefer high-impact algorithmic work (FDN matrix structure, contiguous delay storage, coefficient updates only on parameter changes, bounded interpolation).

## 4. Real-time and reliability contract

Audio callback SHALL NOT allocate/free heap memory, acquire blocking mutexes, write logs, access files/networks, wait on another thread, call UI APIs, throw exceptions, or create/destroy DSP state. Audio processing must be bounded with no unbounded loop or potentially blocking operation.

Required behaviour:
- valid planar stereo `float` buffers; same-channel in-place processing; host adapter handles the actual interleaved/bus layouts;
- zero-frame blocks, varying blocks, silence, denormals, extreme but valid parameters and parameter changes;
- supported baseline sample rates: 44.1, 48, 88.2, 96 and 192 kHz (v0.1 tests also check 8 kHz); other rates handled explicitly;
- finite output and bounded gain for valid inputs; no NaNs/infs;
- deterministic offline render under fixed configuration; test block partition invariance;
- fast reset and stable re-preparation only off the active audio callback;
- host parameter/state restoration robust to legacy plugin versions;
- switching algorithms without audible clicks; initially crossfade/morph on the audio thread with resources pre-allocated outside the callback;
- hard bypass follows host expectations; smooth wet/mix transitions; tail-on-bypass policy documented and tested;
- avoid host-latency claims until actual wrapper is implemented; core v0.1 does not use lookahead.

**Optimization policy:** profile before micro-optimization; compare Build Release versus Release under identical inputs/hardware; collect median, p95/p99 block time, peak memory, and audible regressions. CI virtual-machine throughput is useful as a regression signal, **not** a fixed CPU usage SLA.

**Provisional performance target (must be calibrated on a named reference PC):** p99 process time <0.25 ms per 128-frame stereo block at 48 kHz for the default Hall, after warmup; zero audio-thread allocations, zero glitches under test harness. 128/48000=2.667 ms callback budget. This is a project target, not a measured result. All actual released results must name OS, CPU, toolchain, compiler flags, sample rate and buffer.

## 5. Host wrapper contract: JUCE 9

Use JUCE only under `plugins/juce_vst3/` with CMake `juce_add_plugin`. The host-facing processor owns an `arverb::Engine` instance; an editor does not own the audio engine.

- **Lifecycle:** `prepareToPlay`, `releaseResources`, `processBlock`, `getStateInformation`, `setStateInformation` are mapped carefully; allocation/state transitions happen off the real-time processing path or via safe prepared-state swaps.
- **Parameters:** stable host-exposed IDs and ranges, scaling with plain engineering units. Begin with block-target capture plus smooth transitions; do **not** claim sample-accurate automation unless sample-offset events are explicitly handled and tested.
- **State:** version number, mode ID, all exposed parameter values, future migration. Round-trip test in real hosts and against malformed input.
- **Formats:** Windows x64 VST3 first. macOS AU and universal packages, CLAP, standalone follow only after the native VST3 is robust; avoid speculative promises.
- **Host tests:** Steinberg VST3 validator, pluginval, REAPER and at least two additional DAWs over time; a successful DLL build is not host validation.
- **Host threading:** JUCE UI message thread separate from audio thread; listener callbacks and value attachments must not introduce blocking work into `processBlock`.

## 6. Compact professional UI system

UI v1 defaults to a compact, non-bulky native JUCE editor. Prefer a coherent set of design tokens rather than independent hardcoded control colours.

- **Surfaces:** near-neutral slate/charcoal dark + restrained accent; no brown, beige or gradient-heavy skeuomorphism.
- **Typography:** modest hierarchy; standard labels ~12–13 px, section labels ~13–14 px, restrained title ~16–18 px; text remains readable with DPI scaling.
- **Spacing:** 4 px spacing quantum; 8/12/16 px common gaps; consistent control sizing and aligned baselines.
- **Layout:** compact header (mode + preset + A/B); main row (Decay, Pre-delay, Mix, Width); secondary rows (Damping, EQ, Modulation, Early/Late, Ducking); footer (input/output meters, bypass). Avoid large empty cards.
- **Interaction:** keyboard accessibility where practical, clear hover/focus, numerical entry and double-click reset, mouse-wheel behaviour tested across DAWs, 100/125/150/200% scaling, undo via host where supported, tooltips with units.
- **Animation:** display at sensible UI frequency, stop when hidden, and never couple DSP rate to rendering rate; GPU/OpenGL path is **optional**, not required for v1.
- **Design source:** `ui/tokens.json` (future), UI snapshot tests (future) and one QA checklist for all modes.

## 7. Repository structure and ownership

```text
AGENTS.md                             AI / engineering invariants [ADDED]
README.md                             Customer- and developer-facing orientation [EXISTS]
CMakeLists.txt                        Source build authority [EXISTS]
include/arverb/                       Public DSP API + C API [EXISTS]
src/                                  DSP implementation, presets, C wrapper [EXISTS]
tests/                                Core tests and offline benchmark [EXISTS]
examples/                             Offline renderer, C# embedding [EXISTS]
docs/ARD.md                           This contract [ADDED]
docs/ARCHITECTURE.md                  v0.1 implementation details [EXISTS]
docs/STATUS.md                        Verifiable project handoff [ADDED]
docs/adr/0001-framework-and-host.md   Framework decision [ADDED]
.github/workflows/core-ci.yml         Portable CORE CI [ADDED]
plugins/juce_vst3/                    JUCE processor / editor [SOURCE WRITTEN, NOT COMPILED]
ui/                                   UI tokens/components [PLANNED, NOT BUILT]
tools/audio_regression/              Objective measurements [PLANNED, NOT BUILT]
```

Boundary tests prevent accidental JUCE includes in `include/arverb/` / `src/` once the wrapper lands. Dependencies should be pinned/tagged with licence file, provenance, and change-control notes. Avoid wildcard dependency updates on main.

## 8. CI strategy and release gates

**Current workflow in this package:** `.github/workflows/core-ci.yml`, with CMake core build and CTest on Ubuntu, Windows, macOS, plus Linux sanitizers. It does **not** build the JUCE plugin because a JUCE host project has not yet been implemented; it must not masquerade as VST3 CI.

**Future pipeline (gated as implemented):**

| Gate | When | Evidence |
|---|---|---|
| G0 | Every PR | Compile (3 OS), CTest, real-time allocation test, sanitizer on supported target, stable parameter IDs. |
| G1 | Audio algorithm PR | Reference signal render + numeric audio regression, tail/RT60 per-band report, listening checklist and recorded differences. |
| G2 | Plugin wrapper PR | VST3 built, host API validation, preset/state round-trip, automation/bypass/zero-frame tests. |
| G3 | Release candidate | Windows DAW smoke tests, macOS if promised, performance baseline on named hardware, binary provenance/attributions. |
| G4 | Public release | Version tag, CHANGELOG, checksums, licence inventory, release notes, verified install/uninstall. |

Use required status checks and at least one review for protected `main` (where plan permits). No direct pushes to `main`; no force pushes. Only promote PRs after green checks and specific human approval for release/license decisions. CI failure must be fixed, not bypassed. Keep `pull_request` jobs unprivileged (`contents: read`) with no write tokens or secrets; release/signing workflow remains separate and gated. Pin external actions to a reviewed immutable SHA for production-hardening.

**Public/private:** private repo is recommended during unresolved IP/licence planning and early code preparation. A public repo is ideal for open-source credibility and standard-runner CI once license, attribution, commit history and release plan are deliberately approved. **Never put third-party protected DSP source or secrets into git history.**

## 9. Cross-thread / agent handoff protocol

**State is the repo, not the model's conversational memory.** `AGENTS.md` defines short invariants, `docs/STATUS.md` declares implemented / planned / blocked items, `docs/ARD.md` covers architecture, and ADRs explain durable decisions. Read only the relevant files for the task rather than dumping the entire repo into context.

For every completed engineering PR:
1. Describe scope and changed files.
2. Show exact commands and results of tests run, with OS/commit reference.
3. State what is **not** tested and known regressions.
4. Update `docs/STATUS.md` and any changed ADR requirement.
5. Keep PR scope small and reviewable. Link any follow-up tasks.

Suggested new-thread prompt: `Continue AR Studio Reverb from the current repository main. Read AGENTS.md and docs/STATUS.md, inspect open PRs and CI; do not assume planned items are implemented. Select the highest-priority unblocked task, work in a branch, run applicable tests, update STATUS.md, and report verification evidence.`

## 10. Phased implementation plan (acceptance based)

**P0 — repository/CI foundation (this document package):** publish only after review; PR runs on 3 OS and Linux sanitizers; source of truth and handoff docs exist. Success requires an observed green GitHub workflow, not merely a YAML file.

**P1 — first JUCE VST3 wrapper:** compile Windows VST3; instantiate core, process stereo, expose stable automation parameters, save/restore state, host bypass and basic editor. Validate with VST3 validator and pluginval, then DAW smoke test.

**P2 — sonic engineering:** A/B test and analyze vocal, acoustic guitar, piano, full mix; implement actually distinct algorithms (Plate/Chamber/Hall) through bounded, benchmarked refactors. No deceptive mode names in release.

**P3 — product polish:** consistent compact GUI, presets, A/B, parameter tooltips, high-DPI, DAW matrix, performance and audio regression gates.

**P4 — distribution:** installer/ZIP, signed binaries if required, source/licence inventory, checksums, tags and public release. Public beta is separate from stable.

### Objective audio QA acceptance candidates

- Difference tests over block divisions (1/64/128/512 samples) and sample rates 44.1/48/96/192 kHz.
- Long-tail finite checks, denormal stress, silence/reset, in-place, automation extremes.
- Frequency-resolved decay, density buildup, spectral balance, mono compatibility, channel decorrelation; compare level-matched excerpts across several vocal genres.
- Controlled blind listening against reference plugins using matched wet loudness, predelay and approximate decay. Record human evaluations; numerical similarity alone is not sonic quality.
- Results on isolated vocals **and** a full mix, both solo and in context, because musical blend is the product's main goal.

## 11. Decision triggers / change control

- **JUCE -> iPlug2:** only if licence economics, portability constraints or verified host issues outweigh migration cost. Since DSP is independent, adapter replacement remains local.
- **Add convolution:** only if tests demonstrate missing perceptual realism in early response and memory/CPU budgets permit it.
- **SIMD:** only when profiled hotspots justify greater complexity and deterministic regression tests exist.
- **Multithreading the DSP:** avoid for single stereo reverb callback unless concrete profiling and scheduling guarantees justify it; extra threads can increase dropouts.
- **Expose a public core API / ABI:** require semantic versioning, tests in a second client and compatibility migration policy first.

## 12. Sources and context (verified 2026-10-24)

- JUCE official features: https://juce.com/features/
- JUCE 9 official licence and plans: https://juce.com/get-juce/ ; https://juce.com/legal/juce-9-licence/
- Dear ImGui project scope: https://github.com/ocornut/imgui
- iPlug2 repository and licensing: https://github.com/iPlug2/iPlug2
- Steinberg VST3 SDK MIT: https://github.com/steinbergmedia/vst3sdk
- GitHub Actions billing: https://docs.github.com/billing/managing-billing-for-github-actions/about-billing-for-github-actions
- OpenAI Codex cloud continuity: https://help.openai.com/en/articles/20001545-using-codex-cloud
- Codex repository guidelines: https://developers.openai.com/cookbook/examples/codex/iterating-development-workflows-with-codex
- Existing code & behaviour: `README.md`, `docs/ARCHITECTURE.md`, `docs/VALIDATION.md`, `tests/test_reverb.cpp`.

**Document control:** document owner/product maintainer must approve framework licensing, public repository visibility, external release, and any irreversible compatibility changes. Engineering agents may implement within approved boundaries but must not represent unverified future features as completed.

## Addendum — 2026-10-28 (v0.2 implementation evidence)

A JUCE 9 VST3/Standalone host wrapper, editor, opt-in CMake target, GitHub Actions Windows plugin workflow, and a `presetValues()` single source of truth were added in source. The new `arverb_contract_tests` checks parameter IDs and presets in Release with live `require()` checks. Linux GCC/Clang Release and Clang ASan/UBSan core tests passed locally. **JUCE SDK was not present and no plugin binary was built, therefore this addendum must not be interpreted as host validation.** See `docs/STATUS.md` for the exact status and `docs/adr/0002-preset-ssot-and-plugin-host.md` for design constraints. Plugin build requires CMake >=3.22 and a C compiler, while core-only builds retain CMake >=3.18.

## Addendum — 2026-10-28 (v0.3 reliability and host testing)

No changes to the primary architectural decision: JUCE 9.0.4 remains a thin plugin/GUI adapter, while the algorithmic core stays C++17 and JUCE-free. v0.3 adds these measurable engineering requirements:

1. **Prepare-time invariants.** Calculate static delay lengths and sample-rate-dependent constants during `prepare()` rather than inside per-sample loops. Keep truly parameter-dependent changes smoothly interpolated without allocating.
2. **Pathological host buffers.** A non-finite or extremely large finite input must not permanently poison the recursive reverb. Sanitisation occurs before feedback injection. Do not silently hard-clip ordinary program material to 0 dBFS.
3. **Automation and state as host contracts.** Test the actual JUCE `AudioProcessor`: 13 stable parameter IDs plus bypass, sample-rate reprepare, process blocks, host bypass, DAW state XML round-trip, and bad state rejection. These tests must build in the Windows plugin workflow; local core success does not count as plugin validation.
4. **Objective test limitations.** Passing IR/stability tests says nothing about perceived vocal blending or sonic similarity to Valhalla. Subjective level-matched listening, acoustic measurements and DAW integration must remain separate acceptance gates.
5. **Provenance and handoff.** `docs/STATUS.md` lists verified test commands, numeric benchmark evidence, and blockers; GitHub commit SHA and workflow URLs must be recorded before claiming a remotely built binary.

v0.3 baseline performance: median offline wall-time for 30 seconds at 48 kHz decreased from 0.3148 s to 0.2740 s under the same Linux test environment (3 runs each). Do not extrapolate this result to a particular host, CPU, or system without a platform-specific benchmark.


## Amendment — v0.4 (2026-10-29): API/ABI and verification gates

- `project(VERSION)` is the canonical product version, exported to C consumers via `arverb_version()`; `ARVERB_ABI_VERSION = 1` is independent. ABI version bumps only on breaking changes, not on every feature release. Stable parameter ID order (13 DSP parameters, plugin bypass separate) must not be changed casually.
- Real C shared-library consumer test and deterministic DSP stress test are release blockers in core CI; see `tests/test_c_abi.c` and `tests/test_stress.cpp`.
- JUCE wrapper and host-contract test are retained but **unverified** with JUCE SDK, Windows DAW, or pluginval. Plugin CI must build all tests before any VST3 artifact is described as build-verified.
- No licensing, branding, or sonic parity claims may be inferred from passing numerical stress tests. Four current presets remain one FDN topology; design separate engines under a future reviewed DSP ADR.
