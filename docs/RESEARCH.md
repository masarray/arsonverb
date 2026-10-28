# Public background — Valhalla-informed goals, original implementation

These references describe *public design ideas and listening targets* rather than giving access to Valhalla's proprietary implementation:

1. Sean Costello, **The Philosophy of ValhallaSupermassive** — delay/FDN topologies and the much harder challenge of suppressing stray resonances and producing tails that fit naturally in the mix:
   https://valhalladsp.com/2020/05/06/the-philosophy-of-valhallasupermassive/
2. **ValhallaRoom: The Reverb Modes** — varying echo-density, high-frequency absorption, pitch-stable stereo modulation:
   https://valhalladsp.com/2011/05/03/valhallaroom-the-reverb-modes/
3. **ValhallaRoom: Early Reflections versus Early Energy** — early diffusion can be more useful than a few hard reflections:
   https://valhalladsp.com/2011/05/04/valhallaroom-early-reflections-versus-early-energy/
4. **Reverbs: Diffusion, allpass delays, and metallic artifacts** — low/moderate diffusion can maintain vocal intelligibility:
   https://valhalladsp.com/2011/01/21/reverbs-diffusion-allpass-delays-and-metallic-artifacts/
5. **ValhallaFutureVerb: The Controls** — varied delay modulation; wet EQ management:
   https://valhalladsp.com/2025/10/03/valhallafutureverb-the-controls/
6. **ValhallaFutureVerb product description** — useful goal of clear, transparent reverb, plus the studio-typical high-/low-cut treatment for making wet effects sit in a mix:
   https://valhalladsp.com/shop/reverb/valhallafutureverb/
7. **ValhallaVintageVerb: The Modes** — treat reverb modes as distinct topologies, not just parameter presets:
   https://valhalladsp.com/2023/02/10/valhallavintageverb-the-modes/

Potential **permissively licensed** implementations for subsequent study and optional integration (none are incorporated in the current code):

- https://github.com/Signalsmith-Audio/basics (MIT)
- https://github.com/GhostNoteAudio/CloudSeedCore (MIT)
- https://github.com/el-visio/dattorro-verb (MIT)

Standards and classical DSP literature to study: Jon Dattorro's 'Effect Design Part 1' and the established feedback-delay-network literature from Jean-Marc Jot and Antoine Chaigne. If future code is borrowed from another project, copy its required license and attribution, record origin/commit, and track the use in THIRD_PARTY_NOTICES.

## A/B audition protocol

For each voice and instrument, render baseline and reference with the same wet loudness, similar pre-delay, and matched decay. Test at 44.1/48/96 kHz, 128/512 samples, dry mono and stereo, and check transitions while parameters change. Pay special attention to tails ringing at fixed pitches, sibilance masking, lead-vocal consonants, side-channel mono fold-down, and reverb overpowering the vocal.
