# Changelog

Notable user-visible changes are listed here. Historical engineering experiment notes remain in [docs/STATUS.md](docs/STATUS.md). Released binary/tag contents are immutable.

## [Unreleased] — website and community maintenance

- Professional public README and GitHub community contribution/security/support documentation.
- Landing page with real JUCE screenshot, Windows/Linux/macOS download links, and accessible stationary tactile buttons.
- Current product QA status split into automated evidence and physical/subjective limits.
- No changes to v1.0.1 released binaries, DSP, factory presets or parameter IDs.

## [1.0.1] — 2026-11-01

- Public native builds for **Windows x64**, **Linux x86_64**, and **macOS universal (Intel and Apple Silicon)**.
- Separate VST3-only and Standalone archives on each platform; Windows has optional selective Inno Setup installer and an installer-in-standard-ZIP alternative.
- Multi-OS native JUCE builds and 12/12 CTests per system, artifact/archive verification, SHA-256 manifests and exact tagged source provenance.
- DSP, preset bank and 13 existing host parameter IDs unchanged from v1.0.0.
- Published source is AGPL-3.0-only; no paid subscription/activation.

[Release notes](docs/RELEASE_V1.0.1.md) · [Download v1.0.1](https://github.com/masarray/arsonverb/releases/tag/v1.0.1)

## [1.0.0] — 2026-11-01

- First public stable Windows x64 VST3 + Standalone distribution, following owner-accepted P9 startup/preset switching improvements.
- Four algorithms (Classic, Plate, Chamber, Hall), 32 factory programs, mono/stereo host layout support, compact satin bronze/gold editor and 13 stable DSP knobs.
- Low-overhead real-time preparation, preset transition smoothing, wet-tail tonal controls and host-state compatibility improvements.
- JUCE host + core regression validation; physical driver and host behavior depends on the user setup.

[Release notes](docs/RELEASE_V1.0.0.md) · [Download v1.0.0](https://github.com/masarray/arsonverb/releases/tag/v1.0.0)

[1.0.1]: https://github.com/masarray/arsonverb/releases/tag/v1.0.1
[1.0.0]: https://github.com/masarray/arsonverb/releases/tag/v1.0.0
