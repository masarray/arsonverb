# ArSonVerb v1.0.0 — First Stable Windows Release

Created by Mas Ari — SonKuPik © 2026
https://www.youtube.com/@sonkupik

## Included
- Windows x64 VST3 plug-in and Windows standalone application (unsigned ZIP).
- Four distinct algorithmic feedback modes: Classic, Plate, Chamber, Hall.
- 32 factory presets with mono-in/mono-out, mono-in/stereo-out and stereo-in/stereo-out layouts.
- Musical high-frequency decay shaping, controlled wet top air, equal-power Mix, presence-aware wet ducking.
- Real-time audio protections: all heavy initial controls prepared before callback, restored DAW state synchronized before prepare, bounded expensive control-coefficient recalculation.
- Small, native clickable creator credit at the bottom of the existing premium gold GUI. No GUI size or DSP latency change.

## Installation (Windows x64)
- Close your DAW before replacing a previous test build.
- Extract the ZIP, and copy the complete `ArSonVerb.vst3` bundle to `C:\Program Files\Common Files\VST3\`. Rescan plug-ins in your DAW.
- The standalone `ArSonVerb.exe` may be run directly. Select the desired audio device/driver in the standalone application's audio settings.
- Existing DAW automation identifiers and factory settings remain compatible with the P8 v0.7.0 source baseline.

## P9 startup and preset-switch stability
- User reported crackles on opening the GUI/plugin and sometimes on factory-preset changes after P8.
- P9 adds prewarm before the first ASIO callback, bounded polynomial hot-path control math (no libm spikes), a coherent 14-parameter preset transaction, redundant host-notification suppression and two-fixed-tap Pre-delay/Size crossfades.
- The P9 fixes have been accepted by the project owner for public stable release on 2026-11-01. This does not claim zero underruns on every driver, interface, or host.
- Automated tests cover coefficient accuracy, no first-frame tap jump, and rapid preset switching across the four feedback modes.

## Engineering acceptance
- Verified P9 [Linux Release and ASan/UBSan](https://github.com/masarray/arsonverb/actions/runs/41225267456) with 11/11 core tests; [Windows VST3/Standalone](https://github.com/masarray/arsonverb/actions/runs/41225265424) with 12/12 tests including real JUCE host. The public release workflow reruns the full Windows tests on the exact published commit.
- No promise of zero round-trip latency: VST3 reports no plugin-added latency, but audio drivers and hardware impose real buffering. Driver underrun validation depends on the user's physical setup.
- The binary is unsigned; Windows/DAW security prompts may occur. No installer or code-signing certificate is included.
- Linux and macOS binaries are not included in this first Windows release.
- JUCE 9 third-party framework is copyright its respective owners. Source and/or third-party licence compliance must be confirmed under the selected distribution terms before running the public-release workflow.

## Licensing and provenance
ArSonVerb is free/open-source under the project owner's chosen **AGPL-3.0-only**, including the bundled DSP and JUCE 9 frontend under JUCE's AGPLv3 option. The full source licence is in [LICENSE](../LICENSE), and the embedded Inter variable font retains its separate [SIL OFL 1.1](../assets/fonts/OFL.txt). Public releases include corresponding tagged source from GitHub, source build instructions and third-party notices. The owner approved free AGPLv3 source distribution and public stable publication on 2026-11-01, following P9 listening acceptance. All binaries are accompanied by the corresponding tagged GitHub source archive; no third-party commercial sonic equivalence or universal driver performance guarantee is implied.

[Source](https://github.com/masarray/arsonverb) · [SonKuPik YouTube](https://www.youtube.com/@sonkupik)
