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
