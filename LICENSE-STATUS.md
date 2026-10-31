# Distribution license status

This repository contains original experimental code from the AR Studio Reverb project. A project-level outbound license has **not yet been chosen**; do not assume this source package is MIT-licensed or approved for redistribution.

JUCE is a separate third-party framework fetched at build time; **JUCE source is not included** in this archive. Comply with the current JUCE license and any applicable commercial terms before distributing a proprietary binary. VST3 SDK licensing does not replace JUCE licensing. The DSP implementation here must not contain Valhalla proprietary source code.

Before a public GitHub repository/release: select the intended project source license, review provenance, set long-term plugin identifiers, and review third-party licenses.

## Public stable release gate (2026-10-31)
The owner requested a public stable release following the v0.7.0 P8 milestone and adding a SonKuPik YouTube footer. **No public release must be labeled compliant until the copyright owner selects and documents an outbound source license or a proprietary-distribution policy, and establishes an appropriate JUCE 9 licensing basis (AGPLv3 with corresponding source obligations OR the applicable commercial JUCE licence).** Do not unilaterally choose AGPLv3: that is a rights grant by the copyright holder. The repository being public does not itself grant an outbound open-source licence.

The CI's unsigned VST3 + standalone artifacts are engineering outputs, not a signed installer, pluginval certificate, driver xrun certification or a compliant distribution by themselves. Public stable must also confirm physical host startup/ASIO validation, embedded font/PNG redistribution, and third-party notices. GitHub-release creation can only proceed after these explicit gates.
