## Latest: PUBLIC ArSonVerb v1.0.1 native release (2026-11-01)

https://github.com/masarray/arsonverb/releases/tag/v1.0.1 is the **latest published stable** release: exact binary source commit `4c65f369348790bb6da7465deaffc2b3921fdeee`. Seven binary choices across Windows/Linux/macOS plus SHA256SUMS and AGPL source. Windows Inno Setup offers no-admin per-user optional VST3/Standalone and proper uninstall; Windows VST3 and portable standalone ZIP also exist. Linux has native VST3 and standalone tar.gz. macOS universal arm64+x86_64 has VST3 ZIP and standalone .app ZIP, ad-hoc sign NOT Apple Developer ID/notarization. Native run 41247949265 had all 3 OS jobs pass 12/12 JUCE host/core tests; publisher 41248747185 re-checked all hashes and published successfully. P9 audio DSP/13 automation IDs/32 presets unchanged. v1.0.0 remains immutable. Current main docs may be ahead of exact v1.0.1 tag; never mutate tag/release assets. Security warnings cannot be defeated by archive format, and code signing is absent on budget; be candid.

---

## LIVE PUBLIC STABLE RELEASE — v1.0.0 (2026-11-01)

**ArSonVerb v1.0.0 was published publicly** at https://github.com/masarray/arsonverb/releases/tag/v1.0.0, from exact source commit `4d4499bc664353a2c6f35e7c9ec3b26de9a372e0`. Windows x64 VST3 + Standalone ZIP, SHA-256 sums, corresponding tagged source and AGPL/OFL notices are available. Release workflow [41245558417](https://github.com/masarray/arsonverb/actions/runs/41245558417) succeeded 12/12 CTests and all publishing steps. Owner approved P9 listening results and explicit public stable. This replaces earlier statements that v1.0.0 is 'not released' or that physical approval is still pending. Continue improving as future versions; never silently mutate the published v1.0.0 tag or assets. No universal ASIO xrun-free/zero-round-trip claims; preserve 13 automation IDs, four algorithms, 32 factory presets and premium gold GUI.

## P9 current user-reported issue and explicit project licence

The copyright owner instructed GNU AGPL-3.0 open-source/free on 2026-11-01; full verbatim `LICENSE` is `AGPL-3.0-only`, JUCE 9 uses AGPLv3, Inter asset `assets/fonts/OFL.txt` is SIL OFL 1.1. Do not select proprietary/commercial licensing. **v1.0.0 stable publication remains BLOCKED** because the user hears crackles on plugin startup and rapid preset switching with P8. P9 PR #30 fixes engine warm-up, partial 14-parameter host states, callback transcendental spikes and delay tap teleportation. All physical QA claims must wait for device acceptance. Do not change the approved gold GUI, stable 13 host automation parameters, four modes, presets or physical layout.

## v1.0.0 public release candidate (not yet publicly distributed)

[PR #29](https://github.com/masarray/arsonverb/pull/29) merged as `488c51e`; Windows JUCE run [41222071166](https://github.com/masarray/arsonverb/actions/runs/41222071166) passed 11/11 host CTests and uploaded Windows VST3+Standalone artifact `13049869119`; Linux [41222071197](https://github.com/masarray/arsonverb/actions/runs/41222071197) passed Release and sanitizer. Footer is one native clickable JUCE HyperlinkButton `Created by Mas Ari - SonKuPik (c) 2026`, URL `https://www.youtube.com/@sonkupik` beneath modules, with original window bounds. DSP unchanged. CMake version 1.0.0. Manual stable GitHub publishing pipeline `.github/workflows/release-stable.yml` must NOT run without explicit compliant JUCE rights (AGPLv3 or commercial), owner outbound/source/font/image rights and actual physical driver QA; no public release exists yet. Do not pick outbound AGPL on owner's behalf. Keep source/CI handoff in `docs/STATUS.md` and `docs/RELEASE_V1.0.0.md`.

## P8 v0.7.0 — merged and PR CI verified, real device acceptance pending

[PR #28](https://github.com/masarray/arsonverb/pull/28) merged at `aef6cbc`. Linux Release + ASan/UBSan [41219722061](https://github.com/masarray/arsonverb/actions/runs/41219722061) SUCCESS 10/10; Windows JUCE VST3/Standalone [41219722058](https://github.com/masarray/arsonverb/actions/runs/41219722058) SUCCESS 11/11 including real JUCE restored-preset first-block test. Unsigned Windows artifact `13049931087`; eight current-P8 impulse WAVs `13049981061` (shared workflow still uses P6 name). P8 primes real APVTS host settings **before** core prepare; primes core coefficient targets/LFO steps; uses bounded 128-processed-frame last-value-wins coefficient coalescing, wet 7.8k top-air shelf and 60-cycle balanced heap test. Same-runner P7/P8 54-case CPU comparisons were mixed; do not claim speedup. Preserve GUI, 13 automation IDs, algorithm ID, 32 presets and C ABI. Workstation startup xrun, musical parity, DAW/pluginval and distribution acceptance remain OPEN. Handoff: `docs/P8_REALTIME_STARTUP_AIR.md`.

## P7 v0.6.0 sonic engine — merged and PR CI verified, physical acceptance pending

P7 [PR #27](https://github.com/masarray/arsonverb/pull/27) merged into main at `b1e7c6f`, intentionally modifying wet HF tail behavior. Windows JUCE VST3/Standalone [41212540202](https://github.com/masarray/arsonverb/actions/runs/41212540202) **SUCCESS 11/11 CTests** and 8 regenerated WAVs (test artifact IDs: unsigned plugin `13046572769`, QA WAV pack `13046897126`, 7-day retention); Linux Release+ASan/UBSan [41212540154](https://github.com/masarray/arsonverb/actions/runs/41212540154) **SUCCESS 10/10 core tests**, with same-runner P6/P7 54-case performance artifact `13047500106`. Existing eight FDN filters, C ABI, 13 scalar automation IDs, four modes, 32 factory values, mono/stereo JUCE layouts and approved gold GUI retained. `expm1` executes only on room size/decay/damping control-target change, not each sample. **Not a certified release:** physical workstation ASIO xruns, matched-loudness P6/P7 vocal/music listening, pluginval/DAW routing and outbound license remain open. Do not claim Valhalla/FabFilter parity. Handoff docs/P7_RT60_AWARE_HF.md.

## P6 v0.5.0 — merged and PR CI verified (2026-10-31)

PR #26 merged as `7cbc9c2` with native JUCE mono→mono, mono→stereo and unchanged stereo→stereo path, plus the 8-WAV measured physical audition tool. [Windows JUCE PR CI 41208990180](https://github.com/masarray/arsonverb/actions/runs/41208990180) succeeded with 10/10 actual JUCE CTests and both artifact uploads; [Linux Core PR CI 41208990172](https://github.com/masarray/arsonverb/actions/runs/41208990172) succeeded including ASan/UBSan. Unsigned Windows VST3/Standalone artifact ID 13045452835; P6 sonic audition artifact ID 13045597649 (7-day retention). P6 does not alter the sound-processing core, original 13 scalar host IDs, appended algorithm ID, 32 presets, gold GUI, or C ABI. Real workstation listening/mono AUX routing, pluginval, physical ASIO p99/xruns and distribution licence remain **OPEN**. Do not claim global sonic parity. Details: docs/P6_PHYSICAL_AUDITION.md.

# Current integration baseline (reconciled 2026-10-31)

**Canonical DSP baseline:** P5 source `8da785b` retained unchanged through P6 source merge `7cbc9c2` (PR #26); 24 PRs merged, 0 open at P6 reconciliation. Historic topic branches are NOT pending work. P1–P6 are integrated. Post-merge core CI 41205502461 and Windows JUCE CI 41205502513 succeeded. The **unsigned test build is NOT a certified release**: physical vocal/music audition, mono/phase routing, real-device ASIO/WASAPI p99/xruns, pluginval/other-host, and distribution license remain unresolved. Keep the 13 original scalar automation IDs and appended algorithm choice, 32 factory programs, VST3 state migration, original gold GUI and C ABI. Detailed handoff: `docs/STATUS.md` and `docs/P5_MUSICAL_ENGINE_AUDIT.md`. Only change DSP based on reproducible sonic acceptance gates, not visual polish or unvalidated parity claims.

## P5 blocking musical Mix/Air correction — merged and CI verified (2026-10-31)

User tested P4 and reported severe Mix drop and unmusical, airless sound. Never treat 9/9 earlier CI as auditory certification. P5 fixes linear Mix through cached sine/cos targets plus gain-space smoothing, a bright high-cut filter slope, bounded HF feedback damping and wet air shelf without new latency or GUI/host parameter IDs. This is a **deliberate sonic change**. PR #24 merged; Linux Release/sanitizer and Windows VST3+Standalone 10/10 CTests passed. Actual human listening/DAW routing diagnosis remains open. Do not call this Valhalla parity or measured cancellation cure until audio correlation testing is verified.

## P4 distinct FDN algorithm milestone — merged and CI verified (2026-10-31)

Classic uses the original P3 Householder path, Plate uses a normalized Hadamard, Chamber four independent orthogonal pair rotations and Hall an alternate Householder vector. One tank, 100ms matrix morph, zero new delay buffers and zero reported extra plugin latency. The new host algorithm choice follows the original thirteen IDs and bypass, so older DAW states explicitly restore Classic. Premium compact algorithm capsule; keep full factory browser. PR #23 merged; Windows JUCE 9/9 CTests, Linux Release and ASan/UBSan passed. Prior-session XML -> Classic regression verified. Real ASIO callback p99 and level-matched physical listening remain required; do NOT claim Valhalla/FabFilter parity. Details in docs/P4_ALGORITHMS.md.

# ArSonVerb — repository rules for coding assistants

## P3 musical early-to-late and bass modal decay — merged, 8/8 Windows verified

P3 changes the actual sound; the first 8-filter variant was rejected for ~31% overhead, and the lean modal variant was benchmarked in PR #22: a continuous early-to-late energy and stereo-crossfeed morph using the 6 existing taps are blended above Diffusion 0.62; the shared Householder projection gains a single one-pole low-bass absorption contour driven by existing Wet Low Cut control. Preserve original sample rates, controls, factory values, GUI, zero added plugin latency and all real-time no-allocation invariants. Avoid claims that numeric DSP stability proves musical beauty or Valhalla/FabFilter parity. Windows JUCE VST3+Standalone 8/8 CTests and Linux Release/sanitizers verified in PR #22. Manual level-matched hearing and physical ASIO CPU acceptance remain required. References in docs/SONIC_BASELINE.md.

## P2 measured optimization — merged, CI verified

Removed only 5 unused audio-rate smoothing values. Independent Decay/Rate/EQ coefficient smoothing is unchanged; the 8 consumed per-sample controls are still updated in the original order. Static gold logo and module lettering have moved to the JUCE faceplate cache; moving LED/arc remain live. Verified 27/27 float-bit-identical P1/P2 audio cases, 54 same-runner CPU AB, Windows JUCE 7/7 CTests, and Linux sanitizer. CPU median delta approximately 0%, so do not claim universal speedup. Real DAW screenshots and device xrun still need user verification. Do not claim realtime zero device latency.

## P1 performance instrumentation — 2026-10-31

P1 profiling infrastructure is merged in PR #20, and its CI is verified: `tests/benchmark.cpp` measures 54 audio-callback workload cases, `tests/test_plugin_host.cpp` optionally measures real-JUCE editor paint/resize, and `docs/PERFORMANCE_P1.md` defines how to repeat and interpret CPU p99. 54-case Linux CI CPU timings and 2-size Windows offscreen GUI paint measurements are preserved under docs/performance/ and summarized in docs/P1_RESULTS_20261031.md. Results are not proof of a glitch-free DAW or the user's physical workstation p99. Do not tune DSP arithmetic, claim CPU improvements or change approved gold GUI until reproducible baseline and sonic regression evidence. CI quick profiling is informational, not a speed gate. Avoid new CI matrix jobs and do not add profiling overhead to the audio path.


Purpose: develop an original, high-quality reverb for vocal/music mixing, with a **reusable C++17 DSP core**. Keep instructions short; read the relevant source/tests before editing. For full rationale use `docs/ARD.md`. For verified current state use `docs/STATUS.md`. As of 2026-10-31, **P5 PR #24 is merged, Windows VST3 + Standalone 10/10 tests passed** (run 41204928207), with post-merge Windows and cross-platform core CI passing (runs 41205502513 and 41205502461). Preserve the user-approved golden hardware faceplate with 98-unit small knobs, 200-unit Decay, green moving LED dots, static embedded gold PNG and 125px two-size cache partition. Preserve the approved embedded champagne ArSon-Verb PNG logo and dynamic Decay LCD baseline, along with all previous automation and preset contracts. Verified PR #11 (2026-10-31, Windows 6/6 CTest) factory catalog has 32 original complete native-unit presets in src/Presets.cpp and docs/FACTORY_PRESETS.md; first four enum IDs unchanged. Never fabricate Valhalla algorithm modes. Verified visual milestone: official ArSon-Verb champagne PNG is embedded in JUCE BinaryData and replaces left header wordmark, with Decay LCD linked to the PRE-DELAY label bottom and Decay dial shifted +15 reference units. Keep the core unchanged. Verified PR #13: gold physical knob PNG is a single optimized 160px alpha asset embedded in JUCE and cached in two GUI sizes, never baked with rotating pointer/level lights. Preserve the existing 13 APVTS IDs and 32 factory presets. Verified precision input contract from PR #17: Ctrl+wheel is 8x finer, normal wheel must remain unchanged, the numeric LCD value edits on double-click with a strict native-unit parser; preserve 280px drag, host attachments, no audio work from GUI. Latest UX direction: the preset selector must reflect the actual 13 APVTS values (CUSTOM for edits, recognized factory match for restored states), and the compact header should continuously show the last changed parameter value without polling. Render LED halo behind its dark physical bezel. Latest visual correction: real 160px PNG gold occupies ~85% of source nominal radius. Decay rendered face ratio 0.468 vs small 0.422 equalizes *actual gold-to-arc clearance*; LED center at 77% of visible gold radius prevents bezel overhang. Keep these optical geometry invariants, arc/LED synchronized to same JUCE normalized value. Current approved baseline: moving rotary pointer + continuous jade progress arc on cached brushed-metal knob faces, deep black socket for small knobs, framed satin-bronze modules, crisp Inter LCD preset/power labels. See latest docs/GUI_DESIGN.md. Previous fixed 12-o'clock pointers, amber segmented LEDs and cyan neon rings are superseded. Preserve this without explicit user request.

## Non-negotiable

- Do not import JUCE, ImGui, Steinberg, GUI code, windowing or networking into `include/arverb/` or the audio DSP implementation in `src/`.
- No heap allocation/free, locks, I/O, logging, UI calls, blocking waits, exceptions or unbounded work in the audio callback.
- `prepare()` and `reset()` must not race with audio processing; one engine = one processing thread.
- Host automation numeric IDs cannot be renumbered after a public plugin release without an explicit migration.
- In historical v0.3, Plate/Chamber/Hall were preset names using one topology; since P4 (#23), Classic/Plate/Chamber/Hall use genuinely distinct feedback matrices within one FDN tank. Do not imply independent full reverberator instances or unverified sonic parity.
- Never copy proprietary Valhalla code or suggest a sonic equivalence without level-matched listening evidence. Third-party code requires explicit licence review.
- Optimize for measured performance, clarity and maintainability; avoid speculative wrappers, graph abstractions, SIMD or background DSP threads.
- Keep GUI compact and coherent if editing the JUCE wrapper; no unrelated visual systems or bulky cards.

## Common commands

```bash
cmake -S . -B build-ci -DCMAKE_BUILD_TYPE=Release -DARVERB_BUILD_SHARED=ON
cmake --build build-ci --config Release --parallel
ctest --test-dir build-ci -C Release --output-on-failure
```

For Windows MSVC: `cmake -S . -B build-msvc -G "Visual Studio 17 2022" -A x64 -DARVERB_BUILD_SHARED=ON` then `cmake --build build-msvc --config Release` and `ctest --test-dir build-msvc -C Release --output-on-failure`.

## How to work

1. Identify current state from `docs/STATUS.md` and code, not chat history.
2. Keep work in a focused branch / PR; avoid changing unrelated systems.
3. Add or update tests for functional DSP changes; run applicable commands and report exact outcomes.
4. Update `docs/STATUS.md` with verified changes and limitations. Update ARD/ADR only when changing an architectural decision.
5. Never claim GitHub CI is green until actual GitHub Actions runs are observed.
6. Report the tested OS, command, actual output, known issues and next task for a clean new-thread handoff.

Current verified *CI build* status: **v0.4.1 / P5, merged PR #24, Windows JUCE9 VST3+Standalone 10/10 CTests passed**. Post-merge `main` Linux/Windows/macOS + sanitizer run [41205502461](https://github.com/masarray/arsonverb/actions/runs/41205502461), and Windows VST3/Standalone run [41205502513](https://github.com/masarray/arsonverb/actions/runs/41205502513), succeeded for source SHA `8da785b`; unsigned artifact `13044660087` (7-day CI retention). Follow-up `24c1f95` only changes documentation. **Not a release:** human audition, DAW routing/mono correlation, pluginval, real ASIO/WASAPI xruns, licensing and public distribution approval remain open. 0 plugin-reported added sample latency does not imply 0 device latency.

## v0.4 continuation notes (2026-10-29)

- Read `docs/STATUS.md` and `docs/QA_V0.4.md` before starting. The v0.3 status preserved below the new v0.4 header is historical.
- CMake owns product version. ABI major is `ARVERB_ABI_VERSION=1`. Stable `arverb_parameter_count()=13`; never reorder `Parameter` values or `ARVERB_*` enum IDs without explicit migration.
- Always build with `-DARVERB_BUILD_SHARED=ON` when testing C ABI. In plugin-enabled CI, run the complete current suite (10/10 at P5, including real JUCE host and Mix/Air regressions), not historical six-test assumptions.
- Since P4, four algorithm choices have distinct feedback matrices within **one** FDN tank (Classic, Plate, Chamber, Hall); 32 factory programs provide parameter settings and choose a mode in JUCE. Do not claim that separate complete physical room models or Valhalla-quality modes have been demonstrated.
- Canonical repository is `masarray/arsonverb`. Verify the current commit, workflow run IDs, and PRs before changing code; never infer pluginval success.
