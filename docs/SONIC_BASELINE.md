# P5 Musical Clarity Recovery — verified CI, listening acceptance pending (2026-10-31)

Prior linear crossfade cut dry by −6 dB at 50% Mix even while the delayed/ducked wet was too weak to fill in. The verified P5 DSP computes exact sine/cosine gain targets only when Mix changes, then smooths dry and wet gains separately sample by sample. No per-sample LUT/trigonometry; steady 50% dry is −3.01dB and Mix 100% remains wet-only for sends. Two highpass stages on the wet low end remain, but high-cut lowpass now morphs smoothly from two stages at dark settings toward a single stage at bright settings, followed by a small wet-only 6.5kHz air shelf. HF feedback loss is limited with a bounded lowpass shelf blend; late/early return has modest fixed level normalization. No noise/shimmer/FFT/new delay lines/latency; dry at Mix 0% passes unchanged.

The original 8-line FDN, native 13 scalar parameters, four algorithm modes, 32 preset parameter arrays, gold JUCE GUI and C ABI remain. This changes sound across existing sessions, so recorded P4 mixes should be reviewed with level matching. New tests check dry onset and equal-power 50% law, 100% wet-only, non-silent stereo/mono wet Mix sweeps, air high-vs-low response, finiteness and previous C ABI/regression. This is not a guarantee against signal-specific phase cancellation in DAW sends.

See `docs/P5_MUSICAL_ENGINE_AUDIT.md` for research, measurements, limitations and the next direction.

---

# P3 verified — lighter early-to-late energy and modal bass decay

The final P3 design adds **no extra delay taps or buffer reads**. At Diffusion <=0.62, the original early crossfeed and early-send formula remain; at higher Diffusion, the six existing stereo taps smoothly increase their crossfeed from 0.10 to 0.18 and their share of the signal sent to the existing four-stage diffusers from 0.28 to 0.40. This is a modest original early-energy *mixing* innovation, **not a new Plate/Hall/Chamber mode** and not an increase in the number of reflection taps.

Inside the eight-line Householder feedback network, one existing projection is now processed by a **single shared first-order bass contour**. The low-frequency absorption coefficient is shaped by the existing Wet Low Cut (85 + 0.14×LowCut Hz, with approx 3–10% low-band modal absorption). This alters the *one reflection eigenmode* response without eight new per-line filters and without a full multiband Decay Rate EQ. Input dry is unmodified; existing output wet HP/LP and presence ducking are retained. Zero extra reported latency and the lock-free/audio-allocation-free processing contract remain.

Two more expensive variants were rejected from measured evidence rather than merged. Final 108-case same-runner quick benchmark shows median mean callback +5.59% normal / +5.98% high-Diffusion versus P2 with no offline deadline crossings. Physical DAW and level-matched A/B must decide whether the different early energy and low-mid reverb decay are musically preferable. Numerical tests are not listening tests.

Source behavior references: [ValhallaRoom early energy](https://valhalladsp.com/2011/05/04/valhallaroom-early-reflections-versus-early-energy/), [ValhallaRoom early send](https://valhalladsp.com/2011/05/18/valhallaroom-the-early-controls/), [FabFilter frequency-dependent decay](https://prod.fabfilter.com/help/pro-r/using/decayrateEQ). No proprietary algorithms were copied.

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
