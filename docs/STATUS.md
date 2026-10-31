# RECONCILED — Pre-P1 performance baseline (2026-10-31)

- Canonical repository: `masarray/arsonverb` `main`. Last feature merge **PR #18** at `cb6cfa0aa7f27904a7ef43916099d74d329a2a32`; thereafter exactly two commits changed **only** `AGENTS.md` and `docs/STATUS.md`. Feature source and plugin build inputs were not changed after the verified merge.
- **All 16 recorded PRs (#1, #3, #5–#18) are merged; 0 open PRs.** 16 historical feature/fix branches remain as references to those already-merged PRs. Do NOT merge or delete them as unmerged work.
- **Post-merge main CI**: [Core C++ CI run 41174229314](https://github.com/masarray/arsonverb/actions/runs/41174229314) **SUCCESS** on Linux, Windows, macOS plus Linux ASan/UBSan; [JUCE Windows VST3 + Standalone run 41174229390](https://github.com/masarray/arsonverb/actions/runs/41174229390) **SUCCESS**. Earlier PR #18 run [41173756696](https://github.com/masarray/arsonverb/actions/runs/41173756696) ran 6/6 CTests successfully.
- Compatibility retained: 13 APVTS automation IDs, 32 programs, 8-line Householder FDN, C ABI and gold hardware GUI. Plugin reports zero **additional** latency samples; that is not proof of zero host/device buffer latency or freedom from callback deadline misses.
- **P1 NOT STARTED**. Existing `tests/benchmark.cpp` reports only an offline aggregate processing time; CPU p95/p99, 1/8/16-instance scaling, 32/64-sample worst cases, GUI redraw/resize timings and real hardware ASIO/WASAPI underrun behavior have not been measured. A future P1 may only proceed from the verified `main` source, with reproducible profiling before optimizing DSP or UI.
- This reconciliation is documentation-only: no engine, GUI, benchmark, build config or CI workflow changes; no new CI requested. Subjective listening and DAW screenshot acceptance remain separate.

---

# VERIFIED — PR #18 Musical Clarity DSP & Premium Header Polish (2026-10-31)

- [PR #18](https://github.com/masarray/arsonverb/pull/18) **MERGED** as `cb6cfa0aa7f27904a7ef43916099d74d329a2a32`.
- [Windows JUCE VST3/Standalone CI](https://github.com/masarray/arsonverb/actions/runs/41173756696) **SUCCESS: 6/6 CTests PASSED**, including presence-selective audio regression and real JUCE host/editor controls; unsigned artifact ID **13032193185**, 8,599,945 bytes (original archive).
- [Linux Release + ASan/UBSan CI](https://github.com/masarray/arsonverb/actions/runs/41173756668) **SUCCESS**.
- GUI: Mix name inset +16 design units; CUSTOM ComboBox text 16px rather than 9px left inset. No layout or asset regressions.
- DSP: Original stereo-linked presence detection ~1.25–4.2kHz, 10ms attack/180ms release; selectively ducks wet presence when input occupies it. Existing DUCK amount scales the effect; at DUCK=0 the new shaping is exactly output-transparent. Dry, feedback FDN, RT60, 13 host parameter IDs, all 32 factory programs, state format, C ABI and reported latency unchanged. No added lookahead, allocations, threads or locks in audio callback.
- Regression validates stronger attenuation around 2.5k than 230Hz, 0% duck transparency, stereo symmetry, finite samples, plus prior C ABI, host, automation, stress and geometry.
- **Not yet certified:** subjective vocal/music/instrument listening, actual host visual appearance at 100/125/150% DPI, level-matched parity versus commercial reverb, RT60 by frequency and named-hardware CPU p99.

---

# VERIFIED — PR #17 Precision Control & Direct LCD Value Editing (2026-10-31)

- [PR #17](https://github.com/masarray/arsonverb/pull/17) **MERGED** to main as `cfb635975ec43fdf663c569c2830c251fab85fa0`.
- [Windows JUCE 9.0.4 VST3+Standalone CI](https://github.com/masarray/arsonverb/actions/runs/41165637177) **SUCCESS**, **6/6 CTests PASSED**, including real JUCE host smoke testing: direct inline LCD editor commit, invalid input recovery, 13 precision rotary controls, Ctrl-wheel delta scaling, preserve stock wheel, existing geometry/presets/automation/C ABI. Artifact **13029208079**, 8,599,230 bytes, unsigned, expires 2026-11-07 UTC.
- [Linux Release and ASan/UBSan CI](https://github.com/masarray/arsonverb/actions/runs/41165637133) **SUCCESS**.
- Ctrl+wheel multiplies native JUCE wheel deltas by **0.125** (8× finer); unmodified wheel continues original JUCE behavior. Rotary drag sensitivity configured to 280px full-range.
- Persistent parameter LCD has separate name and double-click-editable numeric field. Enter commits, Esc cancels; accept native units, percentages, Hz/kHz shorthand; reject invalid and out-of-range. All edits pass via existing APVTS SliderAttachments.
- JUCE reports **0 additional plugin latency samples**; this does not eliminate host/device buffering, wet pre-delay, or acoustic reverb decay. No audio DSP changes, timers, worker threads, new GUI render loops, parameter-ID/preset/asset changes.
- **Manual acceptance still required** in Windows host: Ctrl-wheel physical sensitivity (mouse/trackpad), native inline edit, DAW automation, 100/125/150% DPI screenshot and listening.

---

# VERIFIED — PR #16 premium GUI UX and preset integrity (2026-10-31)

- [PR #16](https://github.com/masarray/arsonverb/pull/16) **MERGED** as `56612b67acdf0d43cc64c33409375c5b2d8f34f2`.
- [Windows VST3 + Standalone CI](https://github.com/masarray/arsonverb/actions/runs/41164159808) **SUCCESS**: MSVC build + **6/6 CTests PASSED**, including `arverb_plugin_host_tests` and all previous DSP, automation, stress, and C ABI tests. Artifact **13029265452** (`ARStudioReverb-Windows-x64-unsigned`, 8,595,817 bytes; expires 2026-11-07 UTC).
- [Linux Release and sanitizer CI](https://github.com/masarray/arsonverb/actions/runs/41164159788) **SUCCESS**.
- Preset ComboBox now displays `CUSTOM` if actual 13 APVTS values do not match any of 32 factory profiles; restores factory selection when values match again. Drag, wheel, double-click and host-driven attachment updates use a GUI-only bounded check. User-visible parameter LCD persists instead of disappearing at drag end.
- Emerald LED halo is drawn behind its recessed bezel. PR #15 approved 98/200 knob geometry, embedded PNG cache, 77%-inset moving LED, audio engine, parameter IDs, factory catalog and standalone ABI are **unchanged**.
- Host regression tests verify preset→CUSTOM→preset restoration at minimum/default/resized/max window geometry.
- **Remaining manual gate**: actual Windows GUI appearance at 100%, 125%, 150% scale and level-matched vocal/music listening. Headless CI is not proof of pixel-perfect luxury appearance.

---

# VERIFIED — PR #15 Decay gold gap / inset LED (2026-10-31)

- [PR #15](https://github.com/masarray/arsonverb/pull/15) **MERGED** at `b356a053faf6a6550506e9b4f65dfc071fe880d5`.
- [Windows VST3 + Standalone CI](https://github.com/masarray/arsonverb/actions/runs/41162733041) **SUCCESS: 6/6 CTest passed** including GUI geometry and JUCE host smoke test. GitHub unsigned artifact ID **13027862989**.
- [Core Release and sanitizer CI](https://github.com/masarray/arsonverb/actions/runs/41162733037) **SUCCESS**.
- Corrected apparent gold radius for 15% transparent PNG margin: Decay source image face factor `0.416 → 0.468`; small stays `0.422`. Visible gold to jade arc centre gap: small ~7.48, Decay ~7.44 reference units.
- LED center moved from old `side*0.344` to `side*vintageGoldVisibleRatio(side)*0.77`, seated on the gold disc rather than bezel. LED angle and continuous arc still driven by same normalized JUCE slider position.
- No audio DSP, host automation IDs, presets, parameter routing, window size, or gold asset edits. Existing two-size cached asset renderer preserved.
- **Remaining manual gate:** screenshot and listening QA in actual Windows host at 100%, 125%, 150% DPI; CI does not certify premium/pixel-perfect visual quality.
- Latest test ZIP derived from the CI binary with VST3 and Standalone only; compiler import libraries excluded.

---

# Design plan — verified in PR #15

# 2026-10-31 — Optical gold radius and recessed LED pointer correction

**Source:** user screenshot `image(20261031-025850).png` (actual 844×422 Windows screenshot).

## Root cause and mechanical correction
- `assets/gui/ArSonVerbGoldKnob.png` is an alpha PNG with metal reaching approximately 85% of its nominal image radius. Previous code compared **image bounding radius**, not **visible metal**, so Decay had a much larger perceived black gap to the jade track than the small dials.
- Preserve the already accepted 98-reference-unit small knobs and 200-reference-unit Decay hitboxes/centres. Change **only the rendered Decay gold face ratio** `0.416→0.468` (small remains `0.422`). Native JUCE image scaling/cache retained. This matches physical gold-to-arc centreline gaps: at 1100×510 design scale small `98×(0.435−0.422×0.85)≈7.48` and Decay `200×(0.435−0.468×0.85)≈7.44`. Also matches at the default 842×394 scale.
- Move the emerald LED dot away from the metallic/bezel boundary. Its location is calculated as **77% of the visible gold radius** (not the old fixed `0.344×sliderSize`); position and jade arc endpoint still use the *same* automated `vintageRotaryAngle`. The dot is separate native JUCE vector hardware, not baked into the PNG.
- If a PNG fails decoding, draw a fallback metal disc with the same visible radius so the arc stays readable.

## Stable engineering contract
- No changes to DSP, 13 host automation parameters, 32 real factory presets, C ABI, window size, visual groups, audio callback, existing logo, gold PNG, or pointer/arc update source.
- Source of truth is the `vintageGoldVisibleRatio()`/`vintageLedPositionRatio()` helpers in `PluginEditor.hpp`. Host contract checks normalized gold-to-arc gap at design and default scale, and verifies every LED dot remains inset including its bezel.
- One Windows JUCE VST3+Standalone PR CI and existing core Release/sanitizer only; merge if 6/6 CTest and artifact pass. Real Windows screenshot still required for aesthetic acceptance.


---

# VERIFIED — Golden Luxury Hardware Reference GUI (2026-10-31)

- [PR #14](https://github.com/masarray/arsonverb/pull/14) **MERGED** as `93f7a720419606d3764517aad5c95501fcdf7f08`.
- [Windows JUCE VST3 + Standalone run](https://github.com/masarray/arsonverb/actions/runs/41161532829) **SUCCESS**, all **6/6 CTest PASSED**, including host UI layout/knob renderer and parameter/DAW state tests. Unsigned artifact `ARStudioReverb-Windows-x64-unsigned` ID **13028020553** uploaded.
- [Core Release + sanitizers](https://github.com/masarray/arsonverb/actions/runs/41161532867) **SUCCESS**.
- Golden hardware visual: 12 small gold knobs at 98 design units; large Decay 200; luminous jade moving LED dot/continuous arc; resized metallic ArSon-Verb logo; bronze/champagne four-module faceplate; dynamic aligned LCD.
- Caching: faceplate once per resize; two gold-face caches divided at 125 drawing pixels. Existing 160×160 embedded alpha PNG is reused. No DSP, host parameter ID, C ABI, 32 presets, plugin/host state or audio callback modifications.
- **Still open for visual acceptance:** screenshots in real Windows Standalone and DAW at 100%, 125%, 150% DPI. CI tests do not establish pixel-perfect match with the user-approved illustration, nor Valhalla sonic parity.
- Next work must start from this verified code and user screenshot, not reintroduce tiny gold knobs or the moving thin ivory strip.

---

# Historical design plan — verified by PR #14

# Golden Hardware Reference — user-approved target (2026-10-31)

The 1774×887 user-selected image is the canonical screenshot for this **JUCE** GUI milestone. Treat it as a visual target, not as a substitute for a functioning editor. Existing ArSonVerb assets were approved earlier and are reused: `ArSonVerbLogo.png`, `ArSonVerbGoldKnob.png`.

**Visual geometry in the existing 1100×510 JUCE design space:** four unchanged grouped panels (Mix/Decay, Tone/Control, Shape, Space/Motion). Small knob diameter **98** reference units (formerly 74), Decay hero diameter **200** (formerly 172) with centre (295,253), coherent upper/lower rows y=214/360; the shape module remains 132 units wide. Captions remain controlled JUCE Labels. Decay LCD is still anchored to the PRE-DELAY label bottom; its new x=231/width=128 centers it under Decay. Upper-left logo box expands to 315×61; top row and header controls remain stable.

**Premium materials:** cocoa/bronze vignette chassis, champagne engraved double-line frames, satin taupe modules, stationary brushed-gold PNG physical surfaces, black recessed sockets, mint/emerald arc, and **large round luminous green pointer** (the pointer and arc endpoint use the same `vintageRotaryAngle` normalized slider state). No cyan tick bars, fake LEDs, static 12-o'clock pointer, bitmap rotation or WebView.

**Performance:** one cached chassis image rebuilt on resize and two cached gold-face surfaces (small/hero). Cache slot threshold 125px prevents the large-host small knobs from evicting the hero cache; no shader, timer, disk access, or changes to audio callback. Desktop-host rendering, real DPI/screenshot visual acceptance and listening QA still required beyond compile and 6/6 CTests.

**Regression:** host test draws 6 relevant knob sizes, validates cache split and LED size, all 13 sliders/32 presets, responsive bounds and Decay PRE-DELAY LCD alignment. No parameter IDs, presets, DSP code, C API or host state changes.


---

# VERIFIED — Luxury Gold Knob Native UI (2026-10-31)

- [PR #13](https://github.com/masarray/arsonverb/pull/13) **MERGED** as `d91d9381c6ae42e20ecd9bff1f4e52b93749ecf0`.
- [Windows VST3 + Standalone CI #41158987236](https://github.com/masarray/arsonverb/actions/runs/41158987236) **SUCCESS**: MSVC plugin/standalone compiled and **6/6 CTest passed**. Unsigned GitHub Actions artifact `ARStudioReverb-Windows-x64-unsigned` ID `13026661341`.
- [Core Release + sanitizer CI #41158987166](https://github.com/masarray/arsonverb/actions/runs/41158987166) **SUCCESS**; DSP core, C ABI, 13 APVTS IDs and 32 original factory programs unchanged.
- Official gold physical knob asset `assets/gui/ArSonVerbGoldKnob.png`: **160×160, 6256 bytes, transparent PNG** (Git blob SHA `5f1ed9233364bdef86a7eb271a72e4bdd5bcc294`). Exactly one BinaryData image across all 13 rotary controls.
- Native GUIs keep **real movable ivory pointer and jade level arc** drawn by JUCE. The gold physical texture stays stationary. Two pre-scaled image caches cover 12 small knobs + one large Decay. No new worker, animation timer, audio-thread allocation, or filesystem dependency.
- First Windows run failed only the unreliable pixel-sampling of an offscreen headless JUCE surface, **not compilation/DSP**. Final host regression checks the **decoded original PNG's transparent border and gold centre**, then calls LookAndFeel painting at 74/172 px without comparing peerless framebuffer pixels. `RectanglePlacement::stretchToFit` explicitly scales the PNG onto each physical face.
- **Remaining user QA**: check actual Windows Standalone and DAW screenshots at 100/125/150% DPI and audio listening. CI success does not prove pixel-perfect appearance or Valhalla sonic equivalence. For a direct test, download the unsigned Windows CI artifact or clean test ZIP.

---

# Historical Gold Knob Plan — CI verified by PR #13

# Gold Knob GUI — Verified Asset Integration Milestone (2026-10-31)

The user-selected premium champagne/brushed-gold physical knob is the source for **one** optimized native PNG: `assets/gui/ArSonVerbGoldKnob.png` (160×160, 6,256 bytes, original rgba transparent). PNG alpha on outer corners, physical metal only: NO pointer, NO level arc or ticks, NO rotation/animation in the asset.

- Single image decoded once per process on JUCE GUI thread via `BinaryData::ArSonVerbGoldKnob_png`.
- One `HardwareLookAndFeel` reuses **two** cached scaled images (12 equal-sized small knobs, one Decay knob); resize triggers high-quality resampling only when imageSide changes. No per-audio-sample processing and no extra thread, timer, shader, WebView or external filesystem access.
- Old generated graphite disk/concentric ring-render loops replaced with actual champagne/brushed-gold metal, while the dark recessed socket, jade progress arc and moving ivory pointer remain separate JUCE vectors.
- Same parameter-controlled angle for the pointer and arc endpoint as before. The physical gold surface stays stationary, so texture highlights do not orbit during automation.
- No change to 13 APVTS host automation identifiers, 32 preset profiles, bypass, DSP engine, or C ABI.
- Regression host test validates PNG decode, exterior alpha, warm source-center RGB and successful real LookAndFeel draw calls for 74 & 172 reference units. Peerless framebuffer pixel assertions are excluded after Windows headless false failures; screenshot QA remains manual.
- Windows VST3 + Standalone CI + six CTests must pass before merge. Visual match at actual DAW and 125/150% DPI remains for user inspection.


---

# VERIFIED — Metallic ArSon-Verb logo + responsive Decay alignment (2026-10-31)

- [PR #12](https://github.com/masarray/arsonverb/pull/12) **MERGED**, source merge commit `0e042c8004acd934b9255a93f811fb25d9bdba15`.
- [Windows VST3/Standalone run #41154696105](https://github.com/masarray/arsonverb/actions/runs/41154696105) **SUCCESS**: VST3 + Standalone compiled and **6/6 CTest passed**, including PNG alpha decode and PRE-DELAY/Decay LCD lower-edge alignment at four supported sizes.
- [Core Release + sanitizer run #41154696083](https://github.com/masarray/arsonverb/actions/runs/41154696083) **SUCCESS**; the DSP core/C ABI was not altered.
- Windows unsigned artifact ID **13024767856** (`ARStudioReverb-Windows-x64-unsigned`).
- Source logo: approved champagne-gold transparent ArSon-Verb artwork. Optimized **225×39 PNG, 3720 bytes** inside JUCE BinaryData, decoded once for all plugin windows. Header title/platform product identifier remains ArSonVerb for backward compatibility.
- Large Decay knob+caption and LCD shifted +15 reference-x; LCD bottom edge dynamically follows PRE-DELAY caption bottom (not a fixed Y), with the display and text sharing geometry.
- **Manual acceptance still open**: visual screenshots on Windows at 100/125/150% DPI and real DAW plugin loading/audio. CI proves functional correctness, not pixel-perfect design.

---

# Historical proposal — now verified by PR #12

# 2026-10-31 — Product logo and optical Decay layout

- Approved artwork: **ArSon-Verb** metallic champagne serif, original generated transparent PNG. Optimized alpha-indexed derivative embedded at `assets/gui/ArSonVerbLogo.png` (225x39, approximately 3.7 kB, no runtime external resources); the original high-resolution artwork remains the source for future large print exports.
- JUCE `BinaryData::ArSonVerbLogo_png` is decoded **once on GUI/message thread** and the cached image is drawn only when header repaints; audio core remains JUCE-independent, with no added IO or allocations inside the DSP callback.
- The old `drawText("ArSonVerb")` wordmark is replaced by the branded PNG at `scaled(25,15,240,42)`, preserving aspect ratio. Existing window title and host/product identifiers remain `ArSonVerb` for compatibility.
- The large Decay dial center moves right from 266 to **281** reference units (+15 = ~11 px at default). Its `DECAY` caption follows because all knob labels are generated from the same dial record.
- LCD box shifts x from 206 to **221** reference units, matching the dial offset. **Its bottom Y is no longer magic-number-based**: computed from the exact lower edge of the PRE-DELAY label. LCD number and frame reuse `ReverbEditor::decayDisplayBounds()` to prevent drift at 800×370, 842×394, 1000×470, and 1180×545.
- Host UI regression verifies alpha PNG decode/size and LCD alignment at all four dimensions. 13 parameter IDs, 32 factory programs, preset automation, state serialization, audio processing and routing remain unchanged.
- CI discipline: one integrated PR with Windows VST3+Standalone + 6/6 tests; retain manual screenshot checks at 100/125/150% display scale as a separate aesthetic approval.


---

# VERIFIED — Factory32 + Crisp Inter + Raised Decay LCD (2026-10-31)

- [PR #11](https://github.com/masarray/arsonverb/pull/11) **MERGED** as `5d971a5c9cc831426a484578758307e11d35daa0`.
- [Windows JUCE CI run](https://github.com/masarray/arsonverb/actions/runs/41148830105) **SUCCESS**: VST3, Standalone, and 6/6 CTest PASSED (core DSP, 32-preset contract, regression, stress, C ABI, and real JUCE host UI/menu + state tests). GitHub artifact `ARStudioReverb-Windows-x64-unsigned`, ID `13022790317`.
- [Core Release + sanitizer CI](https://github.com/masarray/arsonverb/actions/runs/41148830134) **SUCCESS**.
- Factory bank v1 = **32 original fully populated engine-native programs** with metadata; 10 Vocal, 10 Music, 6 Vintage, 6 Ambient, fully documented in `docs/FACTORY_PRESETS.md`. Previous factory IDs 0–3 and 13 host automation parameter IDs preserved.
- Embedded Inter **true 600 weight** via one cached variable Typeface clone replaces faux-bold. Integer font heights and minimum micro-caption sizes reduce text softness. Decay LCD background and text raised together by 13 reference units; wider preset selector fits longer program names.
- **Pending manual sign-off:** Windows GUI screenshot at 100%, 125%, 150% DPI; vocal/music listening tests and host pluginval. Passing CI does **not** prove pixel-perfect typography or sonic equivalence to Valhalla DSP.
- No new reverb algorithms, external dependencies, audio callback allocations, parameter IDs or state schema changes. Keep DSP core portable to C#/other hosts.
- Engineering workflow: reuse this verified baseline; avoid Windows CI reruns for documentation-only updates.

---

# Factory32 + true Inter 600 milestone — CI pending

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

# VERIFIED — Premium Bronze Control Surface (2026-10-31)

- [PR #10](https://github.com/masarray/arsonverb/pull/10) **MERGED** into main at `51f9c769b76d312aad417d2349ba541265299bad`.
- [Windows VST3 + Standalone run #41143173309](https://github.com/masarray/arsonverb/actions/runs/41143173309) **SUCCESS**: MSVC VST3 + Standalone built, **6/6 CTest PASS**, including real JUCE host-contract and extra small-knob socket geometry checks. Unsigned artifact **#13018978289** (ARStudioReverb-Windows-x64-unsigned) uploaded.
- Core C++ Release + sanitizer run [#41143173140](https://github.com/masarray/arsonverb/actions/runs/41143173140) **SUCCESS**.
- Current approved visual baseline: satin bronze/graphite chassis, champagne ivory labels, dark-taupe framed modules, muted-jade rotary light arc and proportionally deeper black socket for small knobs. Solid LCD preset with explicitly positioned Inter text, retro ACTIVE/BYPASSED without text compression.
- Visual test gate remains **OPEN**: manual Windows screenshot, 100/125/150% DPI and actual DAW/pluginval, because CTest proves stability rather than visual satisfaction. Do not claim Valhalla sonic parity.
- No DSP, C ABI, parameter ID, preset, plugin processor, or automation changes. Keep cached JUCE-only graphics.

---

# Historical design plan — now verified by PR #10

# Premium Vintage Color & Control Surface — 2026-10-31

Authoritative milestone after PR #9. Preserve the original 13 automatable DSP parameters and JUCE-independent C++ engine.

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

# VERIFIED — Brushed Metal Moving Pointer GUI (2026-10-31)

- [PR #9](https://github.com/masarray/arsonverb/pull/9) **MERGED** at commit `77fca6bed3f8b3162db6c6832e266f671558de82`.
- [Windows JUCE 9 VST3 CI](https://github.com/masarray/arsonverb/actions/runs/41140089146) **SUCCESS**: VST3 + Standalone compiled; **6/6 CTest PASS** including JUCE host-contract, pointer-angle unit cases, core and C ABI. Unsigned Windows artifact **#13017818722** uploaded.
- [Core CI](https://github.com/masarray/arsonverb/actions/runs/41140089142) Linux Release + ASan/UBSan **SUCCESS**.
- The previous amber tick ring and fixed 12 o'clock pointer are superseded. Current knobs have moving cream pointer aligned with the green continuous arc, dark milled recess, native fine concentric brushed-metal face, and exactly two face-cache slots.
- Default editor 842×394, compact four framed modules. Mix/Predelay row positions and all 12 small knob diameters unified; no 13-parameter IDs, presets, audio core, or C ABI modified.
- Windows test ZIP is an unsigned test build; pluginval, DAW visual screenshot, 125%/150% HiDPI and user listening are **NOT yet verified**. Do not state aesthetic parity with the reference until a screenshot test is reviewed.

---

# Active vintage knob fidelity milestone

## 2026-10-31 — Light Volume Knob fidelity / rotary feedback fix

**Reference:** user-provided `LIGHT-VOLUME-KNOB.png` (brushed concentric titanium face, deep black recessed bezel and continuous green level arc). The preceding amber segmented-ring design is superseded.

- The main physical pointer MUST rotate using the exact JUCE rotary angle: `start + clamp(sliderPos, 0..1) * (end-start)`; never draw a fixed 12-o'clock pointer.
- Progress arc starts at the rotary minimum and ends at **the same angle as the pointer**, so drag, mouse wheel, DAW automation and preset changes stay synchronized automatically.
- ONE continuous luminous green arc instead of 19/29 orange ticks. Track and lit arc line thickness are **both constant at 2.9 drawing units**, for small and large knobs; outer 4.5-unit soft glow keeps a physical light-ring appearance.
- Brushed metal is drawn in native JUCE and cached as only two tiny ARGB faces (small unified 74-unit diameter, large Decay 172-unit diameter). Repeated repaint adjusts only arc and pointer, not the machining texture. No external image, GPU texture download or animation timer.
- Compact default 842×394 retained; top Mix aligns with top rows, bottom Pre-delay aligns with bottom rows; uniform small control diameter. All four framed groups stay consistent.
- Six-test Windows regression adds deterministic pointer angle assertions at 0%, 25%, 50%, 75%, 100%, including clamping, plus existing offscreen paint, 13 APVTS IDs, host state and audio.
- **Never change the DSP topology, C API, parameter IDs or presets as part of the knob redraw.** Visual screenshot approval and real host/pluginval still require follow-up.


---

# ArSonVerb — VERIFIED vintage modular GUI milestone (2026-10-31)

- [PR #8](https://github.com/masarray/arsonverb/pull/8) **MERGED**; production-source merge commit: 39debeac77ebd18bc9f111c1083f03e6dcb05dc9.
- [Windows VST3 CI run 41137416007](https://github.com/masarray/arsonverb/actions/runs/41137416007): **SUCCESS**, VST3+Standalone MSVC build, **6/6 CTest PASS**, raw unsigned artifact #13017189533.
- Core+sanitizer run 41137415993 **SUCCESS**. Test suite verifies real JUCE host, automation, session state, resize bounds and offscreen paint calls; **pixel appearance still needs human screenshot review**.
- Physical visual system: fixed ivory 12-o'clock index, 19/29 amber segmented value LEDs, four fully-framed vintage hardware modules, classic power button with real APVTS bypass state, Inter bold on lighter rose/champagne panel faces.
- Native C++17 DSP and C API were not modified. Original 13 host parameter IDs, all four presets and mouse wheel preserved.
- First two Windows workflows exposed a **false headless alpha assertion** (not compiler/DSP failure). Resolved in final commit with peerless paint smoke test and strict independent control/bounds checks.
- User testing still needed at 100/125/150% Windows DPI, actual DAW/VST host and pluginval; **no visual or Valhalla sonic-parity claim based solely on CI**.

---
# 2026-10-31 — Pending CI: vintage modular GUI milestone
Canonical branch: feat/legendary-vintage-modular-control-surface
Knob matches reference #1 (segmented amber LED, fixed ivory calibration pointer); panel box layout follows reference #3; classic Bakelite power symbol; readable Inter bold.
Static faceplate + module contours cached on resize. No changes to reusable audio DSP, C ABI, 13 automation IDs, four presets or host state.
Read docs/GUI_DESIGN.md first. A successful Windows compile/host test and manual screenshot are separate gates; merge only after CI evidence.

---
# ArSonVerb current verified GUI — 2026-10-31

- [PR #7](https://github.com/masarray/arsonverb/pull/7) **merged** in main commit `bdf15ac46522a57aff0c06b424a5236099b50d16`. Native recessed blue/cyan-lit knob groove, rectangular LED pointers, refined brushed texture, engraved separators, ArSonMix-inspired tactile BTN180 bypass with correctly fitting BYPASSED label, bigger Inter fonts, scroll-wheel operation, black LCD-style preset/menu/tooltip and tighter wordmark.
- Inter font is embedded from the official Google Fonts mirror, with SIL OFL 1.1 in `assets/fonts/OFL-Inter.txt`. BinaryData is linked **only** by JUCE plugin and JUCE host test, not by standalone C++ DSP core.
- [Windows build](https://github.com/masarray/arsonverb/actions/runs/41129293853) **SUCCESS**: MSVC VST3 + Standalone build, CTest **6/6 passed**, unsigned artifact #13012891311 uploaded. Core Release and sanitizer [run](https://github.com/masarray/arsonverb/actions/runs/41129293855) **SUCCESS**.
- GUI stays at 842×394 default with min 800×370 and max 1180×545, and C++ reusable core/audio callbacks unchanged.
- **Pending gate**: actual Windows/DAW screenshots, visual fit on 125/150% DPI screens, pluginval, audio listening; CI alone does not verify visual perfection or Valhalla sound quality.
- Next-thread agents read `docs/GUI_DESIGN.md` and preserve stable 13 APVTS IDs, presets and JUCE-independent core. Avoid pointless CI reruns.

---

# Active GUI P4 milestone

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

---
# Verified GUI refinement — 2026-10-30

- [PR #6](https://github.com/masarray/arsonverb/pull/6) merged into main as commit `9469ce48b3e374cf4772d345f368b32fb2f300db`.
- [Windows JUCE VST3 + Standalone CI run](https://github.com/masarray/arsonverb/actions/runs/41069403126) **SUCCESS**, with **6/6 CTest passed**; unsigned GitHub Actions artifact #12984048469.
- [Core PR CI](https://github.com/masarray/arsonverb/actions/runs/41069403270) **SUCCESS** including sanitizer.
- Default compact editor 842×394; cached richer brushed-alloy finish with subtle dark lower panel. Short live slider values avoid JUCE default raw floating-point popup. Stable 13 parameter IDs and DSP untouched.
- Manual DAW screenshot visual QA / high-DPI and pluginval remain open. Do not infer sonic parity with Valhalla.

---

# Historical GUI planning note

## 2026-10-30 premium compact brushed alloy GUI
- Original user screenshot feedback: flat stripes, washed-out coral, oversized default, full-precision JUCE tooltip.
- New default 842x394, min 800x370, max 1180x545.
- Multi-stop warm alloy gradient, deterministic irregular brushed texture, darker lower third, painted into cached JUCE Image on resize only.
- Native giant full-float popup removed. Compact short unit-correct value readout in header appears during knob drag; permanent Decay display remains.
- Compile-time dial/DSP order guard, host resized bounds tests, stable parameter-ID checks.
- All 13 automatable controls, 4 presets, bypass, reusable DSP core untouched; check Windows GUI build before merging.

---
# ArSonVerb GUI visual milestone — 2026-10-30 (VERIFIED)

- [PR #5](https://github.com/masarray/arsonverb/pull/5) merged into main, squash commit `465876fa0064f8b3857046c58f8dd947386ea614`.
- [GitHub Actions Windows JUCE run #41063910932](https://github.com/masarray/arsonverb/actions/runs/41063910932): **SUCCESS**. Windows MSVC VST3 + Standalone built, **6/6 CTest PASS** including real JUCE host contract and layout bounds checks at 830x390, 860x460, 1100x510, 1440x680. Raw unsigned artifact #12981532508.
- Fast PR core CI (Linux Release and sanitizer) also **SUCCESS**: https://github.com/masarray/arsonverb/actions/runs/41063910926
- GUI changed from 7x2 regular grid to original hardware-inspired copper/coral horizontal faceplate, dominant Decay, charcoal knobs, green numeric readout, and Tone/Shape/Space groups. **All 13 DSP parameter IDs** remain attached. Four presets and host bypass remain available.
- No modifications to realtime DSP, presets, core ABI, processor serialization, or parameter IDs. Native JUCE vector painting, no large bitmap, WebView, animation timers, or added library dependencies.
- **Still unverified:** actual screenshot / manual visual acceptance in a DAW, pluginval, high-DPI scaling on multiple displays. Do not say visually pixel-identical to Valhalla. Reference is for layout only; original ArSonVerb art.
- For future GUI changes see `docs/GUI_DESIGN.md`. Do not introduce fake controls or change automation IDs for purely visual alignment.
- Note: main may run additional Windows CI after PR merge; the successful **PR head** run above is authoritative evidence for this GUI source, while main status must be checked separately.

---

# ArSonVerb hardware-inspired GUI milestone — 2026-10-30

- Target image supplied by user: warm brushed hardware faceplate, large Decay knob, small tactile black controls, compact green readout, spatial Tone/Shape/Space groups.
- Implemented in `plugins/juce_vst3/PluginEditor.*` with native JUCE vector drawing, no bitmap resource or WebView. Original ArSonVerb colors and wordmark; no third-party brand assets.
- All **13 DSP controls**, 4 selectable presets, bypass, APVTS attachments and parameter automation are preserved. Knob captions correspond to actual DSP parameters. Decay display updates from the attached slider; user knob drag clears stale preset selection.
- Message-thread-only drawing and responsive layout; no changes to C++ DSP / audio callback. No new runtime dependency.
- JUCE host test extended to assert visible controls remain in bounds at 830x390, 860x460, 1100x510, 1440x680.
- Validation: review the corresponding GitHub Windows plugin workflow on PR before asserting a built binary exists. DAW visual QA and hardware remain an outstanding acceptance gate.
- This is an original hardware-inspired visual redesign, not a pixel-perfect or branded copy of Valhalla.

---

# ArSonVerb — verified integration status (2026-10-29)

**Canonical repo:** https://github.com/masarray/arsonverb  
**Main integration SHA:** `14d8c28e80e9838038692645b5c7f90f6f3b4cd8`  
**Product:** ArSonVerb 0.4.1, JUCE 9.0.4 + independent C++17 DSP core.  
**Status:** Windows VST3 **compiled** and real JUCE host-contract CTest **passed**. **Not** yet pluginval-, DAW-, or hardware-validated; **not** sound-quality parity with Valhalla.

## Verified evidence

- [PR #1](https://github.com/masarray/arsonverb/pull/1) merged, MSVC C2131 fixed (`constexpr juce::Colour` replaced by runtime `const`). New plugin name and bundle metadata, stable existing JUCE manufacturer/plugin numeric codes and parameter IDs.
- [PR #3](https://github.com/masarray/arsonverb/pull/3) merged: optional offline `scripts/sonic_qa.py` and `docs/SONIC_BASELINE.md`; Python/SciPy **not** in realtime DSP or plugin.
- [Windows JUCE PR build](https://github.com/masarray/arsonverb/actions/runs/40789813680): VST3, Standalone, host smoke tests **PASS**; six out of six CTest **PASS**; unsigned artifact **uploaded** (artifact #12862115128).
- [Core PR run](https://github.com/masarray/arsonverb/actions/runs/40789813630): Linux Release + ASan/UBSan **PASS**.
- [Core main SHA run](https://github.com/masarray/arsonverb/actions/runs/40790715579): Windows, macOS, Linux and sanitizer CI **PASS**.
- [Windows plugin main run](https://github.com/masarray/arsonverb/actions/runs/40790715515): still in progress at time of this evidence update; confirm final status before claiming main plugin CI green.
- [Unsigned Windows artifact download](https://github.com/masarray/arsonverb/actions/runs/40789813680/artifacts/12862115128): includes `ArSonVerb.vst3` bundle and `ArSonVerb.exe`. Filename in workflow still uses historical `ARStudioReverb` identifier; internal product binary is ArSonVerb.
- Checked bundled `moduleinfo.json`: correct `Name=ArSonVerb`, `Version=0.4.1` and 2 classes. It is **valid JSON5**, as required by Steinberg VST3; trailing commas are permitted, so do not mistakenly treat them as a packaging defect.

## Next unpassed gates

1. Actual pluginval/Steinberg validator result from an appropriately configured Windows/macOS environment.
2. DAW install, scan, preset/state save/restore, automation, mono/stereo behavior and long-term tail test on hardware.
3. Level-matched blind listening, mono-compatibility and per-band decay review before any premium-sonic claims (see [Issue #2](https://github.com/masarray/arsonverb/issues/2)). Four presets still use **one FDN topology**.
4. Resolve source and JUCE distribution licenses prior to a public licensed binary release. Artifact is an **unsigned test build**, not a production release.

---

# ArSonVerb — current GitHub integration handoff

Updated: 2026-10-29 | Base commit: `33f35872e6de82e2c3972bd2e713dc9be0a662f5` | Branch: `feat/arsonverb-brand-msvc-ci-gates`

- Canonical repository: https://github.com/masarray/arsonverb (public). The historical statement below that GitHub was not published is superseded.
- Core C++ CI passed on base commit: https://github.com/masarray/arsonverb/actions/runs/40789129891
- First Windows JUCE VST3 CI failed at GUI compilation: MSVC C2131 in `PluginEditor.cpp` lines 6–9, because `juce::Colour` was used in `constexpr` variables. Build/run: https://github.com/masarray/arsonverb/actions/runs/40789129900
- Proposed fix: file-scope `const juce::Colour` rather than `constexpr`; preserve all four color values. Rename user-visible product to **ArSonVerb** (JUCE plugin code and APVTS parameter IDs unchanged). No current release is established; freeze identifiers before first external public release.
- CI economy: PR core jobs run Linux + sanitizer; Windows/macOS core jobs run on main and manual dispatch. Plugin Windows PR runs only for plugin/CMake/workflow changes; core changes remain covered on main and manual plugin dispatch.
- **This note is not evidence of a Windows VST3 pass.** Read the newest PR workflow logs/artifacts. Gate progression on actual compile + CTest + VST3 artifact; then pluginval and real DAW test. Do not publish sonic parity claims with Valhalla without level-matched listening measurements.

---

# AR Studio Reverb — cross-thread status

Updated: **2026-10-29** | Source milestone: **v0.4.0** | GitHub repo/CI: **NOT PUBLISHED / NOT RUN** | Validated Windows VST3: **NO**

## Latest verified work

- Source of truth: `AR-Studio-Reverb-v0.4.0-Engineering-Milestone.zip`, unpacked source. Base was v0.3 ZIP; no unrelated repositories modified.
- Local GCC 14 Release shared build + CTest **5/5**; Clang 17 Release shared build + CTest **5/5**; GCC 14 Debug ASan + UBSan core **4/4**.
- New C ABI version/parameter-count probes; product version generated from CMake project version; true shared-library ISO-C consumer test.
- Stress test: 1,500 blocks, randomized values of 13 parameters, 0/1/3/17/64/255/513/2048 frames, 8/44.1/48/96/192 kHz, NaN/Inf/FLT_MAX, finite feedback bounds, and heap-allocation interception on process().
- JUCE host test improved for GUI lifecycle/editor sizing; **JUCE is not installed in the local environment**, so plugin compile and DAW proof remain missing.
- Plugin Windows GitHub Actions updated to build all six test executables (five core/C ABI plus plugin host) and package VST3/Standalone **if it passes**. This workflow has **not run**.
- Added `scripts/build-windows.ps1` and `docs/QA_V0.4.md` for reproducible handoff.

## P1 exit blockers (unchanged)

1. Select/create dedicated AR Studio Reverb repository and publish exact source. GitHub `masarray` listings inspected, no dedicated project found; do not push into another project.
2. Run Windows MSVC GitHub Action and fix any **observed** build errors; capture SHA/logs/artifact.
3. Run JUCE host test, pluginval/Steinberg validator and at least one DAW scan/state reload.
4. Complete level-matched listening and spectral QA before premium-sound claims; still a single FDN topology.

## Evidence

See `docs/QA_V0.4.md`. Do not infer JUCE build/test success from core CTest success.

---

## Historical v0.3 handoff (archived below)

# AR Studio Reverb — canonical cross-thread status

Updated: 2026-10-28 | Source milestone: **v0.3.0** | Public release: **NONE**

## Authority / baseline

This package was based on `AR-Studio-Reverb-v0.2.0-JUCE9-Source-Milestone.zip`.
No dedicated AR Studio Reverb repository was found in the accessible `masarray` GitHub listings during this task. This is a **local source artifact**, not a GitHub commit. Do not assume CI has run or that a Windows VST3 binary exists.

## Verified local evidence (Linux, source v0.3)

- GCC 14 Release: CMake/Ninja core + C ABI shared library compiled; `ctest` **3/3 passed**.
- GCC AddressSanitizer + UBSan Debug: `ctest` **3/3 passed**.
- New deterministic `arverb_audio_regression_tests`: 4 preset wet impulse responses, 10 ms pre-delay gating, stereo tails, 5-second no-runaway window and recovery from NaN, Inf and FLT_MAX host input.
- DSP optimizations: immutable early-reflection taps, fixed FDN delay coefficients and ms->samples conversion are precomputed in `prepare()`. Clamp absurd host inputs to +/-32 before recursive feedback; preserves considerable above-0dBFS normal headroom.
- Offline 48 kHz / 30 s / 256-frame benchmark, same runner and release build, 3 repetitions:
  - v0.2: **0.3149, 0.3089, 0.3148 s**, median **0.3148 s**.
  - v0.3: **0.2853, 0.2729, 0.2740 s**, median **0.2740 s**.
  - Median wall-time decrease **12.96%** in this environment. Not a Windows/DAW CPU claim; audio output may differ slightly due to floating-point operation ordering.
- Existing C ABI, parameter ID, preset SSOT, no-heap processing and block-partition tests remain in the suite.

## Implemented source, NOT YET proven using the JUCE SDK

- JUCE 9.0.4 VST3/Standalone plugin wrapper remains source-only; no `.vst3` or `.exe` validated.
- Added `tests/test_plugin_host.cpp` using real `ReverbProcessor`: stereo bus, 14 host parameters (13 DSP + bypass), preset and automation changes, variable block lengths, host bypass callback, user bypass, APVTS XML state save/restore, corrupt state rejection and sample-rate reprepare.
- Updated `plugin-ci.yml`: build both plugin formats **and** host smoke target, run all CTest tests, upload unsigned artifact. This workflow has NOT run on GitHub.
- JUCE 9 SDK not present locally; cannot confirm compilation/API correctness until CI or a Windows developer workstation builds the project. Host test is source-level only today.

## Release blockers

1. Publish this exact source tree to a deliberately selected GitHub repository. Never overwrite another project's repository.
2. Run Windows 2022 MSVC JUCE 9.0.4 plugin workflow; fix any compiler errors from observed logs and attach successful VST3/Standalone unsigned test artifacts.
3. Exercise plugin in REAPER/Cubase or another actual DAW: scan, automated parameters, bypass, state roundtrip, editor resizing and shutdown/rescan.
4. Run Steinberg validator and/or pluginval as a separate release gate; verify actual results, including denormal and dynamic block sizes.
5. Real CPU benchmarking and level-matched vocal/music listening evaluation; four presets STILL share one FDN topology, not four algorithms.
6. Decide source-code license and review JUCE license before redistributing source or binaries. Freeze manufacturer/plugin IDs before first external release.

## Next meaningful milestone — P1 exit gate

The P1 milestone is **not complete** until a real GitHub Actions Windows build produces an installable VST3, host tests pass, and at least one DAW scan/session restore is confirmed. Avoid GUI embellishment and algorithm rewrites until this is demonstrated.

## Handoff

Read `AGENTS.md`, `docs/ARD.md`, `CMakeLists.txt`, and this file. Continue **from current source**, not memory. The next agent should inspect GitHub repository/PR/CI status, push this package only to the intended dedicated project, run plugin CI, resolve actual observed failures, record evidence here, and only then mark plugin compilation validated.
