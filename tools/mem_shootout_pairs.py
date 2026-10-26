#!/usr/bin/env python3
"""Run paired libc/Nelibc memory benchmarks in alternating process order."""

import argparse
import csv
import io
import os
import subprocess
import sys


def read_rows(executable):
    result = subprocess.run(
        [executable], check=True, capture_output=True, text=True
    )
    reader = csv.DictReader(io.StringIO(result.stdout))
    if not reader.fieldnames:
        raise RuntimeError(f"{executable} produced no CSV header")
    return reader.fieldnames, list(reader)


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--pairs", type=int, default=7)
    parser.add_argument("--cpu", type=int, help="pin benchmark children to this CPU")
    parser.add_argument("--output", required=True)
    parser.add_argument(
        "--profile", action="append", nargs=3,
        metavar=("NAME", "HOST_LIBC_BINARY", "NELIBC_BINARY"), required=True,
    )
    args = parser.parse_args()
    if args.pairs < 1:
        parser.error("--pairs must be positive")
    if args.cpu is not None:
        if not hasattr(os, "sched_setaffinity"):
            parser.error("CPU affinity is unavailable on this platform")
        os.sched_setaffinity(0, {args.cpu})

    output_rows = []
    base_fields = None
    for profile, host_binary, nelibc_binary in args.profile:
        for pair in range(1, args.pairs + 1):
            order = (
                [("host", host_binary), ("nelibc", nelibc_binary)]
                if pair % 2
                else [("nelibc", nelibc_binary), ("host", host_binary)]
            )
            order_name = f"{order[0][0]}-first"
            for implementation, executable in order:
                fields, rows = read_rows(executable)
                if base_fields is None:
                    base_fields = fields
                elif fields != base_fields:
                    raise RuntimeError(f"CSV schema mismatch from {executable}")
                for row in rows:
                    output_rows.append({
                        "profile": profile,
                        "pair": pair,
                        "order": order_name,
                        **row,
                    })
    fieldnames = ["profile", "pair", "order", *(base_fields or [])]
    with open(args.output, "w", newline="") as output:
        writer = csv.DictWriter(output, fieldnames=fieldnames)
        writer.writeheader()
        writer.writerows(output_rows)
    print(f"wrote {len(output_rows)} rows to {args.output}")


if __name__ == "__main__":
    try:
        main()
    except (OSError, RuntimeError, subprocess.CalledProcessError) as error:
        print(error, file=sys.stderr)
        raise SystemExit(1)
