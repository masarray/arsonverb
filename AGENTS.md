# ArSonVerb — repository rules for coding assistants

Purpose: develop an original, high-quality reverb for vocal/music mixing, with a **reusable C++17 DSP core**. Keep instructions short; read the relevant source/tests before editing. For full rationale use `docs/ARD.md`. For verified current state use `docs/STATUS.md`. As of 2026-10-31, PR #10 is merged and Windows VST3+Standalone passed 6/6 tests (run 41143173309). Current approved baseline: moving rotary pointer + continuous jade progress arc on cached brushed-metal knob faces, deep black socket for small knobs, framed satin-bronze modules, crisp Inter LCD preset/power labels. See latest docs/GUI_DESIGN.md. Previous fixed 12-o'clock pointers, amber segmented LEDs and cyan neon rings are superseded. Preserve this without explicit user request.

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

Current verified release status: **v0.3 Linux core Release and ASan/UBSan tests 3/3 pass; JUCE VST3/Standalone and host-contract smoke-test source are authored but have NOT compiled or run using the JUCE SDK.** `.github/workflows/plugin-ci.yml` is unverified on GitHub until published/run.

## v0.4 continuation notes (2026-10-29)

- Read `docs/STATUS.md` and `docs/QA_V0.4.md` before starting. The v0.3 status preserved below the new v0.4 header is historical.
- CMake owns product version. ABI major is `ARVERB_ABI_VERSION=1`. Stable `arverb_parameter_count()=13`; never reorder `Parameter` values or `ARVERB_*` enum IDs without explicit migration.
- Always build with `-DARVERB_BUILD_SHARED=ON` when testing C ABI. In plugin-enabled CI, run all six tests (5 core/C ABI + real JUCE host smoke).
- The 4 preset names currently choose parameter bundles for **one FDN algorithm**. Never describe distinct Plate/Chamber/Hall engines as implemented.
- Canonical repository is `masarray/arsonverb`. Verify the current commit, workflow run IDs, and PRs before changing code; never infer pluginval success.
