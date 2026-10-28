# ADR-0002 — JUCE host bridge and preset single source of truth

Status: PROPOSED — implementation source drafted, runtime JUCE verification pending. Date: 2026-10-28.

## Decision

1. Preserve `arverb::Engine` as JUCE-independent.
2. Pin first plugin build to JUCE release tag `9.0.4` (CI checkout); avoid implicit branch upgrades.
3. Use `AudioProcessorValueTreeState` only in plugin wrapper for stable host automation and state; `Engine::setParameter` handles lock-free audio target changes.
4. Define immutable symbolic parameter IDs in `ParameterInfo`, plus additional host-only `bypass`.
5. Define fixed preset values once in `arverb::presetValues(Preset)`; use from both core and JUCE host; do not construct `Engine` per GUI preset click.
6. Process stereo audio in-place with no temporary audio buffers. Use mix smoothing for bypass and preserve feedback-tail history.
7. Use JUCE native Components for compact, resizable production GUI. No ImGui in the audio callback or plugin runtime.

## Caveats and validation

- Preset application uses sequential host automation gestures. It is NOT sample-atomic; audio thread may observe intermediate parameter sets. This is acceptable for first functional prototype, but must be redesigned if sample-exact preset snapshots are required.
- Do not assume APVTS `copyState()` is RT-safe. The host's state callbacks are not part of our DSP hot loop, but still require real host behavior tests.
- VST3 plugin identity codes in CMake are **provisional**. Lock down before public releases, or DAW session recall may break.
- JUCE 9 build, editor accessibility/resize, bypass behavior and state round-trip are NOT confirmed until CI and host validation.
- Changes to parameter ID strings require explicit migration tests; no reordering of enum values.
