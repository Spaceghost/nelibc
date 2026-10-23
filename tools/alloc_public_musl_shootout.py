#!/usr/bin/env python3
"""Run paired static-musl host and Nelibc public allocator benchmarks."""

from __future__ import annotations

import argparse
import csv
import subprocess
from pathlib import Path


def main() -> int:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--host", required=True)
    parser.add_argument("--nelibc", required=True)
    parser.add_argument("--output", required=True)
    parser.add_argument("--pairs", type=int, default=24)
    args = parser.parse_args()

    rows = []
    for pair in range(1, args.pairs + 1):
        order = [("musl-host", args.host), ("nelibc-public", args.nelibc)]
        if pair % 2 == 0:
            order.reverse()
        for expected, executable in order:
            output = subprocess.check_output([executable], text=True)
            parsed = list(csv.DictReader(output.splitlines()))
            for row in parsed:
                if row["implementation"] != expected:
                    raise RuntimeError(
                        f"{executable} reported {row['implementation']!r}, "
                        f"expected {expected!r}"
                    )
                rows.append({"pair": pair, **row})

    output_path = Path(args.output)
    output_path.parent.mkdir(parents=True, exist_ok=True)
    with output_path.open("w", newline="") as stream:
        writer = csv.DictWriter(
            stream,
            fieldnames=[
                "pair", "implementation", "function", "size_bytes",
                "iterations", "median_ns_per_call",
            ],
        )
        writer.writeheader()
        writer.writerows(rows)
    print(f"{len(rows)} paired benchmark rows written to {output_path}")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
