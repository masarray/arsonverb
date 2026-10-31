# ArSonVerb — Musical Engine Audit & P5 Repair (2026-10-31)

**Merged source**: [PR #24](https://github.com/masarray/arsonverb/pull/24), `8da785b48a785fa17ad9f19583feea9e2799d52b`.

## 1. What was wrong — evidence not marketing

P4 was numerically stable and had real alternative FDN matrices, but a stable FDN is NOT a professionally tuned musical reverb. A user audition found severe Mix loudness loss and no airy high-frequency tail.

Actual P4 source: `out = (1-Mix)*dry + Mix*duckGain*wet`. At Mix 0.5, dry is at **0.5 linear (−6.02dB)**, irrespective of how delayed or attenuated the wet is. The effect includes pre-delay and several early reflections; at transient onset the missing dry cannot be filled by wet. This is a **proven gain-law dip** but not direct evidence of phase cancellation. At 100% Mix, dry intentionally becomes zero (send effect behavior).

The wet sound was also repeatedly darkened by HF feedback damping each tank recurrence and two cascaded output wet lowpasses even at a bright High Cut value. Midrange presence ducking compounds perceived tonal weakness, though it remains user-controllable.

## 2. How actual world-class reverbs differ

| Public reference | Critical principle | ArSonVerb remaining gap |
|---|---|---|
| [ValhallaRoom — high level Mix](https://valhalladsp.com/2011/05/02/valhallaroom-the-high-level-sliders/) | Sine/cosine equal-power Mix, 100% wet for sends | P5 implemented |
| [ValhallaRoom — late reverb](https://valhalladsp.com/2011/05/18/valhallaroom-the-late-controls/) | Independent high/low decay and crossovers, tuned/varied modulators | P5 adjusts HF feedback, but true multi-band RT60 still missing |
| [Valhalla — diffusion and metallic artifacts](https://valhalladsp.com/2011/01/21/reverbs-diffusion-allpass-delays-and-metallic-artifacts/) | Echo density must rise without metallic ringing; more diffusion is not necessarily better for vocals | P4 uses only fixed four allpass per side and one 8-line FDN |
| [FabFilter Pro-R 2](https://www.fabfilter.com/help/pro-r) | Separate Decay Rate EQ and Post EQ, distance, brightness, ducking, thickness, actual spatial design | No 6-band decay-rate contour or independent post-EQ |
| [Eventide Blackhole](https://www.eventideaudio.com/press-releases/blackhole-press-kit/) | Modulation integrated within reverb structure to smooth roughness | Only basic LFO delay modulation, no mode-specific density validation |
| [LiquidSonics Seventh Heaven](https://www.liquidsonics.com/software/seventh-heaven-professional/) | Early reflections, low-frequency and late-reverb layers treated individually | One shared wet early/late structure; no separate spectral tail analysis |

These are **public behavior sources**, not algorithms or proprietary DSP that we have copied.

## 3. P5 production corrections

1. Mix: compute `cos(Mix*pi/2)` and `sin(Mix*pi/2)` **only on target change**; smooth each gain at audio rate. 0% exact dry, 50% dry about −3 dB, 100% pure wet. No per-sample trig or table lookup.
2. High-frequency decay: apply feedback lowpass as a smoothed, convex high-frequency damping *shelf* rather than filtering 100% of each FDN recurrence. Maintains damping control and bounded energy.
3. Wet output: smoothly transition from the old double-stage high-cut LP at dark settings to a gentler one-stage LP at bright settings, followed by a bounded 6.5kHz wet-only air shelf.
4. Wet level: modestly increase late readout from 0.24 to 0.34 and early scaling from 0.38 to 0.42. No hard clipping and no gain changes to the input dry path.
5. All 13 host scalar IDs, 4 modes, 32 numeric factory program values, GUI, C ABI and 0 additional reported sample latency remain unchanged. **P5 deliberately changes sonic character across old sessions.**

## 4. Automated evidence (limitations stated)

| Gate | Verified result |
|---|---|
| [Linux Release + ASan/UBSan](https://github.com/masarray/arsonverb/actions/runs/41204928129) | Success |
| [Windows JUCE VST3 + Standalone](https://github.com/masarray/arsonverb/actions/runs/41204928207) | Success |
| Windows CTests | 10/10 passed |
| 50% Mix dry onset | Equal-power −3.01 dB, no old −6.02 dB drop |
| 100% Mix | Wet-only by design, no unexpected dry signal |
| Wet/mono sweep | Non-negligible output and finite behavior in 4 modes |
| Spectral air shelf | High band selectively lifted over low band |
| Engine algorithmic added latency | 0 samples reported |

**CPU rejection gate:** First LUT-per-sample P5 candidate had 12 theoretical offline deadline crossings on a 54-case Linux shared-runner short profile (192kHz/32/16 instances worst). We rejected that candidate. Revised gain-space smoother had **0** crossings and worst p99 budget **74.07%** (192kHz, 32 frames, 16 serial instances, steady); reference was **97.95%** on separate shared-runner experiment. These are *not same-machine speedups or actual DAW xrun counts*. The final CI was Windows 10/10 and Linux Release/sanitizers green.

## 5. Musical acceptance and next real engineering milestone

The subjective sound has not been listened to in this environment. Neither static tests nor sample fingerprint differences mean ArSonVerb already equals Valhalla, FabFilter, LiquidSonics or Eventide.

For real audition, render the same vocal/snare/piano/guitar with matched output level and identical settings in P4 and P5. Evaluate 0/25/50/75/100 Mix. Use 100% Mix only as an aux/send return while preserving the separate dry path. Start DUCK at 0% to isolate Mix, then compare HIGH CUT at 8/12/16k. Check stereo image and mono fold-down, sibilants and sibilance-tail preservation, transient clarity, and frequency-dependent RT60.

**Next high-value milestone** after acceptance: capture individual channel impulse responses, objective echo-density build-up, tonal decay RT60 per octave, mono cancellation/correlation, and dynamic spectral high-tail decay; profile mode-specific p99 with physical 32/64-sample ASIO before changing topology or adding controls. Prioritize controlled, dense early/late structures and zero-metallic ringing, not more factory presets or ungrounded global parity claims.
