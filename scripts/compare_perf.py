#!/usr/bin/env python3
"""Compare ArSonVerb P1 callback percentiles from the SAME machine/build profile.

Standard library only; nonzero exit only for invalid or mismatched CSV inputs.
A larger p99_us is slower. This does not assess DAW xrun/dropout performance.
"""
import argparse
import csv
from pathlib import Path

KEY = ("sample_rate", "block_size", "instances", "mode")

def load(path: Path):
    with path.open(newline="", encoding="utf-8") as f:
        rows = [line for line in f if line.strip() and not line.lstrip().startswith("#")]
    parsed = list(csv.DictReader(rows))
    if not parsed:
        raise ValueError(f"{path}: no CSV benchmark cases")
    cases = {}
    for row in parsed:
        key = tuple(row.get(k, "") for k in KEY)
        if not all(key) or key in cases:
            raise ValueError(f"{path}: malformed or repeated case {key}")
        cases[key] = {
            "p99_us": float(row["p99_us"]),
            "mean_us": float(row["mean_us"]),
            "callbacks": int(row["callbacks"]),
        }
        if cases[key]["p99_us"] <= 0 or cases[key]["mean_us"] <= 0:
            raise ValueError(f"{path}: non-positive timing {key}")
    return cases

def main():
    cli = argparse.ArgumentParser(description=__doc__)
    cli.add_argument("baseline", type=Path)
    cli.add_argument("candidate", type=Path)
    args = cli.parse_args()
    old = load(args.baseline)
    new = load(args.candidate)
    if old.keys() != new.keys():
        raise SystemExit(f"Scenario mismatch: only baseline={old.keys() - new.keys()},"
                         f" only candidate={new.keys() - old.keys()}")
    delta = []
    for key in old:
        p99 = (new[key]["p99_us"] / old[key]["p99_us"] - 1.0) * 100.0
        mean = (new[key]["mean_us"] / old[key]["mean_us"] - 1.0) * 100.0
        delta.append((p99, mean, key))
    delta.sort(reverse=True)
    print("Positive change means SLOWER. Same CPU, load, compiler and run length required.")
    for label, subset in [("Worst p99 regressions", delta[:8]),
                          ("Largest p99 improvements", delta[-8:])]:
        print(f"\n{label}")
        for p99, mean, key in subset:
            print(f"  rate={key[0]} block={key[1]} instances={key[2]}"
                  f" mode={key[3]} | p99={p99:+.1f}% mean={mean:+.1f}%")
    print("\nCI runners are noisy; no automatic pass/fail threshold is inferred.")

if __name__ == "__main__":
    main()
