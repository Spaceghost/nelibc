<div align="center">

# nelibc

### A C library, rebuilt piece by piece in [Nelua](https://nelua.io)

Replace selected libc functions with Nelua implementations. Keep the rest on
glibc or musl, then grow the replacement toward a standalone runtime.

[Quickstart](docs/quickstart.md) ·
[API coverage](docs/musl-1.2.5-api-coverage.csv) ·
[Roadmap](docs/core-roadmap.md) ·
[Contributing](#contributing)

**Experimental** · **Linux x86-64** · **Hybrid builds working** ·
**Standalone in progress**

</div>

## Try it

Install [Nelua](https://nelua.io) and [Zig](https://ziglang.org), then run:

```sh
git clone https://github.com/Spaceghost/nelibc.git
cd nelibc
make dogfood-zig
```

This builds the library, links a C program against it, runs the program, and
checks that Nelibc's symbols made it into the executable.

To link your own C program:

```sh
make lib-zig
zig cc -Iinclude -fno-builtin app.c build/libnelibc-zig.a -o app
```

`-fno-builtin` directs calls to the library symbols under evaluation. The
[quickstart](docs/quickstart.md) includes GCC, musl, and optional TinyCC setups.

## What works today

| Area | Hybrid builds | Standalone |
| --- | --- | --- |
| Memory, strings, conversions | Broad Nelua implementation | Broad and growing |
| Math | Broad real and complex coverage | Broad; edge cases remain |
| Stdio | Mostly host libc | Partial Nelua implementation |
| Files, processes, threads | Host and Nelua mix | Linux x86-64 base; gaps remain |
| Locale, wide chars, networking | Host and Nelua mix | Partial; resolver APIs remain |

Hybrid archives add Nelua implementations while uncovered symbols continue to
come from the selected host libc. Standalone is a separate profile with its
own gaps. The [musl 1.2.5 API inventory](docs/musl-1.2.5-api-coverage.csv)
tracks 1,471 function symbols. In the pinned hybrid comparison:

| Coverage status | Symbols | Share |
| --- | ---: | ---: |
| Exported by Nelibc in at least one hybrid profile | 813 | 55% |
| Delegated to the selected host libc | 223 | 15% |
| Not yet declared by Nelibc | 435 | 30% |

These counts track exported names, not behavioral conformance. See the
[coverage notes](docs/api-coverage.md) for methodology and per-profile gaps.

## Build profiles

Zig cc is the default compiler and linker. GCC and TinyCC are comparison
profiles; hybrid archives can target glibc or musl.

```sh
make dogfood-matrix
make smoke-matrix
```

TinyCC is optional. See the [quickstart](docs/quickstart.md) for requirements
and profile-specific commands.

## Project guide

| Guide | What it covers |
| --- | --- |
| [Quickstart](docs/quickstart.md) | Build and link a C program |
| [API inventory](docs/musl-1.2.5-api-coverage.csv) | Function and symbol coverage |
| [Coverage notes](docs/api-coverage.md) | How coverage is measured |
| [Core roadmap](docs/core-roadmap.md) | Priorities and missing core pieces |
| [Implementation catalog](docs/implementation-catalog.md) | Implementations and consumer checks |

Refresh local per-profile coverage with `make api-coverage`.

## Contributing

Pick a small API slice, implement it in Nelua, expose its C ABI, and add a C
consumer check. Record any host delegation in the coverage tracker. The
[implementation catalog](docs/implementation-catalog.md) shows existing
patterns.

## License

MIT. See [third-party notices](THIRD_PARTY_NOTICES.md) for fdlibm-derived math
code.
