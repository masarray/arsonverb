# P3 candidate — dense early energy + frequency-dependent bass recurrence

The formerly six isolated early taps now have a sparse+diffuse morph controlled by the existing Diffusion knob. When Diffusion <= 0.62, no extra reads or early-layer change occurs. Above threshold, six low-amplitude taps form additional stereo early energy, using existing sample-rate-allocated early delay buffers and smoothly varying a ~10% blend. This is intentionally original DSP, not reimplementation of ValhallaRoom.

The eight-line FDN now absorbs very low frequencies in its *shared Householder reflection eigenmode* as well as using independent wet HPF at output. Bass absorption is controlled by existing Wet Low Cut: cutoff 85+0.14×cutoff Hz, mild 3%-10% low-band reduction per recirculation. This differentiates low-band decay, while preserving upper midband decay and the dry path. Each filter state advances continuously for automation, with coefficient calculation only at control changes and sample-wise smoothing. Audio callback is still lock-free, allocation-free, O(8); there is no lookahead or added latency sample.

Industry behavioral references (not borrowed algorithms):
- https://valhalladsp.com/2011/05/04/valhallaroom-early-reflections-versus-early-energy/
- https://valhalladsp.com/2011/05/18/valhallaroom-the-early-controls/
- https://prod.fabfilter.com/help/pro-r/using/decayrateEQ

This is a *new sound* and must be tested with level-matched natural vocal, snare, piano and acoustic guitar. Numeric response checks do not validate musical quality. Large uncontrolled changes in tail brightness and bass should be rejected by listening. Need CPU p99 and real Windows DAW screenshots. Factory programs retain all 13 existing numeric values, and no old automation IDs are changed.

---

# 2026-10-31 — Musical clarity DSP milestone (PR #18 merged; CI verified, listening pending)

The verified earlier architecture is one original 8-line Householder FDN for all 32 factory programs. Prior to this milestone wet output had low/high cuts, FDN high damping, input predelay and broadband DUCK but no selective vocal/instrument presence protection. There is no claimed spectral AI classifier.

## Original real-time technique

- `src/dsp/Primitives.hpp` now has a small JUCE-independent `PresenceDucker`. Stereo-linked dry input filters estimate an approximate 1.25–4.2 kHz presence band.
- 10ms attack and 180ms release smooth a shared envelope, gating a **bounded subtraction** of this frequency band in the *wet only* output. This protects intelligibility while retaining more bass/air than a broadband cut. The existing Duck parameter controls the new shaping as well as previous broadband duck. At Duck=0 the new shaping is exactly output-transparent; the eight filters still advance internal state to avoid a sudden transient when Duck automation rises.
- All coefficients precomputed in `prepare()`; `process()` adds a constant number of one-pole multiplications and no allocations, locks, worker, FFT, lookahead or I/O. Dry remains untouched. RT60 FDN gain design, modulation, predelay, 13 parameter IDs, 32 factory programs, C ABI and reported zero additional samples of plugin latency remain unchanged.

## Evidence and limits

FabFilter Pro-R 2 independently exposes ducking, post EQ and frequency-dependent decay EQ; Valhalla DSP discusses echo density and early reflection control. These are product research references, **not** claims of reproduction, licensing or identical algorithms.
- https://prod.fabfilter.com/help/pro-r/using/maincontrols
- https://prod.fabfilter.com/help/pro-r/using/decayrateEQ
- https://valhalladsp.com/2010/06/07/diffusion-vocals-and-real-rooms/
- https://valhalladsp.com/2011/05/18/valhallaroom-the-early-controls/

A band from ~1.25 to 4.2 kHz is a sensible adjustable prototype focus, not a universal "sweet spot": sibilance, vocal formants and instrument fundamentals vary. We intentionally preserve the user's existing preset voicing and do not automatically add a new EQ UI or permanent carved notch.

## Validation

- Objective core tests: 2.5k wet signal must be ducked more than 230Hz, stereo symmetry, Duck=0 transparency, existing impulse/predelay/tail/NaN/realtime allocation tests.
- Exact GitHub Windows VST3/Standalone 6/6 CTest + Linux Release/sanitizer required before merge.
- Still necessary before calling sonic parity: matched-level dry vocal, piano, guitar and full-mix A/B, RT20-by-band and mono fold-down, measured CPU p99 on named hardware, plus separate algorithm development for Plate/Hall/Chamber identities.

---

# ArSonVerb sonic measurement baseline (2026-10-29)

This is an **offline diagnostic**, not proof that ArSonVerb matches or sounds like Valhalla. Do not ship raw reference audio or proprietary algorithms. The current four presets are all based on the **same 8-line FDN topology**.

## Reproduce

Core Release build, offline C++ IR exporter, then optional isolated Python analysis:

```bash
cmake -S . -B build-sonic -DCMAKE_BUILD_TYPE=Release -DARVERB_BUILD_TESTS=ON
cmake --build build-sonic --target arverb_demo
mkdir -p build-sonic/ir
for preset in plate chamber hall ballad; do
  ./build-sonic/arverb_demo "build-sonic/ir/${preset}.wav" "${preset}"
done
python -m pip install numpy scipy
python scripts/sonic_qa.py build-sonic/ir/{plate,chamber,hall,ballad}.wav --output build-sonic/sonic-baseline.json
```

This script accepts precisely stereo 32-bit IEEE float WAV output from `arverb_demo`. Use five seconds at 48 kHz for these initial comparison figures. Baselines were generated from the v0.4 core on Linux/GCC Release. Values are measurement observations, not targets or sonic ratings.

| Preset | Apparent RT20×3 (s) | Correlation after 150 ms | Mono energy retained | Early/late (dB) |
|---|---:|---:|---:|---:|
| Vocal Plate | 1.403 | -0.0311 | 0.4844 | -1.53 |
| Vocal Chamber | 1.020 | -0.0176 | 0.4912 | +2.17 |
| Music Hall | 2.040 | -0.1151 | 0.4425 | -3.31 |
| Airy Ballad | 2.914 | -0.2304 | 0.3848 | -11.39 |

**Measurement definitions:** RT20×3 is an apparent full-band T60 estimate derived by backward Schroeder energy integration from -5 dB to -25 dB, scaled by three. Correlation is calculated from left/right wet audio after 150 ms; mono energy retention compares `(L+R)/2` energy with average per-channel energy. Early/late compares 50–150 ms against 150–1000 ms. Band estimates use optional 3rd-order Butterworth offline analysis.

**Engineering implication:** The Airy Ballad stereo width can lose more energy when folded to mono; whether this is musical requires matched-level listening, not a unilateral parameter change. Frequency-selective decay appears shorter in the highs than lows, but no objective metric alone certifies good vocal blending.

## Acceptance gates for P2

- Level-matched dry sung voice, piano, guitar and full mix; compare timbre, density, consonant clarity, metallic ringing, mud buildup and perceived space with legal reference products.
- Preserve the exact DSP, sample rate, preset, input level, stereo routing and tool version in each report.
- Add distinct Plate/Chamber/Hall DSP topologies only behind regression tests and documented sonic listening decisions.
- Keep Python/SciPy completely isolated from C++ DSP, JUCE plugin, and normal CI. Run this offline tool manually when sound changes; no new automatic CI runner is required.

Tracked in [Issue #2](https://github.com/masarray/arsonverb/issues/2).
