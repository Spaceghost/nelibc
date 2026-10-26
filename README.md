<div align="center">

# nelibc

### A C library, rebuilt piece by piece in [Nelua](https://nelua.io)

Replace selected libc functions with Nelua implementations. Keep using glibc or
musl for the rest, while Nelibc grows toward a standalone runtime.

[Quickstart](docs/quickstart.md) · [API coverage](docs/musl-1.2.5-api-coverage.csv) · [Roadmap](docs/core-roadmap.md) · [Contribute](#contribute)

**Experimental** · Linux x86-64 · Hybrid builds usable · Standalone runtime in progress

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

## What works

| Area | Current state |
| --- | --- |
| Memory, strings, conversions | Broad implementation; checked across build profiles |
| Math | Broad real and complex coverage; edge cases remain |
| Stdio | Hybrid support; standalone support growing |
| Files, processes, threads | Linux x86-64 base; standalone POSIX coverage is partial |
| Locale, wide chars, networking | Partial locale and Unicode support; resolver APIs remain |

**Hybrid builds** replace selected functions and use glibc or musl for the rest.
**Standalone builds** use Nelibc's runtime and syscall layer; they are incomplete.

## Track completeness

Symbol presence and behavioral coverage are tracked separately. The API audit
compares declarations, archive exports, and host delegation.

- [Function and symbol inventory](docs/musl-1.2.5-api-coverage.csv)
- [Coverage methodology](docs/api-coverage.md)
- [Prioritized core roadmap](docs/core-roadmap.md)
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
