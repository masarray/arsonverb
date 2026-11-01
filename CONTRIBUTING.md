# Contributing to ArSonVerb

ArSonVerb is free/open source and welcomes reproducible bug reports, fixes, documentation and high-quality audio test cases. Small but complete, reviewable changes are preferred over speculative DSP rewrites.

## Before opening an issue or PR

1. Read the [README](README.md), [Quality Status](docs/QUALITY_STATUS.md), [Changelog](CHANGELOG.md) and search [existing issues](https://github.com/masarray/arsonverb/issues).
2. For a bug, share your OS, DAW/host, audio interface and driver, sample rate, buffer size, mono/stereo bus layout, plugin version, exact reproduction steps, expected/actual results and non-sensitive logs or recordings.
3. For audio-quality proposals, identify the measurable regression and provide repeatable audio or impulse fixtures, approximate reference settings, and a level-matched comparison protocol. Avoid unmeasured commercial parity claims.

## Developer setup

The JUCE-free DSP core compiles with C++17 and CMake 3.18+:

```sh
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release -DARVERB_BUILD_TESTS=ON
cmake --build build --config Release --parallel 2
ctest --test-dir build -C Release --output-on-failure
```

The plugin requires JUCE 9.0.4 (pinned via CMake), CMake 3.22+, OS development libraries and a supported VST3 host. Configure `-DARVERB_BUILD_PLUGIN=ON` to enable the VST3 and Standalone targets. Visual Studio 2022 x64 is used for Windows releases. CI source is under `.github/workflows/`.

## Pull request quality gate

- Explain the user-visible problem and the smallest complete change that solves it.
- Keep the **13 existing DSP automation IDs**, 32 preset defaults, plugin host identity, bypass/state schema and C ABI compatible unless an explicitly versioned migration is approved.
- The real-time audio callback must not allocate, lock, block, log, access disk/network or construct unbounded data. Do expensive processing during prepare/GUI, not in the callback.
- Run relevant core/host tests. Provide deterministic audio regression fixtures for sound-processing changes; distinguish automated assertions from physical listening.
- For visual changes, check desktop and mobile rendering and keyboard focus (site), or screenshots from the real JUCE renderer (plugin). No mockup claimed as product evidence.
- For release/package updates, retain AGPL source, Inter OFL, upstream notices and SHA256 provenance; never modify already-published assets.
- Keep CI focused: do not run heavy JUCE builds for static documentation changes. Update `CHANGELOG.md`, `docs/QUALITY_STATUS.md` and relevant docs when user-visible behavior or evidence changes.

See [PR template](.github/pull_request_template.md), [Security](SECURITY.md) and [Code of Conduct](CODE_OF_CONDUCT.md). **Submitting a contribution does not grant permission to relicense code;** only submit code you have rights to distribute under [AGPL-3.0-only](LICENSE). Do not submit proprietary presets/source copied from competing plug-ins.
