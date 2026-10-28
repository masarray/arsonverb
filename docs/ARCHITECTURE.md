# AR Studio Reverb Core — architecture v0.1

```
                   dry L/R ------------------------> Dry/Wet mix ----> stereo out
                      |                                  ^
                      v                                  |
              Smoothable pre-delay                       |
                      |                                  |
               +------+-------+                          |
               |              |                          |
               v              v                          |
          early taps     allpass diffusion (4x stereo)   |
               |              |                          |
               |              v                          |
               |   8-channel orthogonal Householder FDN |
               |   + per-line RT60 gain + LP damping    |
               |   + low-rate fractional-delay motion  |
               |              |                          |
               +---- wet stereo readout -----------------+
                              |
                   2-pole HP + 2-pole LP
                              |
                    M/S wet-only stereo width
                              |
                    vocal-envelope ducking
                              |
                         Dry/Wet mix
```

## Why Householder FDN?

In an `N`-line feedback delay network, a dense orthogonal transform would require O(N²) multiply-adds. A Householder reflection based on vector `v` where `v[i]` is either +1 or -1:

```
H(x) = x - (2/N) * v * dot(v, x)
```

is energy preserving (before damping/gains) and costs O(N). For N=8, it avoids a dense matrix multiply. The line-delay gains target a 60-dB decay via:

```
feedbackGain[i] = exp(-ln(1000) * delaySeconds[i] / desiredRT60)
```

This maps broadband nominal RT60; the damped frequency response has a different effective T60. Rigorous multi-band RT60 is not implemented yet.

## Real-time constraints

- No allocation or locks in the audio callback; delay storage is sized in `prepare`.
- Non-power-of-two delays are embedded in power-of-two ring buffers for bitmask wrap.
- Block-rate parameter target capture + sample-rate one-pole smoothing.
- Feedback gain and oscillator step are recomputed only when respective parameters change.
- Oscillators use recursive sin/cos updates and rare renormalization; no per-sample sin().
- Feedback filter states are flushed to exact zero when sufficiently tiny.
- A dedicated C ABI exposes engine handles; VST3 and GUIs are **adapters**, not owners of DSP code.

## Separation for long-term maintainability

```
include/arverb/Reverb.hpp      Independent C++ DSP interface
include/arverb/arverb_c.h      Stable-ish C interface for external applications
src/Reverb.cpp                 Reverb engine and processing architecture
src/dsp/Primitives.hpp        Private delay, filter, allpass, oscillator primitives
src/Presets.cpp                User-facing preset parameter sets
src/arverb_c.cpp               Thin exception-safe C ABI facade
tests/                         DSP behavior and performance regression tests
examples/                      Offline IR renderer
```

For a production architecture: move `Delay`, `Allpass`, `Oscillator`, filters, envelope follower and each mode to individual headers/translation units; introduce a mode strategy, ABI versioning and preset serialization once the topologies become distinct. Deliberately avoid premature template metaprogramming or a generalized audio graph in the prototype.

## Known technical gaps

- Subjective audio quality has not been validated against Valhalla or real studio recordings.
- The preset named `VocalPlate` is **not** a physical/true plate simulation, just a tuned base FDN preset.
- Room size changes glide the delay read heads, and may create temporary pitch variation; production version needs dual-head/morph transitions.
- Crossover-dependent RT60 curves, stereo input preservation, true-stereo early spatialization, host latency reporting, plugin bypass and preset file format remain future work.
- No resampling, convolution, dynamic EQ, oversampling or lookahead is performed.
