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

## P9 crackle-fix gate (DO NOT MARK STABLE WITHOUT DEVICE ACCEPTANCE)
- User reported crackles on opening the GUI/plugin and sometimes on factory-preset changes after P8.
- P9 adds prewarm before the first ASIO callback, bounded polynomial hot-path control math (no libm spikes), a coherent 14-parameter preset transaction, redundant host-notification suppression and two-fixed-tap Pre-delay/Size crossfades.
- Static tests validate coeff accuracy, no first-frame delay jump and rapid four-mode preset switching; real-device ASIO xrun counters and listening acceptance are still required.
- Install/testing should use the **latest P9 Windows CI artifact** for actual physical evaluation. Do not substitute older P8 v1.0.0 builds.

## Engineering acceptance
- Verified P8 Linux Release + sanitizers; Windows JUCE VST3/Standalone and actual host test suite 11/11. The first public stable build workflow MUST rerun full JUCE Windows CTests at the exact tagged SHA before publishing.
- No promise of zero round-trip latency: VST3 reports no plugin-added latency, but audio drivers and hardware impose real buffering. Driver underrun validation depends on the user's physical setup.
- The binary is unsigned; Windows/DAW security prompts may occur. No installer or code-signing certificate is included.
- Linux and macOS binaries are not included in this first Windows release.
- JUCE 9 third-party framework is copyright its respective owners. Source and/or third-party licence compliance must be confirmed under the selected distribution terms before running the public-release workflow.

## Licensing and provenance
ArSonVerb is free/open-source under the project owner's chosen **AGPL-3.0-only**, including the bundled DSP and JUCE 9 frontend under JUCE's AGPLv3 option. The full source licence is in [LICENSE](../LICENSE), and the embedded Inter variable font retains its separate [SIL OFL 1.1](../assets/fonts/OFL.txt). Public releases include corresponding tagged source from GitHub, source build instructions and third-party notices. The manual workflow still requires project/artwork rights and **explicit successful physical DAW/ASIO QA**, not merely green CI.

[Source](https://github.com/masarray/arsonverb) · [SonKuPik YouTube](https://www.youtube.com/@sonkupik)
