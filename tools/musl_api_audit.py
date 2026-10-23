#!/usr/bin/env python3
"""Compare Nelibc's declared/exported APIs with a pinned musl header+archive set."""

from __future__ import annotations

import argparse
import csv
import glob
import re
import shlex
import subprocess
import sys
from pathlib import Path

sys.dont_write_bytecode = True

from api_coverage import HOST_DELEGATED, declaration_names, is_public_api_name


ROOT = Path(__file__).resolve().parents[1]


def header_apis(directory: Path) -> dict[str, list[str]]:
    found: dict[str, list[str]] = {}
    for filename in sorted(glob.glob(str(directory / "**/*.h"), recursive=True)):
        path = Path(filename)
        text = path.read_text(errors="replace")
        # The project declaration scanner is intentionally small; remove the
        # C++ linkage braces used by musl before passing each header to it.
        text = re.sub(r'extern\s+"C"\s*\{', "", text)
        text = re.sub(r"^\s*}\s*$", "", text, flags=re.M)
        for name in declaration_names(text):
            if is_public_api_name(name):
                found.setdefault(name, []).append(path.relative_to(directory).as_posix())
    return found


def archive_exports(nm: list[str], archives: list[Path]) -> set[str]:
    result: set[str] = set()
    for archive in archives:
        output = subprocess.check_output(
            [*nm, "-g", "--defined-only", str(archive)], text=True,
            stderr=subprocess.DEVNULL, cwd=ROOT,
        )
        for line in output.splitlines():
            fields = line.split()
            if len(fields) >= 3 and fields[-2] in {"T", "W"}:
                result.add(fields[-1])
    return result


def parse_named_path(value: str) -> tuple[str, Path]:
    name, separator, filename = value.partition("=")
    if not separator or not name or not filename:
        raise argparse.ArgumentTypeError("profile must be NAME=ARCHIVE")
    return name, Path(filename)


def main() -> int:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--musl-include", type=Path, required=True)
    parser.add_argument("--musl-archive", type=Path, action="append", required=True)
    parser.add_argument("--nelibc-include", type=Path, default=ROOT / "include")
    parser.add_argument("--nelibc-profile", action="append", type=parse_named_path,
                        default=[], metavar="NAME=ARCHIVE")
    parser.add_argument("--nm", default="nm")
    parser.add_argument("--output", type=Path, default=ROOT / "build/musl-api-audit.csv")
    args = parser.parse_args()

    for path in [args.musl_include, args.nelibc_include, *args.musl_archive,
                 *(archive for _, archive in args.nelibc_profile)]:
        if not path.exists():
            parser.error(f"input does not exist: {path}")

    nm = shlex.split(args.nm)
    musl_headers = header_apis(args.musl_include)
    musl_exports = archive_exports(nm, args.musl_archive)
    # Intersect declarations with the built release archives. This removes
    # function-like macros and parser false positives from the baseline.
    baseline = {name: headers for name, headers in musl_headers.items()
                if name in musl_exports}

    nelibc_headers = header_apis(args.nelibc_include)
    profiles = dict(args.nelibc_profile)
    nelibc_exports = {
        profile: archive_exports(nm, [archive])
        for profile, archive in profiles.items()
    }
    output = args.output if args.output.is_absolute() else ROOT / args.output
    output.parent.mkdir(parents=True, exist_ok=True)
    columns = ["api", "musl_headers", "nelibc_headers", "nelibc_exports", "status"]
    counts = {"implemented": 0, "host-delegated": 0, "declared-not-exported": 0,
              "not-declared": 0}
    with output.open("w", newline="") as stream:
        writer = csv.DictWriter(stream, fieldnames=columns, lineterminator="\n")
        writer.writeheader()
        for name in sorted(baseline):
            declared = name in nelibc_headers
            exported_profiles = [profile for profile, symbols in nelibc_exports.items()
                                 if name in symbols]
            delegated_profiles = sorted(set(profiles) & HOST_DELEGATED.get(name, set()))
            if exported_profiles:
                status = "implemented"
            elif delegated_profiles:
                status = "host-delegated"
            elif declared:
                status = "declared-not-exported"
            else:
                status = "not-declared"
            counts[status] += 1
            writer.writerow({
                "api": name,
                "musl_headers": ";".join(sorted(set(musl_headers[name]))),
                "nelibc_headers": ";".join(sorted(set(nelibc_headers.get(name, [])))),
                "nelibc_exports": ";".join(exported_profiles),
                "status": status,
            })
    print(f"musl public APIs: {len(baseline)}")
    for status, count in counts.items():
        print(f"{status}: {count}")
    print(f"wrote {output}")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
