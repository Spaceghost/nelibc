<div align="center">

# nelibc

**A C library, rebuilt piece by piece in [Nelua](https://nelua.io).**

Replace the functions you need. Keep the rest from glibc or musl.
Grow the replacement toward a standalone runtime.

[Quickstart](docs/quickstart.md) · [Coverage](docs/musl-1.2.5-api-coverage.csv)
· [Roadmap](docs/core-roadmap.md) · [Contributing](#contribute)

Experimental · Linux x86-64 · Hybrid usable · Standalone in progress

</div>

## Try it

Needs [Nelua](https://nelua.io) and [Zig](https://ziglang.org/).

```sh
git clone https://github.com/Spaceghost/nelibc.git
cd nelibc
make dogfood-zig
```

This builds Nelibc, links a C program against it, runs the program, and checks
that Nelibc's symbols made it into the executable.

Use Nelibc in your own C program:

```sh
make lib-zig
zig cc -Iinclude -fno-builtin app.c build/libnelibc-zig.a -o app
```

`-fno-builtin` keeps compiler builtins from bypassing the library. See the
[quickstart](docs/quickstart.md) for GCC, musl, and TinyCC commands.

## Project status

The pinned musl 1.2.5 comparison tracks **1,471 APIs**:

- **804** exported by Nelibc in at least one tested hybrid profile
- **222** provided by the host libc in hybrid profiles
- **445** not yet declared by Nelibc

These are symbol-surface counts, not conformance scores. The standalone
runtime is a separate, incomplete profile.

| Area | Status |
| --- | --- |
| Memory, strings, conversions | Broad implementation; hybrid and standalone |
| Math | Broad real and complex coverage; edge cases remain |
| Stdio | Hybrid support; standalone coverage growing |
| Files, processes, threads | Linux x86-64 base; standalone coverage partial |
| Locale, wide chars, networking | Partial locale and Unicode; resolver APIs remain |

## Coverage details

The API audit tracks declarations, archive exports, and host delegation.
Symbol presence and behavioral coverage are reported separately.

- [Full function and symbol inventory](docs/musl-1.2.5-api-coverage.csv)
- [How coverage is measured](docs/api-coverage.md)
- [Core roadmap](docs/core-roadmap.md)
- [Implementation and smoke catalog](docs/implementation-catalog.md)

Refresh the local, per-profile report with `make api-coverage`.

## Compare builds

Zig cc is the default. GCC and TinyCC are comparison toolchains; hybrid builds
also cover glibc and musl.

```sh
make dogfood-matrix
make smoke-matrix
```

TinyCC is optional. See the [quickstart](docs/quickstart.md) for setup details.

## Contribute

Pick a small API slice, implement it in Nelua, expose its C ABI, and add a C
consumer check. Record host delegation in the coverage tracker.

## License

MIT. See [third-party notices](THIRD_PARTY_NOTICES.md) for fdlibm-derived math
code.
