# Nelibc quickstart

Nelibc replaces selected C library functions with implementations compiled
from Nelua. Hybrid archives link against the selected host libc, so uncovered
functions continue to come from glibc or musl. This is an experimental Linux
x86-64 project; the standalone runtime is a separate, incomplete profile.

## Requirements

- Nelua compiler (`nelua`)
- Zig (`zig cc`) for the default build and final linking
- GCC for the GCC comparison profile
- TinyCC and its `libtcc1.a` for the TinyCC profile

## Build and run a real C consumer

From the repository root:

```sh
make dogfood-zig
```

This builds `build/libnelibc-zig.a`, compiles
[`examples/incremental.c`](../examples/incremental.c) with `zig cc`, runs it,
and checks that the executable contains Nelibc's `memcpy`, `strlen`, `strchr`,
and `strtol` symbols. Expected output:

```text
Nelua allocator + memory/string/strtol/math: nelua-libc:42 -> 42 (13 bytes, sqrt=9 cbrt=3 cbrtf=-0.5 fmod=-1 remainder=-1 min=2 max=5 fdim=5 fdimf=5 modf=9.0+0.5 frexp=0.5*2^4 ldexp=12 scalbnf=-12 scalbln=12 rint=2 rintf=2 nearbyint=-2 nearbyintf=-2 exp=2.71828183 expf=2.71828175 expm1=0.648721271 log=0.693147181 logf=0.693147182 log1p=0.405465108 log2=3 log10=2 exp10=100 pow=1024 powf=1024 hypot=5 hypotf=5 sin=0.841470985 cos=0.540302306 sinf=0.841470957 cosf=0.540302277 tan=1.55740772 tanf=1.55740774 sinh=1.17520119 sinhf=1.17520118 cosh=1.54308063 coshf=1.54308069 tanh=0.761594156 tanhf=0.761594176 asinh=0.881373587 asinhf=0.881373584 acosh=1.3169579 acoshf=1.31695795 atanh=0.549306144 atanhf=0.549306154 atan=0.785398163 atanf=0.785398185 atan2=2.35619449 atan2f=2.3561945 nextafter=1.0000000000000002 nexttoward=1.0000000000000002 nexttowardf=1.00000012 ilogb=3 logb=3)
```

The example allocates through `nelibc_malloc`, uses sixty-five standard
functions plus GNU's `exp10` extension from the archive, and prints through
the host's `printf`. This is the incremental replacement model: each archive
contributes the symbols it owns; the host runtime supplies the rest.

To link your own program against the default glibc hybrid archive:

```sh
make lib-zig
zig cc -Iinclude -fno-builtin your_program.c build/libnelibc-zig.a -o your_program
./your_program
```

Use `-fno-builtin` so compiler-generated builtin calls do not bypass the
library symbols being evaluated. `nelibc_malloc` and related prefixed calls
are declared in `include/nelibc_alloc.h` and avoid changing the host allocator
for unrelated host-owned objects.

The current Nelua math slice exports binary32 and binary64 forms of `fabs`,
`copysign`, `trunc`, `floor`, `ceil`, `round`, `rint`, `nearbyint`, `exp`,
`log`, `log1p`, `log2`, `log10`, `exp2`, `exp10`, `expm1`, `sin`, `cos`, `tan`, `sinh`, `cosh`, `tanh`, `hypot`, `sqrt`, `fmod`,
`remainder`, `fmin`, `fmax`, `fdim`, `cbrt`, `frexp`, `modf`, `ldexp`, `scalbn`,
`scalbln`, `pow`, `atan`, `atan2`, `asinh`, `acosh`, `atanh`, `ilogb`,
`logb`, `nextafter`, and `nexttoward`.
`rint` observes the active rounding direction and may raise `FE_INEXACT`;
`nearbyint` preserves the direction and suppresses that exception. In
standalone builds, `include/math.h` also provides basic floating-point
classification macros. Run
`make math-core-diff-matrix` to compare the slice against host libm on four
hybrid profiles, or `make math-core-standalone-matrix` to run it in static
`-nostdlib` binaries built with Zig, GCC, and TinyCC.
The inverse hyperbolic functions pass edge and 100,000 generated input checks
within eight ULP on all four hybrid profiles; their domain and pole `errno`
behavior is checked as well.
The current `exp`/`log` differential sample stays within 1 ULP for `exp` and
2 ULP for `log` in double precision, and within 1 ULP for both float forms.
These are observed bounds from the current test set, not a correctly-rounded
or all-input guarantee.
The shared trigonometric reducer handles the full binary64 exponent range; the
current differential sample observed at most 1 ULP for `sin`/`cos` in all four
float/double forms, including very large finite inputs.
The math instruction backend targets x86-64, matching the current Linux
platform. Negative `sqrt` and invalid `fmod`/`remainder` domains set `EDOM`;
`exp` overflow/underflow, `log` domain/pole, and infinite `sin`/`cos` inputs
set the expected `errno`;
`ldexp`/`scalbn` overflow and underflow to zero set `ERANGE`. Floating-point
exception reporting and most of the C math API remain incomplete.

## Compare supported build profiles

```sh
make dogfood-gcc
make dogfood-zig-musl
make dogfood-tcc TCC=/path/to/tcc TCC_LIBTCC1=/path/to/libtcc1.a
```

`dogfood-zig-musl` is a static x86-64 musl executable. The project supplies its
stdio and allocator linker-wrap flags. TinyCC compiles the consumer source;
Zig cc performs the final link and adds TinyCC's runtime archive. Run all four
profiles with `make dogfood-matrix` and the same `TCC` / `TCC_LIBTCC1` values.
The default `make dogfood-zig` needs only Nelua and Zig; GCC and TinyCC are only
needed for their comparison profiles. `TCC` can be a wrapper command, which is
useful when TinyCC runs in a container, but `TCC_LIBTCC1` must name an archive
that the host Zig linker can read.

The four-profile dogfood matrix passed on 2026-10-23 with Zig and GCC on glibc,
Zig on musl, and TinyCC on glibc. The sample program checks that selected
Nelibc symbols are linked into each executable and exercises allocation,
memory, strings, integer conversion, and the current Nelua math slice.

## Useful checks

- `make smoke-matrix` exercises the compiler, libc, syscall, standalone, and
  differential-test matrix.
- `make lib-consumer-matrix` runs a larger C program against the four hybrid
  archives; this covers more APIs than the small quickstart example.
- `make sem-clockwait-musl-smoke` compiles and runs a static musl consumer of
  `sem_clockwait`, including deadline, wakeup, and cancellation behavior.
- `make api-coverage` refreshes `build/api-coverage.csv`. It tracks declared
  APIs and archive exports; a listed export is not proof of full conformance.
- `make shootout-matrix` runs performance comparisons. Treat these as local
  microbenchmarks and compare runs on the same machine.

See [`core-roadmap.md`](core-roadmap.md) for known gaps and priorities.
