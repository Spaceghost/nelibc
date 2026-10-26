#!/usr/bin/env python3
"""Generate Unicode weights for standalone German collation."""

from __future__ import annotations

import unicodedata
from pathlib import Path

ROOT = Path(__file__).resolve().parents[1]
SOURCE = ROOT / "src/locale_standalone.nelua"
VERSION = "16.0.0"
START = "-- BEGIN GENERATED GERMAN LATIN EXTENDED-A WEIGHTS"
END = "-- END GENERATED GERMAN LATIN EXTENDED-A WEIGHTS"
MARKS_START = "-- BEGIN GENERATED GERMAN COMBINING MARK WEIGHTS"
MARKS_END = "-- END GENERATED GERMAN COMBINING MARK WEIGHTS"

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


def canonical_mark(codepoint: int) -> int:
    decomposition = unicodedata.normalize("NFD", chr(codepoint))
    if len(decomposition) != 1:
        return 0
    return ord(decomposition)


def accent_weight(codepoint: int) -> int:
    canonical = canonical_mark(codepoint)
    if canonical == 0:
        return 0
    return ACCENTS.get(canonical, 0x40 + canonical - 0x0300)


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
        weight = accent_weight(mark)
        if weight == 0:
            continue
        uppercase = chr(codepoint).isupper()
        base |= 0x20 if uppercase else 0
        rows.append((codepoint, base, weight, 0x21 if uppercase else 0x20))

    lines = [START, f"-- Generated from Unicode {VERSION}; do not edit by hand."]
    lines.append(f"local german_extended_a_weight_count: csize = {len(rows)}_csize")
    lines.append(f"local german_extended_a_weights: [{4 * len(rows)}]cuint = {{")
    values = [part for row in rows for part in row]
    for offset in range(0, len(values), 8):
        lines.append("  " + ", ".join(f"0x{x:x}_cuint" for x in values[offset:offset + 8]) + ",")
    lines.extend(["}", END])
    return "\n".join(lines)


def generate_marks() -> str:
    rows = []
    for codepoint in range(0x110000):
        if unicodedata.combining(chr(codepoint)) == 0:
            continue
        decomposition = unicodedata.normalize("NFD", chr(codepoint))
        components = [ord(char) for char in decomposition]
        if len(components) > 2 or any(unicodedata.combining(chr(part)) == 0
                                      for part in components):
            continue
        first, second = components[0], components[1] if len(components) == 2 else 0
        rows.append((codepoint, accent_weight(first), unicodedata.combining(chr(first)),
                     accent_weight(second) if second else 0,
                     unicodedata.combining(chr(second)) if second else 0))

    lines = [MARKS_START, f"-- Generated from Unicode {VERSION}; do not edit by hand."]
    lines.append(f"local german_combining_info_count: csize = {len(rows)}_csize")
    lines.append(f"local german_combining_infos: [{5 * len(rows)}]cuint = {{")
    values = [part for row in rows for part in row]
    for row in rows:
        lines.append("  " + ", ".join(f"0x{x:x}_cuint" for x in row) + ",")
    lines.extend(["}", MARKS_END])
    return "\n".join(lines)


def main() -> None:
    text = SOURCE.read_text()
    for start, end, generated in (
        (START, END, generate()),
        (MARKS_START, MARKS_END, generate_marks()),
    ):
        if start not in text or end not in text:
            raise SystemExit("generated collation markers missing from locale source")
        before, remainder = text.split(start, 1)
        _, after = remainder.split(end, 1)
        text = before + generated + after
    SOURCE.write_text(text)


if __name__ == "__main__":
    main()
