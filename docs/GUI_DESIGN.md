# Gold Knob GUI — Verified Asset Integration Milestone (2026-10-31)

The user-selected premium champagne/brushed-gold physical knob is the source for **one** optimized native PNG: `assets/gui/ArSonVerbGoldKnob.png` (160×160, 6,256 bytes, original rgba transparent). PNG alpha on outer corners, physical metal only: NO pointer, NO level arc or ticks, NO rotation/animation in the asset.

- Single image decoded once per process on JUCE GUI thread via `BinaryData::ArSonVerbGoldKnob_png`.
- One `HardwareLookAndFeel` reuses **two** cached scaled images (12 equal-sized small knobs, one Decay knob); resize triggers high-quality resampling only when imageSide changes. No per-audio-sample processing and no extra thread, timer, shader, WebView or external filesystem access.
- Old generated graphite disk/concentric ring-render loops replaced with actual champagne/brushed-gold metal, while the dark recessed socket, jade progress arc and moving ivory pointer remain separate JUCE vectors.
- Same parameter-controlled angle for the pointer and arc endpoint as before. The physical gold surface stays stationary, so texture highlights do not orbit during automation.
- No change to 13 APVTS host automation identifiers, 32 preset profiles, bypass, DSP engine, or C ABI.
- Regression host test validates embedded PNG decode/alpha and physically warm metal center color when rendering real LookAndFeel at 74 & 172 GUI reference units.
- Windows VST3 + Standalone CI + six CTests must pass before merge. Visual match at actual DAW and 125/150% DPI remains for user inspection.


---

# 2026-10-31 — Product logo and optical Decay layout

**Verified in PR #12:** Windows VST3 + Standalone, 6/6 CTest PASSED: https://github.com/masarray/arsonverb/actions/runs/41154696105

- Approved artwork: **ArSon-Verb** metallic champagne serif, original generated transparent PNG. Optimized alpha-indexed derivative embedded at `assets/gui/ArSonVerbLogo.png` (225x39, approximately 3.7 kB, no runtime external resources); the original high-resolution artwork remains the source for future large print exports.
- JUCE `BinaryData::ArSonVerbLogo_png` is decoded **once on GUI/message thread** and the cached image is drawn only when header repaints; audio core remains JUCE-independent, with no added IO or allocations inside the DSP callback.
- The old `drawText("ArSonVerb")` wordmark is replaced by the branded PNG at `scaled(25,15,240,42)`, preserving aspect ratio. Existing window title and host/product identifiers remain `ArSonVerb` for compatibility.
- The large Decay dial center moves right from 266 to **281** reference units (+15 = ~11 px at default). Its `DECAY` caption follows because all knob labels are generated from the same dial record.
- LCD box shifts x from 206 to **221** reference units, matching the dial offset. **Its bottom Y is no longer magic-number-based**: computed from the exact lower edge of the PRE-DELAY label. LCD number and frame reuse `ReverbEditor::decayDisplayBounds()` to prevent drift at 800×370, 842×394, 1000×470, and 1180×545.
- Host UI regression verifies alpha PNG decode/size and LCD alignment at all four dimensions. 13 parameter IDs, 32 factory programs, preset automation, state serialization, audio processing and routing remain unchanged.
- CI discipline: one integrated PR with Windows VST3+Standalone + 6/6 tests; retain manual screenshot checks at 100/125/150% display scale as a separate aesthetic approval.


---

## 2026-10-31 — Text clarity + Factory32 final bank

- LCD Decay box and its numeric text **both move upward by 13 reference units**, about 10 pixels at 842x394, preserving center alignment.
- Native JUCE Inter variable font uses a true 600 weight clone, not synthetic `.boldened()`. Font heights round to integer pixels; fixed minimum heights protect labels and module headers in compact view. Tooltip no longer squashes text with `drawFittedText`.
- Factory preset catalog is a shared no-allocation C++ facility: `presetInfo(Preset)`, `presetValues(Preset)`, `Engine::loadPreset(Preset)`. Total **32 original complete programs**, each with 13 native units: Vocal(10), Music(10), Vintage(6), Ambient(6). See `docs/FACTORY_PRESETS.md` for the full inspectable parameter table.
- First 4 preset enum IDs and actual values remain unchanged; new 28 are appended.
- Menu categories and selectable names are generated directly from catalog. Host automation and VST3 state serialization remain the single parameter source of truth.
- Valhalla VintageVerb has distinct modes absent here: the new profiles are tuning starting points for the existing ArSonVerb core, not Valhalla preset copies. Listening QA still required.
- Tests: core checks all 32 names, ranges, engine roundtrip and finiteness with one prepare; host verifies 32 selectable items and exact parameter propagation. No extra audio threads, plugin modes, or DSP callbacks.
- One PR, then Windows CI; merge only after 6/6 CTest.


---

# Premium Vintage Color & Control Surface — 2026-10-31

Authoritative production design implemented in PR #10 (merged). Windows VST3 + Standalone and 6/6 tests passed: https://github.com/masarray/arsonverb/actions/runs/41143173309 . Preserve the original 13 automatable DSP parameters and JUCE-independent C++ engine.

## Visual tokens (original ArSonVerb artwork)
- Chassis: satin dark bronze `#584840` → cocoa `#493B36` → graphite `#30292A`.
- Module face: warm taupe `#675851` → `#4A403C`, champagne border `#AA8D72`.
- Text: main ivory `#F6EBDB`, secondary champagne `#D6C6B3`, embedded Inter heavy.
- Hardware: charcoal metal `#101214` with brushed gunmetal center.
- Meter: restrained jade `#73C59E`, replacing neon green; power only: amber `#E6B978`.
- Preset + tooltip: deep LCD charcoal `#111719`, fully opaque text, pixel-aligned solid fill.
- Text contrast on darker panels targets W3C WCAG 2.2 AA minimum (4.5:1 for ordinary text) as a visual accessibility benchmark.

## Mechanical proportions
- Small knob faceRadius = 0.316×control side; grooveRadius = 0.480×control side, increasing visible black metal ring.
- Large Decay knob retains faceRadius = 0.355×side and grooveRadius = 0.465×side.
- One 2.9-unit smooth arc line thickness for both sizes; green changes to softened jade.
- Milled metal face cached in two dimensions only. All 12 small knobs share one cache slot; large Decay another.
- Group positions follow consistent 155-reference-unit vertical row separation, with 110-reference-unit horizontal spacing in two-column panels.
- Boxed modules and compact 842×394 default retained. Preset/power share same y baseline and pixel-aligned bounds.

## Crisp labels and controls
- Native JUCE ComboBox has explicit positionComboBoxText with safe side padding and Inter Bold 13.2px.
- Black LCD avoids translucent/shadowed type; preset arrow is one thin clear chevron.
- Power is solid Bakelite/metal with one warm lit power glyph when ACTIVE; status uses direct drawText, never auto-squeezed drawFittedText.
- All important text uses full-opacity warm ivory and sufficient contrast on bronze.
- No new image assets, WebView, UI timers, worker thread, GPU shader or audio callback work.

## Validation
- PR-scope Windows VST3 + Standalone CI once on completed commit, 6/6 CTest including strict host controls/geometry/state and pointer behavior.
- Core Release and sanitizer green required; no change to DSP/C ABI.
- Manual screenshots at min/default/full and 125/150% DPI remain open for visual approval; do not claim UI perfection on CI alone.


---

# ArSonVerb — Vintage Modular Control Surface (2026-10-31)

**Latest verified design:** moving cream pointer + smooth green continuous level arc, cached concentric metal face (PR #9, 6/6 Windows CI PASS): https://github.com/masarray/arsonverb/actions/runs/41140089146 . Previous amber segmented LEDs and fixed 12-o'clock pointer are obsolete. Keep framed modules and classic power button.

## 2026-10-31 — Light Volume Knob fidelity / rotary feedback fix

**Reference:** user-provided `LIGHT-VOLUME-KNOB.png` (brushed concentric titanium face, deep black recessed bezel and continuous green level arc). The preceding amber segmented-ring design is superseded.

- The main physical pointer MUST rotate using the exact JUCE rotary angle: `start + clamp(sliderPos, 0..1) * (end-start)`; never draw a fixed 12-o'clock pointer.
- Progress arc starts at the rotary minimum and ends at **the same angle as the pointer**, so drag, mouse wheel, DAW automation and preset changes stay synchronized automatically.
- ONE continuous luminous green arc instead of 19/29 orange ticks. Track and lit arc line thickness are **both constant at 2.9 drawing units**, for small and large knobs; outer 4.5-unit soft glow keeps a physical light-ring appearance.
- Brushed metal is drawn in native JUCE and cached as only two tiny ARGB faces (small unified 74-unit diameter, large Decay 172-unit diameter). Repeated repaint adjusts only arc and pointer, not the machining texture. No external image, GPU texture download or animation timer.
- Compact default 842×394 retained; top Mix aligns with top rows, bottom Pre-delay aligns with bottom rows; uniform small control diameter. All four framed groups stay consistent.
- Six-test Windows regression adds deterministic pointer angle assertions at 0%, 25%, 50%, 75%, 100%, including clamping, plus existing offscreen paint, 13 APVTS IDs, host state and audio.
- **Never change the DSP topology, C API, parameter IDs or presets as part of the knob redraw.** Visual screenshot approval and real host/pluginval still require follow-up.



**Verified:** PR #8 merged; Windows VST3+Standalone and 6/6 CTest successful: https://github.com/masarray/arsonverb/actions/runs/41137416007 . User visual acceptance in a real DAW remains open.

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
