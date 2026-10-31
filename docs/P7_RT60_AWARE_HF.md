# P7 — Decay-Aware High-Frequency Feedback / Sonic Tail Milestone

Date: 2026-10-31. Engineering branch: feat/p7-rt60-aware-hf-tail-20261031.

## Problem and source audit

P5 successfully avoided the harsh double low-pass output slope and fixed disappearing linear Mix, but the FDN high-frequency damping amount was independent of both reverb Decay (RT60) and Size: a fixed feedback loss from 5% to 43% **at every feedback traversal**, regardless of whether a room was short or long. The same preset can therefore have disproportionate high-frequency shortening when Decay increases, even with a bright High Cut. This is an audible sonic-policy change, not a performance-only refactor.

The P6 impulse measurement project now provides 4-mode, 2-excitation repeatable audio plus per-frequency T20 estimates. It is not a human hearing test, but allows a rational next sonic change.

### Public professional benchmark principles (behavior, not proprietary DSP)
- [ValhallaRoom Late controls](https://valhalladsp.com/2011/05/18/valhallaroom-the-late-controls/): high/low frequency tails, late crossfeed, and modulation are musical dimensions separate from the high-cut output EQ.
- [FabFilter Pro-R 2 Decay Rate EQ](https://prod.fabfilter.com/help/pro-r/using/decayrateEQ): decay varies by frequency and is independently shaped from post EQ.
- [FabFilter Pro-R 2 reference](https://www.fabfilter.com/help/pro-r): perceptual target is a clear, dense and natural-sounding space, not arbitrary increases in wet highs.

This is NOT an implementation of Pro-R 2's six-band decay EQ. It is a low-cost original **HF decay-coupled shelf** using existing ArSonVerb controls and existing 8-line FDN topology.

## P7 implementation contract

For Decay `T`, Size `S` and existing Damping frequency `D`:
1. Compute a target high-frequency / midband RT60 fraction `R = 0.58 + 0.40 * clamp((D - 800)/17200, 0, 1)`. This is 58%–98% of the Decay target, not an additional public automation parameter. Damping still moves the existing low-pass corner.
2. Use the **mean** existing eight FDN line delay lengths, `L` seconds, scaled by Size: `L = mean(kDelayMs) * .001 * S`.
3. At control-target changes only: `loss = 1 - exp[-ln(1000) * L / T * (1/R - 1)]` using `expm1` for precision when the loss is small. Clamp to bounded [0,0.92].
4. Reuse the existing smooth `hfDampingBlend` and eight LP histories: `feedback += loss * (lowpassed_feedback - feedback)`; no heap, locks, extra delay, FFT, dynamic buffers or new controls in the audio loop.
5. Existing midband RT60 loop-gains `g_i` and four orthogonal matrices are untouched, and the original 13 scalar IDs, appended mode, C ABI, 32 factory numeric presets, premium gold GUI, and plugin zero-*added*-latency report remain unchanged.

**Mathematical limitation:** the calculated R is a representative high-frequency decay *target*, not a guaranteed exact measured RT60 at every frequency, because lines have different lengths, modulation changes spectral content and the damping lowpass is a single pole. Use the offline per-band P6 diagnostics and real listening before calling it premium-quality. P7 deliberately changes old project wet tails; keep an exported P6 A/B available.

## Realtime and automated gates

- 11th CTest: `arverb_rt60_tail_tests`. Exercises 4 feedback algorithms × two sizes × two Decay values with actual 3-second impulse output, proving high-band 2.6–6.2kHz wet tail remains higher with 16.5k Damping than 1.3k Damping, finite/bounded at every sample, including worst 12s/Size2 setting. This is numeric **directionality**, not an EQ fidelity or listening certificate.
- Existing 10 CTests, real JUCE Windows host, core sanitizer and P6 audio signature still run.
- P7 Linux PR CI compares previous P6 `src/Reverb.cpp` to P7 with identical compiler flags and the same runner using all 54 CPU callback cases. This is informational, not a universal performance claim. It adds no CI job or matrix; results saved in `ArSonVerb-P7-P6-vs-P7-Linux-CPU`.
- The existing Windows JUCE CI still builds VST3+Standalone and generates the 8-WAV P6 audition pack, now with the modified P7 tail. No new test application or extra project asset.
- On failures or measured major realtime regressions, STOP instead of merging and inspect algorithm/stability hotpath.

## Windows physical listening — before promoting a release

Download latest successful Windows test artifact, insert on mono vocal and stereo music, and compare against **the P6 v0.5.0** output with *same* exact preset and loudness-matched mix return. Prioritize clear vocal consonants, acoustic guitar/piano transient preservation, long sustaining music, sibilants, snare and cymbal tails; at 12–16k High Cut compare short RT60 1.4s and long 5s, Damping 1.3k/6.3k/16.5k. Listen for unnatural prolonged hiss, metallic resonance and dark airless wet sound. Test stereo fold-down and mono-to-stereo FX return.

Record host, device, buffer (32/64/256), xruns, actual CPU p95/p99 where supported, samples rendered, and DAW state restoration. Device zero-latency is NOT implied. A subjective Valhalla/FabFilter comparison requires calibrated level matching and user recordings.

**Stop ship:** disproportionate or harsh HF; unexpected Mix dip; mono losses; plugin scan failure; callback deadline misses/xruns; regressions from existing host automation; missing licensing/public distribution review.

**Verified PR CI and merge:** [PR #27](https://github.com/masarray/arsonverb/pull/27) merged as `b1e7c6f`. Linux Release+ASan/UBSan [41212540154](https://github.com/masarray/arsonverb/actions/runs/41212540154) SUCCESS with 10/10 core CTests and the matched P6/P7 54-case CPU evidence artifact `13047500106`. Windows JUCE VST3+Standalone [41212540202](https://github.com/masarray/arsonverb/actions/runs/41212540202) SUCCESS **11/11 CTests**, 8 regenerated impulse WAVs and unsigned plugin + audition artifacts `13046572769` and `13046897126`. Artifacts expire in 7 days; P7 content is still named P6 by the shared offline workflow. **v0.6.0 is a development test build, NOT a licensed public release or physical/subjective acceptance**. User must validate actual DAW phase, matched-loudness vocal/music, device xruns/CPU and pluginval. Source, QA and handoff must never call measured objective directionality a product-quality certification.
