# ArSonVerb — Vintage Modular Control Surface (2026-10-31)

## Visual contract and references
- Knob screenshot #1: fixed ivory vertical marker at 12 o'clock, discrete amber LEDs on an external black metal indicator ring, dark recessed Bakelite face.
- Layout screenshot #3 (Ventapulse): separate **fully framed modules**, dark etched double border, small header title and clear control grouping, not detached vertical separators.
- All graphics are newly drawn from basic JUCE primitives; no third-party logos or proprietary assets.
- Retro physical power button has amber illuminated power symbol when ACTIVE, dim when BYPASSED, keeping the single APVTS bypass state.
- Embed OFL Inter heavy weight in caption labels and section headers, dark foreground against warm rosy body.

## Layout in 1100 x 510 reference units (default 842 x 394)
| Panel | Rect x/y/w/h | Controls |
|---|---|---|
| Mix / Decay | 17/107/414/370 | Mix, Pre-delay, Decay, digital readout |
| Tone / Control | 439/107/254/370 | HF Damp, High Cut, Low Cut, Duck |
| Shape | 701/107/132/370 | Size, Diffusion |
| Space / Motion | 841/107/242/370 | Early, Width, Rate, Depth |

- Standard knob 19 amber segments, dominant Decay 29. The segments follow normalized APVTS values.
- Previous cyan neon arc/rotating cyan square and disconnected vertical separators are gone.
- Brushed-metal faceplate and module frame background are cached on editor resize; dynamic LED segments draw only on JUCE knob repaint. No timers, WebView or added DSP thread cost.
- No fake controls; preserve all 13 APVTS IDs, all mouse-wheel settings, four presets, host bypass and state serialization.
- Old waveform footer removed, because it overlapped the new framed panel area.

## Acceptance gates
- Windows MSVC VST3 + Standalone, all six CTest including an offscreen full-editor render test at 800x370, 842x394, 1000x470, 1180x545.
- Core Release and sanitizer CI without unnecessary new OS matrices for GUI.
- Actual DAW and high-DPI screenshots remain a separate manual visual QA gate; no claim of Valhalla sonic parity.

---
# ArSonVerb GUI Design & UX Contract

Updated 2026-10-30. Scope: JUCE 9 native VST3 / Standalone editor. Original artwork built in `plugins/juce_vst3/PluginEditor.cpp`; user-provided ValhallaVintageVerb screenshot inspired **only** layout density, knob hierarchy and hardware-panel gestalt. Never reuse Valhalla branding/logo, proprietary assets or misleading menu labels.

## Visual hierarchy
- Reference proportion: **1100 x 510**, resizable min **830 x 390** max **1440 x 680**.
- Whole-panel coral/copper brushed finish, small typographic ArSonVerb wordmark, premium matte charcoal rotary knobs with thin coral pointer, compact green digital Decay display.
- Left: Mix and Pre-delay; large Decay at center-left (primary control). Right: **Tone/Control**, **Shape**, **Space/Motion**, grouped by fine separators and short headers.
- Unintrusive header preset selector and genuine APVTS host Bypass toggle. No fake power indicator, dummy knob, or decorative control lacking effect.
- Fonts light/medium at compact sizes. Avoid overly bold headings, nested cards, oversized empty margins, white panels, heavy textures, WebViews, decorative animated visualizers.

## Exact DSP mapping (stable)
| Section | Controls | DSP parameter IDs |
| --- | --- | --- |
| Left / main | Mix, Pre-delay, Decay | mix, predelay, decay |
| Tone / Control | HF Damp, Low Cut, High Cut, Duck | damping, lowcut, highcut, duck |
| Shape | Size, Diffusion | size, diffusion |
| Space / Motion | Early, Width, Rate, Depth | early, width, modrate, moddepth |

Total: 13 controls plus host bypass. Four preset actions: Vocal Plate, Vocal Chamber, Music Hall, Airy Ballad. Existing parameter IDs/order and stereo DSP contract **must not change** as part of GUI work.

## Engineering performance
- `HardwareLookAndFeel` renders knobs vectorially using gradients and a single pointer. Faceplate is procedurally painted only when JUCE asks to repaint; no timer, audio callback hooks, network, or large bitmap allocations.
- No text boxes under each knob. Value is available via JUCE popup; Decay also has a compact readout connected to its slider.
- Double-click resets parameters to default; mouse wheel disabled to prevent accidental edits during DAW scrolling.
- User dragging a knob clears stale selected-preset state. Current preset menu represents a chosen preset, not a guessed match after DAW automation.
- Responsive anchors use a fixed design coordinate space and uniform knob scale. Add or adjust `tests/test_plugin_host.cpp` checks for each child at min, default, large and arbitrary host window sizes.
- Preserve APVTS SliderAttachment and ButtonAttachment. Never reach across into DSP from paint() or timer.

## Evidence and QA
- [PR #5](https://github.com/masarray/arsonverb/pull/5) merged.
- [Windows JUCE CI](https://github.com/masarray/arsonverb/actions/runs/41063910932): VST3 + Standalone built; 6/6 CTest passed.
- Manual DAW visual QA is still needed: screenshot at native and 125%/150% Windows scale; hover/drag/automation; switch preset and test green display; active/bypass; compare window 830x390, 1100x510, and 1440x680.
- If audio quality changes are requested, do it in the DSP roadmap, **not** by hiding or re-mapping existing parameters. See `docs/SONIC_BASELINE.md` and issue #2.

## 2026-10-30 premium compact brushed alloy GUI
- Original user screenshot feedback: flat stripes, washed-out coral, oversized default, full-precision JUCE tooltip.
- New default 842x394, min 800x370, max 1180x545.
- Multi-stop warm alloy gradient, deterministic irregular brushed texture, darker lower third, painted into cached JUCE Image on resize only.
- Native giant full-float popup removed. Compact short unit-correct value readout in header appears during knob drag; permanent Decay display remains.
- Compile-time dial/DSP order guard, host resized bounds tests, stable parameter-ID checks.
- All 13 automatable controls, 4 presets, bypass, reusable DSP core untouched; check Windows GUI build before merging.

## Verification / current shipped test GUI

- PR #6 merged on 2026-10-30: https://github.com/masarray/arsonverb/pull/6
- Windows MSVC VST3+Standalone, 6/6 CTest success: https://github.com/masarray/arsonverb/actions/runs/41069403126
- Core CI sanitizer passed: https://github.com/masarray/arsonverb/actions/runs/41069403270
- Preferred compact default 842×394; minimum 800×370; vintage brushed metal image cached on resize, no heavy assets; concise parameter readout shown only while dragging.
- Preserve native APVTS parameter IDs, all 13 controls, bypass and four presets; compare screenshot in DAW before concluding aesthetics are final.

## GUI polish P4 — 2026-10-31
- Inter variable font **embedded** as binary resource under SIL OFL 1.1 with full license file `assets/fonts/OFL-Inter.txt`. Installed system fonts are not required; no font source enters the DSP core.
- C++ LookAndFeel draws a *recessed machined metal socket* around every control, cyan tracking arc confined to its gap, and a luminous rectangular pointer. No animation or timer.
- More restrained deterministic metallic microbrushing: per-line alpha reduced ~3–6×, short scratches drawn once every four rows; cached faceplate remains resize-only.
- Engraved twin-tone separators stronger but not dark UI panel walls.
- Native analog implementation of ArSonMiX BTN-180: embossed surface with light OFF / soft-green ACTIVE color and momentary 1px press depth. BYPASSED text cannot overflow: allocated 138 design units (scaled).
- Embedded Inter labels increased to 15 design-font units, readable on compact size; scroll wheel enabled on every knob.
- Preset ComboBox, popup menu, tooltip and live value HUD use deep charcoal LCD with ice-cyan bright lettering.
- Wordmark ArSon/Verb anchors tightened from x=133 to x=106.
- Original all 13 APVTS parameter IDs, automation, 4 presets, bypass state and native audio DSP remain unchanged.
- **Acceptance gate**: Windows JUCE VST3 + Standalone build, host tests and complete 6 CTest must pass before merge. DAW visual screenshots remain manual.
