# ArSonVerb → ArSonMix integration handoff

**Scope ownership:** ArSonVerb supplies an independently released, redistributable-under-AGPL **VST3 binary and C++ DSP core**. The [ArSonMix Factory FX epic #185](https://github.com/masarray/arsonmix/issues/185) owns **host packaging, actual plug-in loading, FX AUX routing, scene/Show state and real hardware acceptance**.

This boundary prevents misleading status claims. An independently downloadable ArSonVerb plug-in does **not** prove that ArSonMix successfully loads, routes, or bundles it.

## Verified upstream supply

- Licence: **GNU AGPL-3.0-only**, owner-approved 2026-11-01. No private/commercial exception, no assertion that a closed-source host may statically combine with the DSP without satisfying licensing.
- Latest release: [v1.0.1](https://github.com/masarray/arsonverb/releases/tag/v1.0.1), exact tagged source commit `4c65f369348790bb6da7465deaffc2b3921fdeee`.
- Windows VST3-only bundle: [ArSonVerb-Windows-x64-VST3-v1.0.1.zip](https://github.com/masarray/arsonverb/releases/download/v1.0.1/ArSonVerb-Windows-x64-VST3-v1.0.1.zip).
- SHA-256 manifest: [SHA256SUMS.txt](https://github.com/masarray/arsonverb/releases/download/v1.0.1/SHA256SUMS.txt).
- Validation: [Windows/Linux/macOS native build jobs, 12/12 CTests each](https://github.com/masarray/arsonverb/actions/runs/41247949265); [signed release archive promotion/hashes](https://github.com/masarray/arsonverb/actions/runs/41248747185). **These are native compilation/contract tests, not an ArSonMix host FAT report**.
- Interface: 13 stable DSP parameter IDs + appended algorithm selector and bypass, four algorithm matrices, 32 factory programs. Mono and stereo layouts are exercised in JUCE host contracts.

## Outstanding **ArSonMix-owned** acceptance

1. Verify **actual VST3 UID/class ID** and module binary identity in the ArSonMix host on Windows, using locked tagged source and SHA-256 manifest. Do not infer compatibility from filename or an old class-ID comment.
2. Host via VST3 module ABI; avoid statically linking this JUCE9 plug-in implementation into a JUCE8 host. Review AGPL/JUCE/third-party font licences and source access as part of any redistributed Factory FX bundle.
3. Build an audible shared stereo FX AUX return: two MIC sends (PRE/POST) into one stereo ArSonVerb instance, exactly one wet return, correct fader/meter/mute and no duplicate dry summing.
4. Validate scene/state recall, host bypass, plug-in latency handling, graph swaps and multi-instance stability with real audio hardware. Block shipping if real-device FAT fails.
5. Do not bundle binaries automatically until the ArSonMix epic proves all host-level and redistribution gates.

Historic ArSonVerb [issue #4](https://github.com/masarray/arsonverb/issues/4) tracks the upstream supply work; remaining dependent host integration is tracked centrally in [ArSonMix #185](https://github.com/masarray/arsonmix/issues/185) and [hardware #15](https://github.com/masarray/arsonmix/issues/15). The ArSonMix typed bus issue [#174](https://github.com/masarray/arsonmix/issues/174) is marked closed, but physical routing evidence is still a separate acceptance criterion.

This document is an engineering handoff, **not proof of completed Factory FX integration**.
