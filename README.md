# nelibc

### A libc, rebuilt piece by piece in [Nelua](https://nelua.io).

Nelibc lets C programs use Nelua implementations for the functions that are
ready, while glibc or musl supplies the rest. The goal is a libc that can
eventually run on its own.

**Status:** experimental · Linux x86-64 · hybrid use works today · standalone
runtime is incomplete

---

## Try it

Install [Nelua](https://nelua.io) and [Zig](https://ziglang.org/), then:

```sh
git clone https://github.com/Spaceghost/nelibc.git
cd nelibc
make dogfood-zig
```

That builds Nelibc, links a C program against it, runs the program, and checks
that Nelibc's symbols made it into the executable.

To link your own C program:

```sh
make lib-zig
zig cc -Iinclude -fno-builtin app.c build/libnelibc-zig.a -o app
```

`-fno-builtin` makes sure the compiler calls the library functions being
tested. See the [quickstart](docs/quickstart.md) for GCC, musl, and TinyCC.

## What works

| Area | Snapshot |
| --- | --- |
| Memory, strings, conversions | Broad implementation; differential checks across build profiles |
| Math | Wide real and complex coverage; numerical edge cases remain |
| Stdio | Hybrid stdio plus a growing standalone implementation |
| Files, processes, threads | Linux x86-64 base; standalone POSIX coverage is partial |
| Locale, wide chars, networking | Partial locale and Unicode support; resolver APIs remain |

Hybrid builds can replace selected functions and leave the rest to the host
libc. Standalone builds use Nelibc's own runtime and syscall layer, and are
still under development.

## Track completeness

The API tracker compares declarations, archive exports, and host delegation
across compiler and libc profiles. It is the source of truth for symbol-level
progress; symbol counts do not measure behavioral completeness.

- [Function and symbol audit](docs/musl-1.2.5-api-coverage.csv)
- [How coverage is measured](docs/api-coverage.md)
- [Prioritized core roadmap](docs/core-roadmap.md)
- [Builds, implementations, and test catalog](docs/implementation-catalog.md)

Refresh the local report with `make api-coverage`.

## Build profiles

Zig cc is the default. GCC and TinyCC are used for comparison; hybrid builds
cover glibc and musl.

```sh
make dogfood-matrix
make smoke-matrix
```

TinyCC is optional. See the [quickstart](docs/quickstart.md) for setup and
individual build commands.

## Contributing

Pick a small API slice, implement it in Nelua, expose its C ABI, and add a C
consumer check. Keep host delegation explicit and update the coverage tracker.

## License

MIT. See [third-party notices](THIRD_PARTY_NOTICES.md) for the fdlibm-derived
math code.
