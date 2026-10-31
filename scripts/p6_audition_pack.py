#!/usr/bin/env python3
"""P6 OFFLINE ONLY: deterministic stereo IR set + inspectable sonic QA.
Standard library only. Does not run in an audio thread or ship with the VST3.
Usage: python scripts/p6_audition_pack.py --renderer PATH --output DIRECTORY
"""
import argparse
import csv
import hashlib
import json
import math
import os
from pathlib import Path
import struct
import subprocess

FS = 48000
MODES = ("classic", "plate", "chamber", "hall")
INPUTS = ("mono", "left")
BANDS = (250, 1000, 4000, 8000)
WINDOW = 960  # 20ms, used for repeatable *diagnostic* Schroeder T20.
HEADER = "<4sI4s4sIHHIIHH4sI"
FIELDNAMES = ["mode", "excitation", "peak_dbfs", "rms_dbfs",
              "lr_correlation_late", "mono_energy_retention_late",
              "rt60_250_hz_s", "rt60_1000_hz_s",
              "rt60_4000_hz_s", "rt60_8000_hz_s", "wav_sha256"]


class BandPass:
    """Stable biquad with fixed coefficients from RBJ Audio EQ Cookbook."""
    def __init__(self, center):
        omega = 2.0 * math.pi * center / FS
        sine = math.sin(omega)
        # Constant 1 octave bandwidth; no per-sample transcendentals.
        alpha = sine * math.sinh(math.log(2.0) / 2.0 *
                                       omega / sine)
        a0 = 1.0 + alpha
        self.b0 = alpha / a0
        self.b1 = 0.0
        self.b2 = -alpha / a0
        self.a1 = -2.0 * math.cos(omega) / a0
        self.a2 = (1.0 - alpha) / a0
        self.z1 = self.z2 = 0.0

    def process(self, sample):
        y = self.b0 * sample + self.z1
        self.z1 = self.b1 * sample - self.a1 * y + self.z2
        self.z2 = self.b2 * sample - self.a2 * y
        return y


def rt60_from_windows(energies):
    """Schroeder energy-decay estimate using T20 (-5..-25 dB) x3.
    Diagnostic only: 6s truncation, modulation and early reflections
    preclude identifying true perceptual RT60 or commercial quality.
    """
    remaining = 0.0
    edc = []
    for energy in reversed(energies):
        remaining += energy
        edc.append(remaining)
    edc.reverse()
    if not edc or edc[0] <= 1e-25:
        return None
    values = []
    for i, energy in enumerate(edc):
        if energy <= 0:
            continue
        db = 10.0 * math.log10(energy / edc[0])
        if -25.0 <= db <= -5.0:
            values.append((i * WINDOW / FS, db))
    if len(values) < 6:
        return None
    mean_x = sum(x for x, y in values) / len(values)
    mean_y = sum(y for x, y in values) / len(values)
    den = sum((x - mean_x)**2 for x, y in values)
    if den <= 1e-18:
        return None
    slope = sum((x-mean_x)*(y-mean_y) for x, y in values) / den
    return round(-60.0 / slope, 3) if slope < -0.001 else None


def analyze(path, mode, excitation):
    raw = path.read_bytes()
    if len(raw) < 44:
        raise ValueError(f"Truncated WAV: {path}")
    magic, riffsize, wave, fmt, fmtlen, form, channels, rate,         byte_rate, align, bits, data, datasize = struct.unpack_from(HEADER, raw)
    if (magic, wave, fmt, fmtlen, form, channels, rate, byte_rate,
            align, bits, data) != (b"RIFF", b"WAVE", b"fmt ", 16, 3,
                                   2, FS, FS * 8, 8, 32, b"data"):
        raise ValueError(f"Not 48k stereo float PCM impulse: {path}")
    if riffsize != len(raw) - 8 or datasize != len(raw) - 44 or datasize % 8:
        raise ValueError(f"Incorrect WAV RIFF/data lengths: {path}")
    energies = {band: [] for band in BANDS}
    filters = {band: BandPass(band) for band in BANDS}
    window_energy = {band: 0.0 for band in BANDS}
    sum_l2 = sum_r2 = sum_lr = 0.0
    total = 0.0
    max_amp = 0.0
    count = 0
    late = 0
    for i, (l, r) in enumerate(struct.iter_unpack("<ff", raw[44:])):
        if not math.isfinite(l) or not math.isfinite(r):
            raise ValueError(f"NaN/Inf in {path} at sample {i}")
        count += 1
        max_amp = max(max_amp, abs(l), abs(r))
        total += l*l + r*r
        if i >= FS // 5:  # 200ms after impulse; exclude early section.
            sum_l2 += l*l
            sum_r2 += r*r
            sum_lr += l*r
            late += 1
        mid = 0.5*(l+r)
        for band in BANDS:
            y = filters[band].process(mid)
            window_energy[band] += y*y
        if (i+1) % WINDOW == 0:
            for band in BANDS:
                energies[band].append(window_energy[band])
                window_energy[band] = 0.0
    if count != 6*FS or max_amp < 1e-7 or total < 1e-12 or late < FS:
        raise ValueError(f"Missing or near-silent IR: {path}")
    mono_energy = (sum_l2+sum_r2+2*sum_lr)/4
    stereo_average = (sum_l2+sum_r2)/2
    correlation = sum_lr / math.sqrt(sum_l2*sum_r2) if sum_l2*sum_r2 > 1e-24 else None
    result = {
        "mode": mode,
        "excitation": excitation,
        "peak_dbfs": round(20*math.log10(max_amp), 3),
        "rms_dbfs": round(20*math.log10(math.sqrt(total/(2*count))), 3),
        "lr_correlation_late": round(correlation, 5) if correlation is not None else None,
        "mono_energy_retention_late": round(mono_energy/stereo_average, 5)
            if stereo_average > 1e-24 else None,
        "wav_sha256": hashlib.sha256(raw).hexdigest()
    }
    for band in BANDS:
        result[f"rt60_{band}_hz_s"] = rt60_from_windows(energies[band])
    return result


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--renderer", required=True, type=Path)
    parser.add_argument("--output", required=True, type=Path)
    args = parser.parse_args()
    if not args.renderer.is_file():
        parser.error(f"renderer not found: {args.renderer}")
    args.output.mkdir(parents=True, exist_ok=True)
    rows = []
    for mode in MODES:
        for excitation in INPUTS:
            wav = args.output / f"ArSonVerb_P6_{mode}_{excitation}_48k.wav"
            # Same preset and 0% Duck, 100% wet isolates algorithm topology.
            subprocess.run([str(args.renderer.resolve()), str(wav.resolve()),
                            "clean", mode, excitation], check=True)
            row = analyze(wav, mode, excitation)
            rows.append(row)
            print(f"Verified {wav.name}: peak={row['peak_dbfs']} dBFS, "
                  f"mono={row['mono_energy_retention_late']}", flush=True)

    csv_path = args.output / "P6_sonic_diagnostics.csv"
    with csv_path.open("w", newline="", encoding="utf-8") as report:
        writer = csv.DictWriter(report, FIELDNAMES)
        writer.writeheader()
        writer.writerows(rows)
    manifest = {
        "purpose": "P6 repeatable offline QA / actual audition, not plugin certification",
        "engine_git_sha": os.environ.get("GITHUB_SHA", "local-build"),
        "samplerate": FS,
        "bits": 32,
        "format": "IEEE float stereo WAV",
        "duration_seconds": 6,
        "preset": "Vocal Plate Clean (same numeric values for all modes)",
        "mix": "100% wet",
        "duck": "0%",
        "levels": "Raw engine outputs, NOT normalized or loudness matched",
        "modes": list(MODES),
        "excitations": list(INPUTS),
        "files": rows,
    }
    (args.output / "P6_provenance.json").write_text(
        json.dumps(manifest, indent=2, allow_nan=False) + "\n",
        encoding="utf-8")
    (args.output / "READ_ME_FIRST.txt").write_text(
        """ArSonVerb P6 - AUDITION / PHYSICAL TEST (48kHz, stereo float WAV)
========================================================================
Each mode is the SAME Vocal Plate Clean settings. 100% wet, Duck=0%;
two IRs per mode: dual-mono impulse and LEFT-only impulse.

Files are UNNORMALIZED: compare only after matching perceived loudness.
For reverb tail quality, open in Audacity/Reaper; listen to ringing, tail
smoothness, high-frequency air and left/right/mono differences.
Impulse WAV is a snapshot; time-varying modulation is NOT reproduced by
static convolution of this file, so verify the REAL VST3/Standalone with
recorded dry vocals, acoustic guitar, piano and a stereo music mix.

Real Windows host test:
1. Load VST3 onto MONO vocal insert (mono->mono), then stereo insert.
2. Start MIX 15-25%, DUCK 0; sweep MIX 0/25/50/75/100.
3. For AUX/send, preserve dry path in host and set MIX 100% on FX return.
4. Test all Classic/Plate/Chamber/Hall, HIGH CUT 8/12/16k.
5. Sum stereo outputs to mono; detect unexpected dips, ringing and clipping.
6. Test 48/96k, buffer 32/64/256; record CPU p99 + real xrun counts.
7. Save/reload DAW state; bypass/re-enable while reverb tail is active.

P6_sonic_diagnostics.csv: relative late L/R correlation and mono energy
retention, approximate octave-band Schroeder T20 x3. NONE of these metrics
alone proves lack of phase cancellation on real audio or Valhalla parity.
CSV and SHA256 hashes give reproducibility, not commercial-grade approval.
Outputs are UNSIGNED experimental artifacts; do not redistribute without
outbound license approval. See repo docs/P6_PHYSICAL_AUDITION.md.
""", encoding="utf-8")
    print(f"PASS: {len(rows)} measured IR files; {csv_path}", flush=True)


if __name__ == "__main__":
    main()
