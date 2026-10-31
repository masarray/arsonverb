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

## Physical quality gate still OPEN
The user reports audible crackles on plug-in opening and preset switching in
the 2026-10-31 v1.0.0 candidate. P9 attempts to fix them via bounded callback
math, coherent preset transactions and smooth delay-tap switching. Automated
CI tests cannot certify ASIO underrun-free operation on the user's actual
audio interface/DAW. **Do not publish a stable release until the user has
physically verified the exact P9 build and accepted it.** The manual workflow
must enforce device QA confirmation for this reason.
