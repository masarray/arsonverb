# ArSonVerb copyright and distribution

ArSonVerb — Copyright (c) 2026 Mas Ari / SonKuPik.

**Chosen by the project owner on 2026-11-01: free/open-source AGPL-3.0-only.**
See the complete GNU Affero General Public License Version 3 in [LICENSE](LICENSE).
SPDX identifier: `AGPL-3.0-only`. Source is offered under this licence;
there is no proprietary relicensing or closed-source exception in this repository.

## Third-party rights and corresponding source
- **JUCE 9.0.4** (downloaded at build time, not copied into this repo):
  used under its AGPLv3 option. Preserve JUCE ownership and comply with the
  framework's AGPLv3 terms; see https://github.com/juce-framework/JUCE/blob/9.0.4/LICENSE.md
  and https://www.gnu.org/licenses/agpl-3.0.en.html.
- **Inter Variable font:** embedded in the GUI under the SIL Open Font
  License 1.1; original copyright, terms and redistribution notice preserved
  at [assets/fonts/OFL.txt](assets/fonts/OFL.txt).
- **Logo / gold knob:** project GUI artwork, credited to ArSonVerb. Confirm
  ownership/redistribution rights to any externally contributed derivatives
  before publishing official installer binaries.
- **VST3 SDK:** downstream redistribution must retain and comply with any
  applicable SDK notices. This repository builds using JUCE, not a vendored
  proprietary Valhalla implementation.

The tagged public GitHub source archive must remain available with the exact
released binary and build instructions. Do not publish AGPL binaries lacking
the corresponding source/required upstream notices.

## Current public release: v1.0.1

[Official v1.0.1 release](https://github.com/masarray/arsonverb/releases/tag/v1.0.1) publishes **Windows x64, Linux x86_64 and macOS universal** VST3/Standalone native binaries alongside source and SHA-256 manifests. [Native compile/host test jobs](https://github.com/masarray/arsonverb/actions/runs/41247949265) succeeded (12/12 each OS); [artifact publisher](https://github.com/masarray/arsonverb/actions/runs/41248747185) verified all original package hashes. The supplementary [Windows Installer ZIP](https://github.com/masarray/arsonverb/releases/download/v1.0.1/ArSonVerb-Windows-x64-Installer-v1.0.1.zip) contains the original validated Inno Setup EXE unchanged, with its own checksum.

The open-source AGPL-3.0-only licence permits redistribution **subject to its obligations**, including corresponding source and copyright notices. Reusing a VST3 binary from ArSonMix requires an explicit host-level licence and integration review; a working downstream Factory FX integration is **not asserted**. See [ArSonMix integration handoff](docs/ARSONMIX_INTEGRATION.md).

## P9 physical acceptance and public publication
On 2026-11-01, the owner accepted the P9 build following its startup and
preset-switch remediation and explicitly approved **public stable v1.0.0**.
Windows VST3/Standalone JUCE 12/12 CTests and Linux 11/11 plus ASan/UBSan
passed. The one-shot protected release branch ensures binary and tagged
source come from current `main`, with a full Windows rebuild/test before
publishing. This acceptance does not certify every ASIO/audio-driver or host
combination, nor claim a universal zero-xrun guarantee.
