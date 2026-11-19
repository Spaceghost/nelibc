<div align="center">

# nelibc

### A C library, rebuilt piece by piece in [Nelua](https://nelua.io)

Replace selected libc functions with Nelua implementations. Resolve the rest from
Cosmopolitan libc in the default build, without a GNU libc dependency.
Glibc and musl remain available as comparison profiles.

[Quickstart](docs/quickstart.md) ·
[API coverage](docs/musl-1.2.5-api-coverage.csv) ·
[Roadmap](docs/core-roadmap.md) ·
[Contributing](#contributing)

**Experimental** · **Linux x86-64** · **Hybrid builds working** ·
**Standalone in progress**

</div>

## Try it

Install [Nelua](https://nelua.io) and the
[Cosmopolitan toolchain](docs/cosmopolitan.md#build), then run:

```sh
git clone https://github.com/Spaceghost/nelibc.git
cd nelibc
make dogfood-cosmo
```

This builds the library, links a C program against it, runs the program, and
checks that Nelibc's symbols made it into the executable.

To link your own C program:

```sh
make lib-cosmo
x86_64-unknown-cosmo-cc -fno-builtin app.c build/libnelibc-cosmo.a -o app
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
own gaps. The [host profile](docs/host-profile.md) is standalone without
Linux: the same objects over a table of host callbacks (console, memory,
clock, exit) for kernels and other freestanding environments, with
threads as cooperative lanes on the one hardware thread, checked by `make host-matrix` and by running the unmodified
[Bend](https://github.com/bendlang/bend) 2 runtime that
[Writ](https://github.com/Spaceghost/writ) compiles to. The [musl 1.2.5 API inventory](docs/musl-1.2.5-api-coverage.csv)
tracks 1,471 function symbols. In the pinned hybrid comparison:

| Coverage status | Symbols | Share |
| --- | ---: | ---: |
| Exported by Nelibc in at least one hybrid profile | 813 | 55% |
| Delegated to the selected host libc | 223 | 15% |
| Not yet declared by Nelibc | 435 | 30% |

These counts track exported names, not behavioral conformance. See the
[coverage notes](docs/api-coverage.md) for methodology and per-profile gaps.

## Build profiles

The default `make` target runs the Cosmopolitan hybrid smoke checks.
[Cosmopolitan profile](docs/cosmopolitan.md) documents symbol ownership and
ABI requirements. Zig, GCC, and TinyCC remain comparison profiles for
glibc, musl, standalone, and host builds.

```sh
make dogfood-matrix
make smoke-matrix
```

TinyCC is optional. See the [quickstart](docs/quickstart.md) for requirements
and profile-specific commands. The 2026-11-17 integration compiler is pinned
with an x87 argument fix; see [the compiler patch and reproducer](tools/toolchains/README.md).

## Build Nelua's Lua runtime against standalone Nelibc

With a built x86-64 standalone Zig archive and a Spaceghost Nelua source
checkout, build the bundled Lua interpreter and its builtin Nelua modules:

```sh
tools/build-nelua-runtime.sh "$ZIG" "$ZIG_LIB_DIR" "$NELUA_SRC" . \
  build/libnelibc-standalone-zig.a "$NELUA_SRC/nelua-lua"
"$NELUA_SRC/nelua-lua" -e 'print(_VERSION)'
"$NELUA_SRC/nelua" --version
```

`ZIG_LIB_DIR` is the Zig distribution's `lib` directory. The script uses
its compiler and Linux UAPI headers and links the interpreter directly
against the standalone Nelibc archive. The runtime enables Lua's POSIX
facilities and statically registered modules. Loading shared libraries
through Lua is unavailable. Nelua still needs its `lib` and `lualib`
trees alongside the interpreter and a C compiler to build native programs.

## Project guide

| Guide | What it covers |
| --- | --- |
| [Quickstart](docs/quickstart.md) | Build and link a C program |
| [Full integration gate](docs/integration-gate-20261117.md) | 2026-11-17 Pacific: 14 targets passed; expected failures and namespace skip recorded |
| [API inventory](docs/musl-1.2.5-api-coverage.csv) | Function and symbol coverage |
| [Coverage notes](docs/api-coverage.md) | How coverage is measured |
| [Core roadmap](docs/core-roadmap.md) | Priorities and missing core pieces |
| [libc-test](docs/libc-test.md) | musl libc-test as a conformance gate for the hybrid and standalone archives; what fails and why |
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
