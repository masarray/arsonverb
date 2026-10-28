#!/usr/bin/env python3
"""Offline ArSonVerb impulse-response measurements; NOT a subjective sound grade.

Dependencies: numpy and scipy. No runtime plugin/DSP dependency.
Example:
    cmake --build build --config Release --target arverb_demo
    build/arverb_demo /tmp/plate.wav plate
    python scripts/sonic_qa.py /tmp/plate.wav --output /tmp/report.json
"""
from __future__ import annotations

import argparse
import json
import math
import struct
from pathlib import Path

import numpy as np
from scipy.signal import butter, sosfilt


def float_wav(path: Path) -> tuple[int, np.ndarray]:
    raw = path.read_bytes()
    if len(raw) < 44 or raw[:4] != b"RIFF" or raw[8:12] != b"WAVE":
        raise ValueError(f"{path}: invalid RIFF/WAVE")
    position, fmt, data = 12, None, None
    while position + 8 <= len(raw):
        label, count = struct.unpack_from("<4sI", raw, position)
        position += 8
        if position + count > len(raw):
            raise ValueError(f"{path}: truncated chunk")
        if label == b"fmt ":
            fmt = raw[position:position + count]
        if label == b"data":
            data = raw[position:position + count]
        position += count + count % 2
    if fmt is None or data is None or len(fmt) < 16:
        raise ValueError(f"{path}: missing format or audio")
    kind, channels, rate, _, alignment, bits = struct.unpack_from("<HHIIHH", fmt)
    if (kind, channels, bits, alignment) != (3, 2, 32, 8) or len(data) % 8:
        raise ValueError(f"{path}: expected stereo float32 WAV")
    samples = np.frombuffer(data, dtype="<f4").reshape(-1, 2).astype(np.float64)
    if not np.isfinite(samples).all():
        raise ValueError(f"{path}: non-finite IR samples")
    return rate, samples


def apparent_t60(samples: np.ndarray, rate: int) -> float | None:
    """Schroeder energy-integral T20 x 3; not a guaranteed physical RT60."""
    energy = np.cumsum(np.square(samples)[::-1], dtype=np.float64)[::-1]
    if len(energy) == 0 or energy[0] <= 1e-20:
        return None
    db = 10.0 * np.log10(np.maximum(energy / energy[0], 1e-100))
    t5, t25 = np.flatnonzero(db <= -5.0), np.flatnonzero(db <= -25.0)
    if not len(t5) or not len(t25) or t25[0] <= t5[0]:
        return None
    return round(3.0 * (int(t25[0]) - int(t5[0])) / rate, 3)


def analyze(path: Path) -> dict:
    rate, audio = float_wav(path)
    if len(audio) < rate:
        raise ValueError(f"{path}: IR must contain at least one second")
    energy = np.sum(audio * audio, axis=1)
    total = float(energy.sum())
    if not math.isfinite(total) or total <= 1e-15:
        raise ValueError(f"{path}: silent or invalid IR")
    tail = audio[int(0.15 * rate):]
    left, right = tail.T
    lr_energy = float(np.dot(left, left) * np.dot(right, right))
    corr = float(np.dot(left, right) / math.sqrt(lr_energy)) if lr_energy > 1e-30 else None
    mid = (left + right) * 0.5
    mono_ratio = float(np.dot(mid, mid) / max(
        1e-30, 0.5 * (np.dot(left, left) + np.dot(right, right))))
    mono = audio.mean(axis=1)
    bands = {}
    for name, lo, hi in (("low", 90, 350), ("low_mid", 350, 1500),
                         ("presence", 1500, 4000), ("air", 4000, 12000)):
        hi = min(hi, rate * 0.45)
        if hi <= lo:
            bands[name] = None
            continue
        filtered = sosfilt(butter(3, [lo, hi], btype="bandpass",
                                 fs=rate, output="sos"), mono)
        band_energy = float(np.dot(filtered, filtered))
        bands[name] = {
            "rt20_x3_seconds": apparent_t60(filtered, rate),
            "relative_energy_db": round(10.0 * math.log10(max(1e-30, band_energy) / total), 2),
        }
    def interval(start: float, stop: float) -> float:
        return float(energy[int(start * rate):min(len(energy), int(stop * rate))].sum())
    onset = np.flatnonzero(energy > total * 1e-11)
    return {
        "file": str(path), "sample_rate": rate,
        "seconds": round(len(audio) / rate, 2),
        "peak_dbfs": round(20 * math.log10(max(1e-15, float(np.max(np.abs(audio))))), 2),
        "first_wet_ms": round(1000 * int(onset[0]) / rate, 2) if len(onset) else None,
        "stereo_correlation_after_150ms": round(corr, 4) if corr is not None else None,
        "mono_energy_retention": round(mono_ratio, 4),
        "early_to_late_db": round(10 * math.log10(
            max(1e-20, interval(0.05, 0.15)) / max(1e-20, interval(0.15, 1.0))), 2),
        "full_band_rt20_x3_seconds": apparent_t60(mono, rate),
        "bands": bands,
    }


def main() -> None:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("wav", nargs="+", type=Path)
    parser.add_argument("--output", type=Path)
    options = parser.parse_args()
    report = {"schema": 1, "method": "offline Schroeder T20*3; no subjective rating",
              "results": [analyze(path) for path in options.wav]}
    payload = json.dumps(report, indent=2, allow_nan=False) + "\n"
    if options.output:
        options.output.parent.mkdir(parents=True, exist_ok=True)
        options.output.write_text(payload, encoding="utf-8")
    print(payload, end="")


if __name__ == "__main__":
    main()
