<div align="center">

# nelibc

**A C library, rebuilt piece by piece in [Nelua](https://nelua.io).**

Replace selected functions with Nelua implementations. Keep uncovered calls on
glibc or musl. Grow the replacement toward a standalone runtime.

[Quickstart](docs/quickstart.md) · [API coverage](docs/musl-1.2.5-api-coverage.csv)
· [Roadmap](docs/core-roadmap.md) · [Contributing](#contributing)

**Experimental** · Linux x86-64 · Hybrid builds usable · Standalone in progress

</div>

## Try it

Requirements: [Nelua](https://nelua.io) and [Zig](https://ziglang.org/).

```sh
git clone https://github.com/Spaceghost/nelibc.git
cd nelibc
make dogfood-zig
```

This builds Nelibc, links a C consumer, runs it, and verifies that Nelibc's
symbols are present in the executable.

To link your own program:

```sh
make lib-zig
zig cc -Iinclude -fno-builtin app.c build/libnelibc-zig.a -o app
```

`-fno-builtin` ensures calls reach the library symbols. The
[quickstart](docs/quickstart.md) has GCC, musl, and TinyCC commands.

## Where it stands

The pinned musl 1.2.5 inventory tracks **1,471 API symbols**. This measures
symbol coverage, not behavioral conformance.

| Hybrid coverage | Count |
| --- | ---: |
| Exported by Nelibc in at least one tested profile | 808 |
| Deliberately supplied by host libc | 223 |
| Not yet declared by Nelibc | 440 |

| Area | Status |
| --- | --- |
| Memory, strings, conversions | Broad; hybrid and standalone |
| Math | Broad real and complex coverage; edge cases remain |
| Stdio | Hybrid support; standalone coverage growing |
| Files, processes, threads | Linux x86-64 base; standalone partial |
| Locale, wide chars, networking | Partial; resolver APIs remain |

The standalone runtime has its own coverage profile and is still incomplete.
See the [full inventory](docs/musl-1.2.5-api-coverage.csv) and
[coverage method](docs/api-coverage.md) for per-profile detail.

## Build profiles

Zig cc is the default toolchain. GCC and TinyCC provide comparisons; hybrid
archives can target glibc or musl.

```sh
make dogfood-matrix
make smoke-matrix
```

TinyCC is optional. See [quickstart](docs/quickstart.md) for requirements and
profile-specific commands.

## Project guide

| Read this | For |
| --- | --- |
| [Quickstart](docs/quickstart.md) | Build and link a C program |
| [API inventory](docs/musl-1.2.5-api-coverage.csv) | Function and symbol coverage |
| [Coverage notes](docs/api-coverage.md) | How coverage is measured |
| [Core roadmap](docs/core-roadmap.md) | Priorities and missing core pieces |
| [Implementation catalog](docs/implementation-catalog.md) | Implementations and smoke tests |

Refresh local per-profile coverage with `make api-coverage`.

## Contributing

Choose a small API slice, implement it in Nelua, expose its C ABI, and add a C
consumer check. Record host delegation in the coverage tracker. See the
[implementation catalog](docs/implementation-catalog.md) for existing patterns.

## License

MIT. See [third-party notices](THIRD_PARTY_NOTICES.md) for fdlibm-derived math
code.
