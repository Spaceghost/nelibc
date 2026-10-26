# nelibc

### A libc rebuilt piece by piece in [Nelua](https://nelua.io)

Nelibc lets C programs use Nelua implementations for selected libc functions.
The host libc fills the gaps while the project grows toward a standalone
runtime.

> **Status:** experimental · Linux x86-64 · hybrid builds are usable ·
> standalone runtime is incomplete

## Get started

Requirements: [Nelua](https://nelua.io) and [Zig](https://ziglang.org/).

```sh
git clone https://github.com/Spaceghost/nelibc.git
cd nelibc
make dogfood-zig
```

This builds Nelibc, links and runs a C consumer, and checks that Nelibc's
symbols are present in the executable.

Link your own program:

```sh
make lib-zig
zig cc -Iinclude -fno-builtin app.c build/libnelibc-zig.a -o app
```

`-fno-builtin` makes calls go through the library. See the
[quickstart](docs/quickstart.md) for GCC, musl, and TinyCC instructions.

## What works today

| Area | Status |
| --- | --- |
| Memory, strings, conversions | Broad implementation; differential checks across profiles |
| Math | Broad real and complex coverage; numerical edge cases remain |
| Stdio | Hybrid support plus a growing standalone implementation |
| Files, processes, threads | Linux x86-64 base; standalone POSIX coverage is partial |
| Locale, wide chars, networking | Partial locale and Unicode support; resolver APIs remain |

Hybrid builds replace selected functions and use glibc or musl for the rest.
Standalone builds use Nelibc's runtime and syscall layer; they are still under
development.

## Completeness

The API tracker compares declarations, archive exports, and host delegation
across compiler and libc profiles. Symbol presence is tracked separately from
behavioral completeness.

- [Function and symbol coverage](docs/musl-1.2.5-api-coverage.csv)
- [Coverage methodology](docs/api-coverage.md)
- [Prioritized core roadmap](docs/core-roadmap.md)
- [Implementation and test catalog](docs/implementation-catalog.md)

Refresh the local report with `make api-coverage`.

## Build and compare

Zig cc is the default toolchain. GCC and TinyCC provide comparison builds;
hybrid profiles cover glibc and musl.

```sh
make dogfood-matrix
make smoke-matrix
```

TinyCC is optional. See the [quickstart](docs/quickstart.md) for setup and
individual commands.

## Contributing

Choose a small API slice, implement it in Nelua, expose its C ABI, and add a C
consumer check. Record host delegation in the coverage tracker.

## License

MIT. See [third-party notices](THIRD_PARTY_NOTICES.md) for the fdlibm-derived
math code.
