#!/usr/bin/env python3
"""Build a declaration/export/smoke-reference matrix for Nelibc APIs."""

from __future__ import annotations

import argparse
import csv
import glob
import re
import shlex
import subprocess
from pathlib import Path


ROOT = Path(__file__).resolve().parents[1]
DEFAULT_PROFILES = {
    "zig_glibc": "build/libnelibc-zig.a",
    "gcc_glibc": "build/libnelibc-gcc.a",
    "zig_musl": "build/libnelibc-zig-musl.a",
    "tcc_glibc": "build/libnelibc-tcc.a",
    "zig_standalone": "build/libnelibc-standalone-zig.a",
    "gcc_standalone": "build/libnelibc-standalone-gcc.a",
    "tcc_standalone": "build/libnelibc-standalone-tcc.a",
}
HOST_DELEGATED = {
    name: {"zig_glibc", "gcc_glibc", "zig_musl", "tcc_glibc"}
    for name in (
        "pthread_attr_init", "pthread_attr_destroy",
        "pthread_attr_setdetachstate", "pthread_attr_getdetachstate",
        "pthread_attr_setscope", "pthread_attr_getscope",
        "pthread_attr_setstacksize", "pthread_attr_getstacksize",
        "pthread_attr_setstack", "pthread_attr_getstack",
        "pthread_attr_setguardsize", "pthread_attr_getguardsize",
        "pthread_attr_setinheritsched", "pthread_attr_getinheritsched",
        "pthread_attr_setschedpolicy", "pthread_attr_getschedpolicy",
        "pthread_attr_setschedparam", "pthread_attr_getschedparam",
        "pthread_create", "pthread_cancel", "pthread_join", "pthread_detach", "pthread_equal",
        "pthread_self", "pthread_exit", "pthread_testcancel",
        "pthread_setcancelstate", "pthread_setcanceltype", "pthread_mutexattr_init",
        "pthread_getschedparam", "pthread_setschedparam",
        "pthread_once",
        "pthread_key_create", "pthread_key_delete", "pthread_getspecific",
        "pthread_setspecific",
        "pthread_mutexattr_destroy", "pthread_mutexattr_settype",
        "pthread_mutexattr_gettype", "pthread_mutexattr_setpshared",
        "pthread_mutexattr_getpshared", "pthread_mutexattr_setrobust",
        "pthread_mutexattr_getrobust", "pthread_mutex_init",
        "pthread_mutex_consistent", "pthread_mutex_destroy", "pthread_mutex_lock",
        "pthread_mutex_timedlock",
        "pthread_mutex_trylock", "pthread_mutex_unlock",
        "pthread_spin_init", "pthread_spin_destroy", "pthread_spin_lock",
        "pthread_spin_trylock", "pthread_spin_unlock",
        "pthread_condattr_init", "pthread_condattr_destroy",
        "pthread_condattr_setpshared", "pthread_condattr_getpshared",
        "pthread_cond_init", "pthread_cond_destroy", "pthread_cond_wait",
        "pthread_cond_timedwait", "pthread_cond_signal",
        "pthread_cond_broadcast",
        "pthread_barrierattr_init", "pthread_barrierattr_destroy",
        "pthread_barrierattr_setpshared", "pthread_barrierattr_getpshared",
        "pthread_barrier_init", "pthread_barrier_destroy", "pthread_barrier_wait",
        "pthread_rwlockattr_init", "pthread_rwlockattr_destroy",
        "pthread_rwlockattr_setpshared", "pthread_rwlockattr_getpshared",
        "pthread_rwlock_init", "pthread_rwlock_destroy", "pthread_rwlock_rdlock",
        "pthread_rwlock_tryrdlock", "pthread_rwlock_timedrdlock",
        "pthread_rwlock_wrlock", "pthread_rwlock_trywrlock",
        "pthread_rwlock_timedwrlock", "pthread_rwlock_unlock",
        "sem_init", "sem_destroy", "sem_wait", "sem_timedwait",
        "sem_clockwait",
        "sem_trywait", "sem_post", "sem_getvalue", "sem_open", "sem_close",
        "sem_unlink",
    )
}
SMOKE_GLOBS = ("tools/*_smoke.c", "tools/*_diff.c", "tools/*_smoke.nelua")


def remove_comments(text: str) -> str:
    text = re.sub(r"/\*.*?\*/", " ", text, flags=re.S)
    return re.sub(r"//[^\n]*", " ", text)


def declaration_names(text: str) -> set[str]:
    """Extract external function declarators from the project's C headers.

    This intentionally handles the declaration forms used by these headers;
    it is not a general C parser. Struct bodies, directives, and function
    bodies are removed before each top-level declaration is inspected.
    """
    text = remove_comments(text)
    text = text.replace("\\\n", " ")
    text = re.sub(r"^[ \t]*#.*$", " ", text, flags=re.M)
    text = re.sub(r'"(?:\\.|[^"\\])*"|\'(?:\\.|[^\'\\])*\'', " ", text)

    output: list[str] = []
    depth = 0
    for char in text:
        if char == "{":
            depth += 1
            if depth == 1:
                output.append(";")
            continue
        if char == "}":
            depth = max(0, depth - 1)
            continue
        if depth == 0:
            output.append(char)

    declarations = "".join(output).split(";")
    names: set[str] = set()
    for declaration in declarations:
        declaration = declaration.strip()
        if not declaration or declaration.startswith("typedef"):
            continue
        if re.match(r"(?:static|inline)\b", declaration):
            continue
        match = re.search(r"\b([A-Za-z_]\w*)\s*\(", declaration)
        if match:
            names.add(match.group(1))
    return names


def profile_exports(nm: list[str], archive: Path) -> set[str]:
    output = subprocess.check_output(
        [*nm, "-g", "--defined-only", str(archive)], text=True, cwd=ROOT
    )
    exports = set()
    for line in output.splitlines():
        fields = line.split()
        if len(fields) >= 3 and fields[-2] in {"T", "W"}:
            name = fields[-1]
            exports.add(name)
            # Static musl's strong libc definitions cannot always be replaced
            # directly; consumers use archive __wrap_* entry points with
            # -Wl,--wrap=<symbol>. The allocator bridges dispatch free/realloc
            # by ownership so host and Nelibc allocations can coexist.
            if name in {
                "__wrap_fread", "__wrap_fwrite", "__wrap_fflush",
                "__wrap_fileno", "__wrap_feof", "__wrap_ferror",
                "__wrap_clearerr", "__wrap_ungetc", "__wrap_freopen",
                "__wrap_fgetpos", "__wrap_fsetpos", "__wrap_fseek",
                "__wrap_ftell", "__wrap_fseeko", "__wrap_ftello",
                "__wrap_setvbuf",
                "__wrap_flockfile", "__wrap_ftrylockfile", "__wrap_funlockfile",
                "__wrap_fclose",
                "__wrap_localeconv",
                "__wrap_setlocale",
                "__wrap_atexit",
                "__wrap_exit",
                "__wrap_free",
                "__wrap_realloc",
            }:
                exports.add(name.removeprefix("__wrap_"))
    return exports


def implementation_sources(names: set[str]) -> dict[str, list[str]]:
    found = {name: [] for name in names}
    for pattern in ("src/**/*.nelua", "src/**/*.c", "src/**/*.S"):
        for filename in glob.glob(pattern, root_dir=ROOT, recursive=True):
            path = ROOT / filename
            text = remove_comments(path.read_text(errors="replace"))
            for name in names:
                if path.suffix == ".nelua":
                    present = re.search(
                        rf"codename\s+['\"](?:{re.escape(name)}|__wrap_{re.escape(name)})['\"]",
                        text,
                    )
                elif path.suffix == ".S":
                    present = re.search(rf"(?m)^\s*{re.escape(name)}:", text)
                else:
                    present = re.search(
                        rf"\b{re.escape(name)}\s*\([^;{{}}]*\)\s*\{{", text, re.S
                    )
                    if not present:
                        # Some platform adapters generate public wrappers from
                        # one API name per DEFINE_* macro invocation.
                        present = re.search(
                            rf"(?m)^\s*DEFINE_[A-Z0-9_]+\(\s*{re.escape(name)}\s*,",
                            text,
                        )
                if present:
                    found[name].append(filename)
    return found


def smoke_references(names: set[str]) -> dict[str, list[str]]:
    found = {name: [] for name in names}
    files = set()
    for pattern in SMOKE_GLOBS:
        files.update(glob.glob(pattern, root_dir=ROOT))
    for filename in sorted(files):
        text = remove_comments((ROOT / filename).read_text(errors="replace"))
        text = re.sub(r'"(?:\\.|[^"\\])*"', '""', text)
        for name in names:
            if re.search(rf"\b{re.escape(name)}\b", text):
                found[name].append(filename)
    return found


def parse_profile(value: str) -> tuple[str, Path]:
    name, separator, filename = value.partition("=")
    if not separator or not name or not filename:
        raise argparse.ArgumentTypeError("profile must be NAME=ARCHIVE")
    return name, Path(filename)


def is_public_api_name(name: str) -> bool:
    """Exclude Nelibc internals and compiler runtime helper entry points."""
    if name.startswith(("nelibc_", "__nelibc_", "__wrap_")) or name == "_start":
        return False
    return not re.match(r"__(?:fix|float|va_arg)", name)


def main() -> int:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--nm", default="nm", help="nm command (default: nm)")
    parser.add_argument(
        "--profile", action="append", type=parse_profile,
        help="archive profile in NAME=ARCHIVE form; may be repeated",
    )
    parser.add_argument("--output", default="build/api-coverage.csv")
    args = parser.parse_args()

    profiles = dict(args.profile or [])
    if not profiles:
        profiles = DEFAULT_PROFILES
    missing_archives = [path for path in profiles.values() if not (ROOT / path).is_file()]
    if missing_archives:
        parser.error("missing archive(s): " + ", ".join(map(str, missing_archives)))

    headers = sorted(glob.glob("include/**/*.h", root_dir=ROOT, recursive=True))
    declarations: dict[str, set[str]] = {}
    for header in headers:
        names = declaration_names((ROOT / header).read_text(errors="replace"))
        for name in names:
            if not is_public_api_name(name):
                continue
            declarations.setdefault(name, set()).add(header)

    nm = shlex.split(args.nm)
    exports = {
        profile: profile_exports(nm, ROOT / archive)
        for profile, archive in profiles.items()
    }
    symbols = set(declarations)
    for exported in exports.values():
        symbols.update(name for name in exported if is_public_api_name(name))
    sources = implementation_sources(symbols)
    smoke = smoke_references(symbols)

    profile_names = list(profiles)
    columns = [
        "api", "status", "declared_headers", "implementation_sources",
        *profile_names, "smoke_reference_files",
    ]
    output = ROOT / args.output
    output.parent.mkdir(parents=True, exist_ok=True)
    counts = {profile: 0 for profile in profile_names}
    declared_count = 0
    with output.open("w", newline="") as stream:
        writer = csv.DictWriter(stream, fieldnames=columns)
        writer.writeheader()
        for name in sorted(symbols):
            header_list = sorted(declarations.get(name, set()))
            implementations = sorted(sources.get(name, []))
            present = {profile: name in exports[profile] for profile in profile_names}
            declared_count += bool(header_list)
            for profile, is_exported in present.items():
                counts[profile] += is_exported
            missing = [profile for profile, is_exported in present.items() if not is_exported]
            standalone_missing = [
                profile for profile in missing if profile.endswith("_standalone")
            ]
            unclassified_missing = [
                profile for profile in missing
                if profile not in HOST_DELEGATED.get(name, set())
            ]
            if header_list and standalone_missing:
                status = "missing-standalone-export:" + ",".join(standalone_missing)
            elif header_list and unclassified_missing:
                status = "standalone-export;unexported:" + ",".join(unclassified_missing)
            elif header_list and missing:
                status = "implemented;host-delegated:" + ",".join(missing)
            elif header_list and not implementations:
                status = "exported;source-unmapped"
            elif header_list:
                status = "implemented"
            elif implementations:
                status = "implementation-only"
            else:
                status = "export-only"
            row = {
                "api": name,
                "status": status,
                "declared_headers": ";".join(header_list),
                "implementation_sources": ";".join(implementations),
                "smoke_reference_files": ";".join(smoke.get(name, [])),
            }
            row.update({profile: "yes" if value else "no" for profile, value in present.items()})
            writer.writerow(row)

    print(f"Declared API names: {declared_count}")
    for profile in profile_names:
        print(f"{profile} exported functions: {counts[profile]}")
    print(f"Manifest rows: {len(symbols)} -> {output.relative_to(ROOT)}")
    print("Smoke-reference columns indicate a name occurs in a smoke/differential file;")
    print("they do not prove that every path or edge case is tested.")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
