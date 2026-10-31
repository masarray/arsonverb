# P8 — Real-time startup preparation, bounded automation and airy wet top
2026-10-31 | v0.7.0 DEVELOPMENT candidate | feat/p8-rt-startup-air-20261031

## User-reported symptoms and scope
- User: reverb is promising but not luxurious; top air lacks sparkle. Crackles occur at playback startup despite sample-rate configuration on a high-spec Windows workstation.
- P7 (PR #27) improved decay-dependent HF feedback; that does NOT prove hardware xrun freedom or a clear 10–16 kHz wet sheen.
- No evidence yet that GPU, host driver or actual ASIO buffer is the fault. We must not claim hardware crackles are fixed without a physical test.

## Source-level root causes targeted
1. P7 prepared delay memory and some coefficients, but left all cached control keys at -1, target gains at zero and oscillator steps uninitialized. The *first* processBlock performed eight feedback exponential calculations, eight oscillator sine/cosine pairs and several cutoff exponentials. This is needless work at the most timing-sensitive moment; it also depends on block size.
2. Automation for Decay/Size/Rate/Damping/low/high cuts recalculated all expensive coefficients **every callback**, even at 32-frame/192kHz (0.167 ms blocks). Caching only exact values does not prevent aggressive automation deadline spikes.
3. The P5/P7 wet air shelf was centered ~6.5k with only ~0.14x lift at default HighCut=10k. HighCut above 5.5k already transitioned to a gentler single-pole slope; additional bounded top-air was missing.

## Implementation
- In `Engine::prepare()`, initialize all coefficient targets, caches, gain targets and all eight oscillators. No first-callback coefficient cache miss.
- Coalesce the six expensive coefficient groups to **one update per >=128 processed samples** when parameters change; last current host values win; no background worker or parameter event queue. Mix and algorithm selection still update every host callback, and all continuously smoothed gains/filters stay audio-rate. Maximum extra control-target waiting time is below 128 sample periods (2.67ms at 48k, 0.67ms at 192k), not signal latency.
- Derive wet top air from **both** HighCut openness and Damping Hz. High-cut below 5.5k disables the new shelf, bright rooms cap lift at 0.72 (+4.7dB theoretical shelf asymptote). Wet-only shelf corner moves to 7.8k, leaving dry/mid and FDN stability alone. No fake spectral exciter, generator, shimmer, latency, parameter IDs or GUI changes.
- `reset()` clears the coefficient cooldown and temporal DSP state; no background processing. Delay storage remains preallocated at `prepare`, so no per-block heap allocations.

## Objective gates and limits
- Existing no-allocation random automation stress + NEW 60 prepare/reset cycles with balanced global new/delete counts.
- Cold-process 2048-frame output matches 2048 single-frame calls; real wet 9.8k/500Hz tone pairs confirm HighCut opens the top band more than the low-frequency body for Classic/Plate/Hall.
- Same runner 54-case P7→P8 callback benchmark only on PR branch, using the P7 parent source and existing Linux job. Metrics may be noisy; no universal speedup claims or extra workflow matrix.
- Existing Linux Release + sanitizers and Windows JUCE VST3/Standalone host/QA workflow must pass before merge. Keep C ABI, 13 automation IDs, P4 algorithm ID and 32 presets unchanged.

## Real workstation acceptance is REQUIRED
Use a matched-level mono vocal 1→2 and stereo music 2→2 at 48/96k with true ASIO device. Run buffer sizes 32/64/128/256; save xrun count and audio onset recording for first start, stop/start, preset changes and 8+ parallel instances. Listen to 10–16k sheen, sibilance and cymbal tails on Vocal Plate/Air Bloom/Music Hall against the P7 build at equal return loudness. Explicitly check driver clock, buffer/device selection, GPU activity and another host if crackle continues. The test build is NOT a public release, pluginval result or a promised hardware cure.
