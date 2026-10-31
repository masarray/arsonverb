# P2 — Measured Audio + GUI Hot-Path Optimization

## Scope
Two specific inefficiencies found in P1 source: 5 unused scalar smoothing histories were integrated every audio sample; static gold logo and panel lettering were painted at each parent repaint despite having a faceplate cache.

## DSP: sound preservation
Eight smoothed values used directly by the audible algorithm: Mix, PreDelayMs, Size, Diffusion, ModDepthMs, Width, EarlyLevel, DuckAmount.
DecaySeconds drives FDN feedback gains; ModRateHz drives oscillator step coefficients; DampingHz/LowCutHz/HighCutHz drive independently smoothed filter coefficients. Their unused scalar smoothing histories may be skipped without affecting audio.
All 13 host IDs, 32 factory programs, block partition behavior, presence ducking, C ABI and zero *reported additional* samples remain unchanged. No allocations/locks/threads/timers in process().

## 27-case exact sample signature
tests/audio_signature.cpp computes deterministic FNV bit fingerprints of all stereo samples across 3 rates (48/96/192 kHz), 3 programs and 3 block sizes (1/32/257). Automation changes all five optimized-out controls as well as Mix and Duck. CI builds first-parent P1 Reverb.cpp with the same C++17 -O3 compiler and requires exact diff equality. This proves bit-identical output for those workloads and compiler, not subjective equivalence on all hosts.

## CPU benchmark
Within the same Linux Release runner, P1 reference and P2 candidate both run all 54 P1 benchmark cases. scripts/compare_perf.py produces scenario-keyed mean and p99 changes, without absolute threshold on a noisy CI machine. A reproducible physical-machine ASIO benchmark must precede broad speed claims.

## GUI
Static logo PNG, four group captions, subtitle and bronze chassis now share one opaque cached faceplate composited on resize. Parent paint retains dynamic Decay LCD. Knob/LED/arc remain live native JUCE children. Same image/typographic coordinates, no new caches. The JUCE host test still measures offscreen paint/resize. User Windows 100/125/150% screenshots still required.

## Merge acceptance
Linux exact 27-case audio equality; Linux Release and sanitizer; Windows core and JUCE VST3/Standalone with 7/7 CTests; verify benchmark and GUI CSV artifact, then merge. P2 optimization must not sacrifice musical sound or physical GUI luxury. Steinberg VST3 realtime guidance: https://steinbergmedia.github.io/vst3_dev_portal/pages/FAQ/Processing.html