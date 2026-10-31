# ArSonVerb — repository rules for coding assistants

## P1 performance instrumentation — 2026-10-31

P1 profiling source is now in progress: `tests/benchmark.cpp` measures 54 audio-callback workload cases, `tests/test_plugin_host.cpp` optionally measures real-JUCE editor paint/resize, and `docs/PERFORMANCE_P1.md` defines how to repeat and interpret CPU p99. No current results are proof of real DAW glitch-free operation or real-machine p99. Do not tune DSP arithmetic, claim CPU improvements or change approved gold GUI until reproducible baseline and sonic regression evidence. CI quick profiling is informational, not a speed gate. Avoid new CI matrix jobs and do not add profiling overhead to the audio path.

## Pre-P1 reconciliation — 2026-10-31

PR #18 is the latest merged feature and **all 16 PRs in repository history are merged, 0 open**. The 16 remaining named feature branches are historical, not unmerged work. Post-merge CI on `main` passed: core Linux/Windows/macOS plus Linux ASan/UBSan (run 41174229314), and Windows JUCE VST3 + Standalone (run 41174229390). Latest feature source commit: `cb6cfa0aa7f27904a7ef43916099d74d329a2a32`. Keep original gold GUI, 13 IDs, 32 factory presets, DSP ABI, and bounded wet presence ducking. **P1 is not started**. Before optimizing, measure real callback p95/p99, CPU multi-instance scale, GUI redraw and audio-device underruns. Zero reported plugin latency does not prove zero end-to-end latency. See top of `docs/STATUS.md`.


Purpose: develop an original, high-quality reverb for vocal/music mixing, with a **reusable C++17 DSP core**. Keep instructions short; read the relevant source/tests before editing. For full rationale use `docs/ARD.md`. For verified current state use `docs/STATUS.md`. As of 2026-10-31, **PR #18 is merged, Windows VST3 + Standalone 6/6 tests passed** (run 41173756696). Preserve the user-approved golden hardware faceplate with 98-unit small knobs, 200-unit Decay, green moving LED dots, static embedded gold PNG and 125px two-size cache partition. Preserve the approved embedded champagne ArSon-Verb PNG logo and dynamic Decay LCD baseline, along with all previous automation and preset contracts. Verified PR #11 (2026-10-31, Windows 6/6 CTest) factory catalog has 32 original complete native-unit presets in src/Presets.cpp and docs/FACTORY_PRESETS.md; first four enum IDs unchanged. Never fabricate Valhalla algorithm modes. Verified visual milestone: official ArSon-Verb champagne PNG is embedded in JUCE BinaryData and replaces left header wordmark, with Decay LCD linked to the PRE-DELAY label bottom and Decay dial shifted +15 reference units. Keep the core unchanged. Verified PR #13: gold physical knob PNG is a single optimized 160px alpha asset embedded in JUCE and cached in two GUI sizes, never baked with rotating pointer/level lights. Preserve the existing 13 APVTS IDs and 32 factory presets. Verified precision input contract from PR #17: Ctrl+wheel is 8x finer, normal wheel must remain unchanged, the numeric LCD value edits on double-click with a strict native-unit parser; preserve 280px drag, host attachments, no audio work from GUI. Latest UX direction: the preset selector must reflect the actual 13 APVTS values (CUSTOM for edits, recognized factory match for restored states), and the compact header should continuously show the last changed parameter value without polling. Render LED halo behind its dark physical bezel. Latest visual correction: real 160px PNG gold occupies ~85% of source nominal radius. Decay rendered face ratio 0.468 vs small 0.422 equalizes *actual gold-to-arc clearance*; LED center at 77% of visible gold radius prevents bezel overhang. Keep these optical geometry invariants, arc/LED synchronized to same JUCE normalized value. Current approved baseline: moving rotary pointer + continuous jade progress arc on cached brushed-metal knob faces, deep black socket for small knobs, framed satin-bronze modules, crisp Inter LCD preset/power labels. See latest docs/GUI_DESIGN.md. Previous fixed 12-o'clock pointers, amber segmented LEDs and cyan neon rings are superseded. Preserve this without explicit user request.

## Non-negotiable

- Do not import JUCE, ImGui, Steinberg, GUI code, windowing or networking into `include/arverb/` or the audio DSP implementation in `src/`.
- No heap allocation/free, locks, I/O, logging, UI calls, blocking waits, exceptions or unbounded work in the audio callback.
- `prepare()` and `reset()` must not race with audio processing; one engine = one processing thread.
- Host automation numeric IDs cannot be renumbered after a public plugin release without an explicit migration.
- Do not claim Plate/Chamber/Hall have different DSP topologies in v0.3: currently these are presets on one topology.
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

Current verified release status: **v0.4.1 JUCE 9.0.4 Windows VST3+Standalone 6/6 CTests PASSED in merged PR #18 (workflow 41173756696); Linux Release and ASan/UBSan PASSED (workflow 41173756668)**. Unsigned artifact ID 13032193185. Existing PR #17 precision controls and PR #18 presence-aware wet Duck/premium header are both shipped in the test build. Plugin reports zero added latency samples, not zero device buffering. Real DAW screen and level-matched listening acceptance remain manual.

## v0.4 continuation notes (2026-10-29)

- Read `docs/STATUS.md` and `docs/QA_V0.4.md` before starting. The v0.3 status preserved below the new v0.4 header is historical.
- CMake owns product version. ABI major is `ARVERB_ABI_VERSION=1`. Stable `arverb_parameter_count()=13`; never reorder `Parameter` values or `ARVERB_*` enum IDs without explicit migration.
- Always build with `-DARVERB_BUILD_SHARED=ON` when testing C ABI. In plugin-enabled CI, run all six tests (5 core/C ABI + real JUCE host smoke).
- The 4 preset names currently choose parameter bundles for **one FDN algorithm**. Never describe distinct Plate/Chamber/Hall engines as implemented.
- Canonical repository is `masarray/arsonverb`. Verify the current commit, workflow run IDs, and PRs before changing code; never infer pluginval success.
