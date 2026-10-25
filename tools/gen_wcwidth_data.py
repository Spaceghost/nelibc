#!/usr/bin/env python3
"""Generate standalone wide-character tables from Python's Unicode 16.0 database."""

from __future__ import annotations

import argparse
import hashlib
import unicodedata
from pathlib import Path

ROOT = Path(__file__).resolve().parents[1]
SOURCE = ROOT / "src/locale_standalone.nelua"
VERSION = "16.0.0"
START = "-- BEGIN GENERATED UNICODE WIDTH TABLES"
END = "-- END GENERATED UNICODE WIDTH TABLES"

# Default_Ignorable_Code_Point ranges from DerivedCoreProperties-16.0.0.txt.
# U+00AD SOFT HYPHEN is intentionally width one despite this property.
DEFAULT_IGNORABLE = (
    (0x00AD, 0x00AD), (0x034F, 0x034F), (0x061C, 0x061C),
    (0x1160, 0x1160), (0x17B4, 0x17B5), (0x180B, 0x180D),
    (0x180E, 0x180E), (0x180F, 0x180F), (0x200B, 0x200F),
    (0x202A, 0x202E), (0x2060, 0x2064), (0x2065, 0x2065),
    (0x2066, 0x206F), (0x3164, 0x3164), (0xFE00, 0xFE0F),
    (0xFEFF, 0xFEFF), (0xFFA0, 0xFFA0), (0xFFF0, 0xFFF8),
    (0x1BCA0, 0x1BCA3), (0x1D173, 0x1D17A), (0xE0000, 0xE0000),
    (0xE0001, 0xE0001), (0xE0002, 0xE001F), (0xE0020, 0xE007F),
    (0xE0080, 0xE00FF), (0xE0100, 0xE01EF), (0xE01F0, 0xE0FFF),
)


def property_ranges(predicate) -> list[tuple[int, int]]:
    ranges: list[tuple[int, int]] = []
    first: int | None = None
    last = 0
    for codepoint in range(0x110000):
        if predicate(codepoint):
            if first is None:
                first = codepoint
            last = codepoint
        elif first is not None:
            ranges.append((first, last))
            first = None
    if first is not None:
        ranges.append((first, last))
    return ranges


def flatten(ranges: list[tuple[int, int]]) -> str:
    values = [f"0x{part:x}" for pair in ranges for part in pair]
    lines = []
    for offset in range(0, len(values), 8):
        lines.append("  " + ", ".join(values[offset:offset + 8]) + ",")
    return "\n".join(lines)


def nelua_array(name: str, ranges: list[tuple[int, int]]) -> str:
    values = flatten(ranges)
    return f"local {name}: [{len(ranges) * 2}]cuint = {{\n{values}\n}}\n"


def simple_case_pairs(method: str) -> list[tuple[int, int]]:
    pairs = []
    for codepoint in range(0x110000):
        mapped = getattr(chr(codepoint), method)()
        if len(mapped) == 1 and ord(mapped) != codepoint:
            pairs.append((codepoint, ord(mapped)))
    # Python exposes full casing. These entries preserve UnicodeData.txt's
    # simple mappings where full casing expands to multiple codepoints.
    overrides = {
        "lower": {0x0130: 0x0069},
        "upper": {
            0x1F80: 0x1F88, 0x1F81: 0x1F89, 0x1F82: 0x1F8A,
            0x1F83: 0x1F8B, 0x1F84: 0x1F8C, 0x1F85: 0x1F8D,
            0x1F86: 0x1F8E, 0x1F87: 0x1F8F, 0x1F90: 0x1F98,
            0x1F91: 0x1F99, 0x1F92: 0x1F9A, 0x1F93: 0x1F9B,
            0x1F94: 0x1F9C, 0x1F95: 0x1F9D, 0x1F96: 0x1F9E,
            0x1F97: 0x1F9F, 0x1FA0: 0x1FA8, 0x1FA1: 0x1FA9,
            0x1FA2: 0x1FAA, 0x1FA3: 0x1FAB, 0x1FA4: 0x1FAC,
            0x1FA5: 0x1FAD, 0x1FA6: 0x1FAE, 0x1FA7: 0x1FAF,
            0x1FB3: 0x1FBC, 0x1FC3: 0x1FCC, 0x1FF3: 0x1FFC,
        },
    }[method]
    pairs = [(key, value) for key, value in pairs if key not in overrides]
    pairs.extend(overrides.items())
    pairs.sort()
    return pairs


def nelua_pairs(name: str, pairs: list[tuple[int, int]]) -> str:
    values = [f"0x{part:x}" for pair in pairs for part in pair]
    lines = []
    for offset in range(0, len(values), 8):
        lines.append("  " + ", ".join(values[offset:offset + 8]) + ",")
    return f"local {name}: [{len(pairs) * 2}]cuint = {{\n" + "\n".join(lines) + "\n}\n"


def render() -> str:
    if unicodedata.unidata_version != VERSION:
        raise SystemExit(
            f"need Python Unicode {VERSION}; found {unicodedata.unidata_version}"
        )

    zero_ranges = property_ranges(
        lambda cp: unicodedata.category(chr(cp)) in ("Mn", "Me")
    )
    zero_ranges.extend(DEFAULT_IGNORABLE)
    zero_ranges = [pair for pair in zero_ranges if pair != (0x00AD, 0x00AD)]
    zero_ranges.sort()
    # Merge overlaps and adjacent intervals after adding derived format data.
    merged: list[tuple[int, int]] = []
    for first, last in zero_ranges:
        if merged and first <= merged[-1][1] + 1:
            merged[-1] = (merged[-1][0], max(last, merged[-1][1]))
        else:
            merged.append((first, last))
    zero_ranges = merged

    nonprint_ranges = property_ranges(
        lambda cp: unicodedata.category(chr(cp)) in ("Cc", "Cs", "Cn", "Zl", "Zp")
    )
    wide_ranges = property_ranges(
        lambda cp: unicodedata.east_asian_width(chr(cp)) in ("W", "F")
        and unicodedata.category(chr(cp)) not in ("Cc", "Cs", "Cn", "Zl", "Zp")
    )
    lower_pairs = simple_case_pairs("lower")
    upper_pairs = simple_case_pairs("upper")
    alpha_ranges = property_ranges(
        lambda cp: unicodedata.category(chr(cp)).startswith("L")
        or unicodedata.category(chr(cp)) == "Nl"
    )
    lower_ranges = property_ranges(lambda cp: chr(cp).islower())
    upper_ranges = property_ranges(
        lambda cp: chr(cp).isupper() or unicodedata.category(chr(cp)) == "Lt"
    )
    # Unicode White_Space; Python's isspace() also includes four information
    # separators that are not members of the UCD White_Space property.
    space_ranges = [
        (0x0009, 0x000d), (0x0020, 0x0020), (0x0085, 0x0085),
        (0x00a0, 0x00a0), (0x1680, 0x1680), (0x2000, 0x200a),
        (0x2028, 0x2029), (0x202f, 0x202f), (0x205f, 0x205f),
        (0x3000, 0x3000),
    ]
    control_ranges = property_ranges(
        lambda cp: unicodedata.category(chr(cp)) in ("Cc", "Zl", "Zp")
    )

    body = [
        f"-- Generated from Unicode Character Database {VERSION}; see THIRD_PARTY_NOTICES.md.",
        nelua_array("wcwidth_zero_ranges", zero_ranges),
        nelua_array("wcwidth_wide_ranges", wide_ranges),
        nelua_array("wcwidth_nonprint_ranges", nonprint_ranges),
        nelua_pairs("wcase_lower_pairs", lower_pairs),
        nelua_pairs("wcase_upper_pairs", upper_pairs),
        nelua_array("wctype_alpha_ranges", alpha_ranges),
        nelua_array("wctype_lower_ranges", lower_ranges),
        nelua_array("wctype_upper_ranges", upper_ranges),
        nelua_array("wctype_space_ranges", space_ranges),
        nelua_array("wctype_control_ranges", control_ranges),
        """local function wcwidth_in_ranges(value: cuint, ranges: *[0]cuint,
    count: csize): boolean <inline>
  local low: csize = 0
  local high: csize = count
  while low < high do
    local middle: csize = low + (high - low) // 2
    local first = ranges[middle * 2]
    local last = ranges[middle * 2 + 1]
    if value < first then
      high = middle
    elseif value > last then
      low = middle + 1
    else
      return true
    end
  end
  return false
end

local function wcase_map(value: cwchar_t, pairs: *[0]cuint,
    count: csize): cwchar_t <inline>
  local low: csize = 0
  local high: csize = count
  local codepoint = (@cuint)(value)
  while low < high do
    local middle: csize = low + (high - low) // 2
    local key = pairs[middle * 2]
    if codepoint < key then
      high = middle
    elseif codepoint > key then
      low = middle + 1
    else
      return (@cwchar_t)(pairs[middle * 2 + 1])
    end
  end
  return value
end

local function towlower(value: cwchar_t): cwchar_t <cexport, codename 'towlower'>
  if value < 0 or value > 0x10ffff or (value >= 0xd800 and value <= 0xdfff) then
    return value
  end
  if nelibc_multibyte_mode() == 0 then
    if value >= 65 and value <= 90 then return value + 32 end
    return value
  end
  return wcase_map(value, &wcase_lower_pairs[0], #wcase_lower_pairs // 2)
end

local function towupper(value: cwchar_t): cwchar_t <cexport, codename 'towupper'>
  if value < 0 or value > 0x10ffff or (value >= 0xd800 and value <= 0xdfff) then
    return value
  end
  if nelibc_multibyte_mode() == 0 then
    if value >= 97 and value <= 122 then return value - 32 end
    return value
  end
  return wcase_map(value, &wcase_upper_pairs[0], #wcase_upper_pairs // 2)
end

local function wctype_isin(value: cwchar_t, ranges: *[0]cuint,
    count: csize): boolean <inline>
  if value < 0 or value > 0x10ffff or (value >= 0xd800 and value <= 0xdfff) then
    return false
  end
  return wcwidth_in_ranges((@cuint)(value), ranges, count)
end

local function iswalpha(value: cwchar_t): cint <cexport, codename 'iswalpha'>
  if nelibc_multibyte_mode() == 0 then
    return ((value >= 65 and value <= 90) or (value >= 97 and value <= 122)) and 1 or 0
  end
  return wctype_isin(value, &wctype_alpha_ranges[0], #wctype_alpha_ranges // 2) and 1 or 0
end

local function iswlower(value: cwchar_t): cint <cexport, codename 'iswlower'>
  if nelibc_multibyte_mode() == 0 then return (value >= 97 and value <= 122) and 1 or 0 end
  return wctype_isin(value, &wctype_lower_ranges[0], #wctype_lower_ranges // 2) and 1 or 0
end

local function iswupper(value: cwchar_t): cint <cexport, codename 'iswupper'>
  if nelibc_multibyte_mode() == 0 then return (value >= 65 and value <= 90) and 1 or 0 end
  return wctype_isin(value, &wctype_upper_ranges[0], #wctype_upper_ranges // 2) and 1 or 0
end

local function iswdigit(value: cwchar_t): cint <cexport, codename 'iswdigit'>
  return (value >= 48 and value <= 57) and 1 or 0
end

local function iswxdigit(value: cwchar_t): cint <cexport, codename 'iswxdigit'>
  return ((value >= 48 and value <= 57) or (value >= 65 and value <= 70) or
      (value >= 97 and value <= 102)) and 1 or 0
end

local function iswspace(value: cwchar_t): cint <cexport, codename 'iswspace'>
  if nelibc_multibyte_mode() == 0 then
    return ((value >= 9 and value <= 13) or value == 32) and 1 or 0
  end
  return wctype_isin(value, &wctype_space_ranges[0], #wctype_space_ranges // 2) and 1 or 0
end

local function iswblank(value: cwchar_t): cint <cexport, codename 'iswblank'>
  return (value == 9 or value == 32) and 1 or 0
end

local function iswcntrl(value: cwchar_t): cint <cexport, codename 'iswcntrl'>
  if nelibc_multibyte_mode() == 0 then
    return ((value >= 0 and value <= 0x1f) or value == 0x7f) and 1 or 0
  end
  return wctype_isin(value, &wctype_control_ranges[0], #wctype_control_ranges // 2) and 1 or 0
end

local function iswprint(value: cwchar_t): cint <cexport, codename 'iswprint'>
  if nelibc_multibyte_mode() == 0 then
    return (value >= 0x20 and value <= 0x7e) and 1 or 0
  end
  if value < 0 or value > 0x10ffff or (value >= 0xd800 and value <= 0xdfff) then return 0 end
  return not wcwidth_in_ranges((@cuint)(value), &wcwidth_nonprint_ranges[0],
      #wcwidth_nonprint_ranges // 2) and 1 or 0
end

local function iswgraph(value: cwchar_t): cint <cexport, codename 'iswgraph'>
  if iswprint(value) == 0 or iswspace(value) ~= 0 then return 0 end
  return 1
end

local function iswalnum(value: cwchar_t): cint <cexport, codename 'iswalnum'>
  if iswalpha(value) ~= 0 or iswdigit(value) ~= 0 then return 1 end
  return 0
end

local function iswpunct(value: cwchar_t): cint <cexport, codename 'iswpunct'>
  if iswgraph(value) ~= 0 and iswalnum(value) == 0 then return 1 end
  return 0
end""",
        """local function wcwidth(value: cwchar_t): cint <cexport, codename 'wcwidth'>
  if value == 0 then return 0 end
  if value < 0 or value > 0x10ffff or (value >= 0xd800 and value <= 0xdfff) then
    return -1
  end
  if value < 0x20 or (value >= 0x7f and value <= 0x9f) then return -1 end
  local codepoint = (@cuint)(value)
  local mode = nelibc_multibyte_mode()
  if mode == 0 then
    if value < 0x7f then return 1 end
    return -1
  elseif mode ~= 1 and value >= 0x80 then
    return -1
  end
  if wcwidth_in_ranges(codepoint, &wcwidth_nonprint_ranges[0],
      #wcwidth_nonprint_ranges // 2) then return -1 end
  if wcwidth_in_ranges(codepoint, &wcwidth_zero_ranges[0],
      #wcwidth_zero_ranges // 2) then return 0 end
  if wcwidth_in_ranges(codepoint, &wcwidth_wide_ranges[0],
      #wcwidth_wide_ranges // 2) then return 2 end
  return 1
end""",
    ]
    table_body = "\n".join(body[1:]).rstrip() + "\n"
    digest = hashlib.sha256(table_body.encode()).hexdigest()
    return (
        body[0] + "\n"
        + f"-- Data SHA-256: {digest}\n"
        + table_body
    )


def main() -> int:
    parser = argparse.ArgumentParser()
    parser.add_argument("--check", action="store_true")
    args = parser.parse_args()
    source = SOURCE.read_text()
    if source.count(START) != 1 or source.count(END) != 1:
        raise SystemExit("source must contain exactly one generated-section marker pair")
    before, rest = source.split(START, 1)
    section, after = rest.split(END, 1)
    if args.check:
        lines = section.strip().splitlines()
        expected_prefix = f"-- Generated from Unicode Character Database {VERSION};"
        if len(lines) < 3 or not lines[0].startswith(expected_prefix):
            raise SystemExit("generated Unicode width data is missing or has the wrong version")
        digest_prefix = "-- Data SHA-256: "
        if not lines[1].startswith(digest_prefix):
            raise SystemExit("generated Unicode width data is missing its checksum")
        table_body = "\n".join(lines[2:]) + "\n"
        actual = hashlib.sha256(table_body.encode()).hexdigest()
        expected = lines[1][len(digest_prefix):]
        if actual != expected:
            raise SystemExit("generated Unicode width data checksum failed")
        print(f"Unicode {VERSION} wide-character tables are current")
    else:
        rendered = before + START + "\n" + render() + END + after
        SOURCE.write_text(rendered)
        print(f"Generated Unicode {VERSION} width tables in {SOURCE.relative_to(ROOT)}")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
