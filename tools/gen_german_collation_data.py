#!/usr/bin/env python3
"""Generate canonical Latin Extended-A weights for standalone German collation."""

from __future__ import annotations

import unicodedata
from pathlib import Path

ROOT = Path(__file__).resolve().parents[1]
SOURCE = ROOT / "src/locale_standalone.nelua"
VERSION = "16.0.0"
START = "-- BEGIN GENERATED GERMAN LATIN EXTENDED-A WEIGHTS"
END = "-- END GENERATED GERMAN LATIN EXTENDED-A WEIGHTS"

ACCENTS = {
    0x0300: 0x22,  # grave
    0x0301: 0x23,  # acute
    0x0302: 0x24,  # circumflex
    0x0303: 0x25,  # tilde
    0x0304: 0x2A,  # macron
    0x0306: 0x29,  # breve
    0x0307: 0x2C,  # dot above
    0x0308: 0x21,  # diaeresis
    0x030A: 0x26,  # ring above
    0x030B: 0x2D,  # double acute
    0x0327: 0x27,  # cedilla
    0x0328: 0x2B,  # ogonek
    0x030C: 0x28,  # caron
}


def generate() -> str:
    if unicodedata.unidata_version != VERSION:
        raise SystemExit(
            f"need Python Unicode {VERSION}; found {unicodedata.unidata_version}"
        )
    rows: list[tuple[int, int, int, int]] = []
    for codepoint in range(0x0100, 0x0180):
        decomposition = unicodedata.normalize("NFD", chr(codepoint))
        if len(decomposition) != 2:
            continue
        base, mark = map(ord, decomposition)
        if not (0x41 <= base <= 0x5A or 0x61 <= base <= 0x7A):
            continue
        if mark not in ACCENTS:
            continue
        uppercase = chr(codepoint).isupper()
        base |= 0x20 if uppercase else 0
        rows.append((codepoint, base, ACCENTS[mark], 0x21 if uppercase else 0x20))

    lines = [START, f"-- Generated from Unicode {VERSION}; do not edit by hand."]
    lines.append(f"local german_extended_a_weight_count: csize = {len(rows)}_csize")
    lines.append(f"local german_extended_a_weights: [{4 * len(rows)}]cuint = {{")
    values = [part for row in rows for part in row]
    for offset in range(0, len(values), 8):
        lines.append("  " + ", ".join(f"0x{x:x}_cuint" for x in values[offset:offset + 8]) + ",")
    lines.extend(["}", END])
    return "\n".join(lines)


def main() -> None:
    text = SOURCE.read_text()
    generated = generate()
    if START not in text or END not in text:
        raise SystemExit("generated collation markers missing from locale source")
    before, remainder = text.split(START, 1)
    _, after = remainder.split(END, 1)
    SOURCE.write_text(before + generated + after)


if __name__ == "__main__":
    main()
