# ADR-0001 — JUCE as primary VST host and editor; DSP independent

- **Date:** 2026-10-24
- **Status:** Proposed direction (awaiting maintainer approval; wrapper not yet implemented)
- **Owners:** Product maintainer + DSP/plugin engineering

## Context
A high-quality vocal/music reverb needs proven host lifecycle/state/automation, a compact polished interface and portable DSP usable in unrelated applications. Immediate-mode drawing libraries are not a substitute for a complete plugin framework. Future commercial distribution is possible but its licensing policy is not yet selected.

## Decision
1. Build the initial VST3 host/editor with JUCE 9, using CMake.
2. Keep `arverb_core` C++17 with **zero JUCE / ImGui / Steinberg dependencies**.
3. Permit Dear ImGui only as an optional development instrumentation/debug UI, not a customer-facing dependency.
4. Do not develop parallel iPlug2 wrapper until a specific licence or architectural trigger warrants migration.
5. Complete licence-tier review before sharing compiled closed-source plugins; JUCE 9 dual licensing is not the same as VST3 SDK's MIT license.

## Consequences
+ Good native host facilities, API and UI development ergonomics.
+ Reusable standalone/C ABI engine with isolated wrapper costs.
- JUCE licence needs ongoing review and compliance.
- JUCE wrapper code must be tested across hosts; framework choice does not guarantee glitch-free automation.

## Alternatives considered
- **Dear ImGui + raw VST3 SDK:** manageable for specialized tools, but extra editor window/input/accessibility/state integration without better audible reverb quality.
- **iPlug2 + IGraphics:** permissive zlib-like licence and attractive alternative for closed-source budget sensitivity; fewer code changes if core remains clean, but we will not maintain two plugin wrappers prematurely.
- **Raw VST3 SDK + VSTGUI:** technically valid, less opinionated, but higher host/UI glue ownership for the first product.

## Review trigger
Revisit when JUCE licensing becomes unacceptable, in reproducible major host incompatibilities, or when the first JUCE implementation fails measurable performance and maintenance criteria. Reference: `docs/ARD.md`.
