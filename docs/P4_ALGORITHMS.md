# P4 — Four distinct feedback families

P3 used one Householder FDN for all 32 factory presets. P4 makes Classic (legacy), Plate, Chamber and Hall independently selectable with distinct orthogonal feedback matrices.

| Mode | DSP feedback |
|---|---|
| Classic | The original signed Householder + P3 bass contour. Default for pre-P4 projects |
| Plate | 8-line normalized Hadamard butterfly, O(N log N), dense crossmixing |
| Chamber | Four interleaved Givens-pair rotations, intentionally sparser recurrence |
| Hall | Householder with a different feedback eigenvector |

No new delay buffers, plugin lookahead, FFT, or extra processing thread. During a switch, the same tank crossfades matrices for 100ms; dual evaluation is needed only in the transition.

## Compatibility

Original 13 host control IDs and their values, order, and numeric factory program arrays are unchanged. Bypass remains before the newly appended automatable algorithm choice. Old DAW XML chunks lacking the algorithm row explicitly restore Classic. 32 named factory presets now select a matching mode when chosen in JUCE, but the C ABI and Engine::loadPreset remain parameter-only and default Classic for old third-party hosts. Serialized state type remains unchanged.

## GUI

One 110-unit dark LCD mode selector fits between the active value and original full-width preset field in the premium gold editor. Choices CLASSIC, PLATE, CHAMBER, HALL use native host attachment. No new knob, no larger window, no extra GUI timer.

## Tests and limitations

New mode tests cover four distinct impulse fingerprints at 8k/48k/192k, rapid mode switching with continuous signal, finite stability, and backward session XML migration. Existing latency, C ABI, factory values, RT allocation, VST3/Standalone host and sanitizer tests remain. Require Linux Release/sanitizers, Windows 9/9 tests and mode-specific CPU + real ASIO tests before claiming professional performance or musical parity.

Public research: https://www.fabfilter.com/help/pro-r ; https://valhalladsp.com/2011/05/03/valhallaroom-the-reverb-modes/ ; https://prod.fabfilter.com/help/pro-r/using/decayrateEQ
