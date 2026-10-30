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
