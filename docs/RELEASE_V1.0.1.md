# ArSonVerb v1.0.1 — Cross-platform, no-budget distribution

**Free/open source (GNU AGPL-3.0-only).** Copyright (c) 2026 Mas Ari - SonKuPik.
This release adds installer and distribution formats; the DSP, preset IDs and
premium GUI remain unchanged from the stable P9 v1.0.0 milestone.

## Pick just what you need

| System | Package | Content |
|---|---|---|
| Windows x64 | `ArSonVerb-Windows-x64-VST3-v1.0.1.zip` | VST3 only, manual copy |
| Windows x64 | `ArSonVerb-Windows-x64-Setup-v1.0.1.exe` | Inno Setup, optional VST3 + standalone, uninstall |
| Windows x64 | `ArSonVerb-Windows-x64-Standalone-v1.0.1.zip` | Portable standalone |
| Linux x86_64 | `ArSonVerb-Linux-x86_64-VST3-v1.0.1.tar.gz` | VST3 only |
| Linux x86_64 | `ArSonVerb-Linux-x86_64-Standalone-v1.0.1.tar.gz` | Standalone (requires system desktop/audio libraries) |
| macOS universal | `ArSonVerb-macOS-universal-VST3-v1.0.1.zip` | VST3-only, Apple Silicon + Intel |
| macOS universal | `ArSonVerb-macOS-universal-Standalone-v1.0.1.zip` | Portable .app, Apple Silicon + Intel |

## Installation

**Windows VST3 ZIP:** unzip and copy the `ArSonVerb.vst3` folder to
`%LOCALAPPDATA%\\Programs\\Common\\VST3\\` (current user), or
`C:\\Program Files\\Common Files\\VST3\\` (all users, administrator).
Close the DAW before replacing an existing VST3 plugin, then rescan plugins.

**Windows smart installer:** run the Inno Setup EXE, select VST3 and/or Standalone;
the default per-user installation does not require administrator elevation.
It installs the user VST3 into the standard location, provides an optional
Start Menu shortcut and uninstaller, and retains licence files.

**Linux VST3:** extract the .tar.gz and copy `ArSonVerb.vst3` into
`~/.vst3/`. The standalone archive contains the executable `ArSonVerb`;
it requires libraries matching your Linux distribution and desktop/audio
stack. No DEB, RPM, AppImage or universal system-library compatibility
claims are made.

**macOS VST3:** unzip and put `ArSonVerb.vst3` in
`~/Library/Audio/Plug-Ins/VST3/`. Standalone: move `ArSonVerb.app`
to `~/Applications/`. The universal build contains both x86_64 and
arm64 slices. **Not Developer ID signed or Apple notarized.** macOS
Gatekeeper may demand explicit approval after first launch. Users who
trust the official source and checksum may use System Settings >
Privacy & Security > Open Anyway for that individual item if offered.
Do not disable system security globally.

## Security / zero-cost distribution limits

Repacking EXE as ZIP or tar.gz does **not** remove SmartScreen, antivirus,
Smart App Control or macOS Gatekeeper checks. Self-signing is not a
trusted substitute for a recognized certificate; Inno Setup is not an
exemption from SmartScreen. Keep direct, canonical GitHub Release URLs;
check the `SHA256SUMS.txt` file next to all binary assets and tagged
source. No fake signatures, no mark-of-the-web stripping, no Defender
exclusions, no global Gatekeeper disable scripts, no AV evasion.

If Microsoft Defender identifies a **false positive**, report it to
Microsoft's official Security Intelligence false-positive submission
portal with a reproducible package and hash; do not alter binaries
simply to avoid scanners. Security policy-enforced blocks cannot be
guaranteed to be bypassable. The product is distributed as
**publisher-unsigned** and does not claim universal xrun-free hardware
behavior.

## Verification

- Source build: pinned JUCE 9.0.4 plus original AGPL-3.0-only core.
- Each platform package includes `LICENSE`, `INTER-OFL.txt`,
  `RELEASE_NOTES.md` and `INSTALL.txt`.
- The Windows installer includes the same license and native binaries.
- Before publication: native JUCE VST3 + Standalone builds, CTests and
  archived artifact contents checked by GitHub Actions on each OS.
- Release job requires exact commit equality with `main` and validates
  all expected packages and SHA-256 hashes before publishing once.
- v1.0.0 is immutable; new artifacts belong to **v1.0.1** only.

More: [ArSonVerb source](https://github.com/masarray/arsonverb) ·
[SonKuPik YouTube](https://www.youtube.com/@sonkupik) ·
[Steinberg VST3 paths](https://steinbergmedia.github.io/vst3_dev_portal/pages/Technical%2BDocumentation/Locations%2BFormat/Plugin%2BLocations.html)
