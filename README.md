# nelibc

**A C-compatible libc, incrementally reimplemented in [Nelua](https://nelua.io).**

Nelibc starts alongside an existing libc and replaces functions one at a time.
Use the Nelua implementation where it is ready; let glibc or musl provide the
rest. The long-term target is a libc that can stand on its own.

> **Status:** experimental; Linux x86-64. Hybrid builds are useful now.
> Standalone builds work for a growing slice and are not a complete general
> purpose libc yet.

## Try it

Requirements: [Nelua](https://nelua.io) and [Zig](https://ziglang.org/) (`zig cc`).

```sh
git clone https://github.com/Spaceghost/nelibc.git
cd nelibc
make dogfood-zig
```

This builds the hybrid archive, links a real C program, runs it, and checks
that Nelibc's symbols are present. To link your own program:

```sh
make lib-zig
zig cc -Iinclude -fno-builtin your_program.c build/libnelibc-zig.a -o your_program
./your_program
```

`-fno-builtin` keeps compiler builtins from bypassing the symbols being tested.
The [quickstart](docs/quickstart.md) covers GCC, musl, and TinyCC too.

## How replacement works

| Build | Nelibc provides | Existing libc provides |
|---|---|---|
| Hybrid (default) | Functions Nelibc exports | Uncovered APIs, locale data, and host runtime services |
| Standalone | Its implemented libc and Linux syscall layer | Nothing at runtime; this profile is still incomplete |

Each function keeps the C ABI. This lets ordinary C programs use Nelibc
incrementally, while standalone builds reveal which runtime pieces still need
implementation.

## Progress

The pinned musl 1.2.5 symbol audit tracks **1,471 public functions**:

| Status | Functions | What it means |
|---|---:|---|
| Nelibc implementation | 803 | Implemented in Nelua in at least one hybrid profile |
| Host delegated | 223 | Available through glibc or musl in hybrid builds |
| Not declared yet | 445 | Remaining API-surface candidates |

These are symbol counts, not behavioral completeness scores. Each function
still needs ABI, edge-case, and cross-libc validation. See the
[full function audit](docs/musl-1.2.5-api-coverage.csv),
[coverage notes](docs/api-coverage.md), and
[ranked core roadmap](docs/core-roadmap.md).

| Area | Current state |
|---|---|
| Memory, strings, and conversions | Broad Nelua implementation; differential-tested across compiler/libc profiles |
| Math | Broad real and complex math coverage; numerical and floating-environment gaps remain |
| Stdio | Hybrid wrappers and a growing standalone stream/formatting implementation |
| Processes, files, and threads | Linux x86-64 syscall base; standalone POSIX behavior is still a subset |
| Locale, wide characters, and networking | Partial locale/Unicode behavior; resolver and network database APIs remain major gaps |

## Build profiles and checks

Zig cc is the default. The project also compares GCC and TinyCC, and tests
glibc and musl hybrid builds.

```sh
make dogfood-matrix       # Zig/glibc, GCC/glibc, Zig/musl, TinyCC/glibc
make smoke-matrix         # available build, ABI, and differential checks
make api-coverage         # regenerate build/api-coverage.csv
```

TinyCC is optional; pass `TCC` and `TCC_LIBTCC1` when it is installed outside
the default paths. See the [build and implementation reference](docs/implementation-catalog.md)
for detailed targets, individual API notes, benchmark records, and test scope.

## Contributing

Start with a small API slice: implement it in Nelua, expose the stable C ABI,
add a C consumer check, then compare behavior across the available host
profiles. Keep host-delegated behavior explicit and update the coverage tracker
when declarations or exports change.

## License

MIT. See [THIRD_PARTY_NOTICES.md](THIRD_PARTY_NOTICES.md) for third-party
notices covering the fdlibm-derived math code.
