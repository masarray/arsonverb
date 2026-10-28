# Plugin host validation — acceptance record template

**This is a checklist, not a record of successful DAW testing.**

## Step A: automated Windows CI (required)
- [ ] Source is in dedicated GitHub repository, commit SHA documented.
- [ ] `.github/workflows/core-ci.yml` passes Windows, Linux and macOS core builds.
- [ ] `.github/workflows/plugin-ci.yml` builds JUCE 9.0.4 VST3 + Standalone under MSVC.
- [ ] `arverb_plugin_host_tests` passes using actual JUCE `ReverbProcessor`.
- [ ] Artifact includes `AR Studio Reverb.vst3` and Standalone app. Link to workflow run and artifact.

## Step B: DAW smoke test (required before public beta)
- [ ] Test DAW/version/OS and CPU documented.
- [ ] Plugin scans and opens without warning.
- [ ] Stereo in/out, 44.1/48/96 kHz work; 1/17/256/1024 sample blocks where host supports it.
- [ ] Insert 20% wet vs aux-send 100% wet produce sensible output.
- [ ] Continuous Mix, Pre-delay and Decay automation: no clicks or stuck values.
- [ ] Native plugin bypass and host bypass preserve input and do not produce pops.
- [ ] Save project, close DAW, reopen: all 13 parameters and bypass restored.
- [ ] Switch four presets; editor resize; reopen plugin GUI.
- [ ] Create/remove 20 instances without crash; test unload/rescan.
- [ ] Pass independent pluginval and/or Steinberg validator with attached report.

## Step C: audio QA (needed before claiming premium sound)
- [ ] Level-matched blind A/B on dry vocal, vocal with instruments, acoustic, and dense pop mix.
- [ ] Evaluate sibilance, tail grain, metallic coloration, low-mid buildup and stereo compatibility in mono.
- [ ] Measure CPU peak and stress under automation; compare 48/96 kHz.
- [ ] Evaluate distinct reverb topologies only once actually implemented (currently four presets use one FDN).

**Record:** commit SHA, CI run, tests/DAW logs, audio examples, open bugs, and acceptance decision in `docs/STATUS.md`.
