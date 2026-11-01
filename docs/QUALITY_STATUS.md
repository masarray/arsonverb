# Quality evidence, current limits and verification

**Current public binaries: [ArSonVerb v1.0.1](https://github.com/masarray/arsonverb/releases/tag/v1.0.1).** This page separates *demonstrated results* from *real-workstation acceptance still needed*. It is not a marketing claim of universal stability or a commercial-grade sonic equivalence certificate.

## Verified by automated engineering tests

| Requirement | Evidence | Current status |
|---|---|---|
| Native JUCE VST3 and Standalone build | [v1.0.1 three-platform build run](https://github.com/masarray/arsonverb/actions/runs/41247949265) | Windows, Linux, macOS universal native build jobs succeeded |
| DSP/host regression | Same run; each native job | 12/12 CTests per OS; includes actual JUCE AudioProcessor host test |
| AGPL source and asset provenance | [v1.0.1 publisher](https://github.com/masarray/arsonverb/actions/runs/41248747185) | Seven binary checksums and ZIP/tar validation passed; [installer ZIP](https://github.com/masarray/arsonverb/actions/runs/41250579656) appended separately with identical EXE SHA-256 |
| Four feedback modes | [P4 algorithms and tests](P4_ALGORITHMS.md) | Classic/Plate/Chamber/Hall use distinct orthogonal feedback matrices; not four independent plugin processors |
| 32 factory settings | [Factory presets](FACTORY_PRESETS.md) | 32 named presets and 13 DSP controls |
| Mono/stereo host layouts | [P6 routing and host regression](P6_PHYSICAL_AUDITION.md) | 1→1, 1→2, 2→2 layouts are tested in JUCE host code |
| Real-time DSP callback contract | [P8](P8_REALTIME_STARTUP_AIR.md), [P5](P5_MUSICAL_ENGINE_AUDIT.md), [architecture](ARCHITECTURE.md) | Allocation/lock avoidance, bounded state and modulation tested; cannot guarantee zero device xruns |

**The algorithm reports zero additional plugin latency to the DAW; it does not remove hardware/driver/DAW buffer latency.** An impulse test is not a listening test, and a passing JUCE contract test is not an independent pluginval result.

## Still requiring human/real-host validation

- **Musical listening:** blind, loudness-matched vocal/acoustic guitar/piano/full mix against one or more independent references. Confirm vocal consonants, tail density, unwanted ringing, high-frequency harshness and mix translation. Do not assert sonic parity without those recordings.
- **Real DAWs and devices:** test at least two hosts and both insert and 100%-wet send return (no duplicated dry path); include actual ASIO/WASAPI and, where available, macOS/Linux native backends. Record buffer size, sample rate, p95/p99 CPU if available, xruns/dropouts, session save/recall and GUI DPI.
- **Independent VST3 validation:** pluginval or equivalent report, plus plug-in state/multi-instance and automation stress at representative sample rates.
- **External ArSonMix integration:** the [ArSonMix Factory FX epic](https://github.com/masarray/arsonmix/issues/185) owns all host-level acceptance, AUX routing, pre/post send, FactoryFX supply-chain locking and end-to-end scene/state. ArSonVerb's independently downloadable VST3 is **not evidence** that ArSonMix bundles or hosts it correctly.

## Report a reproducible result

Use the [audio / DAW validation issue template](https://github.com/masarray/arsonverb/issues/new/choose). Record OS, DAW/version, audio interface/driver, sample rate, buffer frames, mono/stereo layout, algorithm/preset, Mix/Duck, reproduction steps, logs/recording and actual vs expected result. Avoid uploading private recordings without authorization.

For historical analysis and repeatable impulse/spectral diagnostics see [P5 measured sonic audit](P5_MUSICAL_ENGINE_AUDIT.md), [P6 audition instructions](P6_PHYSICAL_AUDITION.md), [P7 tail calibration](P7_RT60_AWARE_HF.md) and [P8 startup behavior](P8_REALTIME_STARTUP_AIR.md). These do not turn untested hardware configurations into guaranteed production outcomes.
