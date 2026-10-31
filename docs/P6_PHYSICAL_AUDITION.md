# ArSonVerb P6 — Mono Host and Physical Musical Audition

Date: 2026-10-31. P6 is a code/QA milestone after merged P5, not a sonic parity claim.

## Meaningful product outcome

- Native JUCE VST3 host layouts: mono input to mono output, **mono input to stereo output**, and stereo input to stereo output. Existing stereo engine samples stay unchanged.
- Mono input is routed to both sides of the *same* FDN; output is its mid (L+R)/2. Stack-bounded 128-frame chunks, no process allocations, heap, lock, worker or new latency. The normalized mono downmix explicitly removes side information. This does not magically prevent real-world phase cancellation.
- The real JUCE host test compares 1→1 output with 2→2 downmix, and 1→2 output with genuine dual-mono-fed 2→2 stereo on identical inputs across all four modes and odd buffer sizes. Unsupported mismatched layouts fail negotiation.
- Stable 13 parameter IDs, bypass, appended algorithm choice, 32 presets, C API and gold GUI preserved.

## Physical audition pack (built by existing Windows plugin CI)

Existing arverb_demo exports 8 six-second, 48 kHz, 32-bit stereo float WAV impulse responses: Classic, Plate, Chamber and Hall each with dual-mono and left-only excitation. All modes use exactly the same Vocal Plate Clean numeric preset, Mix 100%, Duck 0%. None of the WAVs is loudness-normalized.

The dependency-free Python script scripts/p6_audition_pack.py renders WAVs, checks headers/length/finite samples/non-silence, and records:
- P6_sonic_diagnostics.csv: peak/rms dBFS, L/R late correlation, mono-retained energy, and approximate octave-band Schroeder T20 x3 (250, 1000, 4000, 8000 Hz).
- P6_provenance.json: Git commit ID and SHA-256 hashes plus settings.
- READ_ME_FIRST.txt: diagnostic and DAW use.

Six-second truncation, modulation and insufficient decay range may make some T20 estimates unavailable (blank CSV/null JSON); do not invent numbers. No single correlation, mono or RT metric proves perceived quality or DAW-specific phase correctness. Static impulse playback cannot fully reproduce a time-varying reverb acting on continuous source audio.

## Physical Windows testing procedure

1. In repo GitHub Actions open latest successful **JUCE VST3 Windows Build**. Download the unsigned Windows VST3 + Standalone artifact and **ArSonVerb-P6-Physical-Audition-48k**. CI artifacts expire after seven days; neither artifact is a signed public release.
2. Extract complete VST3 bundle. For a DAW install it under Windows standard VST3 folder (Program Files / Common Files / VST3), or launch Standalone. No published license approval: do not publicly redistribute.
3. On a **mono vocal insert**, test both 1→1 and, if the host allows, 1→2 stereo expansion; start Mix 15–25%, Duck 0, compare algorithm modes and sweep Mix 0/25/50/75/100%. On a **stereo music insert**, repeat and fold output down to mono. Compare raw recorded audio with levels matched.
4. On an **aux/send**, leave dry voice in DAW and set plugin Mix 100% wet. Raise/lower send and FX return independently. Avoid a second copy of dry, inadvertent parallel insert or duplicate FX returns.
5. Compare high-cut 8/12/16 kHz, decay short/long, recorded vocal consonants and sibilants, guitar, piano and snare. Listen for metallic ringing, echo density, harsh HF, dark tail, frequency masking, and mono loss. The eight impulse files are for inspection, not musical listening certification.
6. Test Windows device driver at 48/96 kHz and buffer 32/64/256 frames. Record DAW version, ASIO/WASAPI device, CPU p99 if available, real xruns/dropouts, plugin count, host state persistence, bypass and preset recall. Report sample time and routing topology for each issue.

## Stop-ship gates

Mono host mismatch or silence; nonfinite/unstable output; unexpected loudness dip on insert; phase cancellation from real routing; harsh/metallic wet tail; real driver xruns; failed DAW state recall; unapproved distribution license; no provenance. Green CTests only confirm source behavior.

## CI and performance constraints

One Windows job, one supplementary audition artifact, no new job matrix; core Linux/Windows/macOS tests remain existing jobs. No new DSP or GUI repaint workload. Physical listening, real host validation and commercial reference A/B remain mandatory before claiming parity.

Public design references: [Steinberg VST3 Processing](https://steinbergmedia.github.io/vst3_dev_portal/pages/FAQ/Processing.html), [FabFilter Pro-R 2 manual](https://www.fabfilter.com/help/pro-r), [ValhallaRoom Mix](https://valhalladsp.com/2011/05/02/valhallaroom-the-high-level-sliders/).
