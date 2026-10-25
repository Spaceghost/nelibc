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

Zig builds default to `-mcpu=x86_64`, so binaries run on baseline x86-64
machines. Override this with `make ZIGFLAGS='-mcpu=x86_64_v3'` when you
intentionally want a newer CPU baseline.

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

The example allocates through `nelibc_malloc` and uses standard memory, string,
integer conversion, and math functions, including long-double `hypotl`, `nextafterl`,
`atan2l`, `asinl`, `logl`, `log1pl`, `expl`, `exp2l`, `expm1l`, `sinl`, `cosl`,
`tanl`, `tanhl`, `sinhl`, `coshl`, `asinhl`, `acoshl`, `atanhl`, `powl`,
`fminl`, `fmaxl`, `fdiml`, and `cbrtl`, plus
long-double scaling and rounding. It also uses GNU's
`exp10` extension from the archive, and prints through the host's `printf`.
This is the incremental replacement model: each archive
contributes the symbols it owns; the host runtime supplies the rest.

The archive also exports the six `<inttypes.h>` helpers `imaxabs`, `imaxdiv`,
`strtoimax`, `strtoumax`, `wcstoimax`, and `wcstoumax`, plus locale-specific
`strtof_l`, `strtod_l`, and `strtold_l` conversions. GNU builds also provide
`strtol_l`, `strtoul_l`, `strtoll_l`, `strtoull_l`, and their wide-character
counterparts. GNU builds also export the narrow byte classification and case
conversion `_l` functions (`isalnum_l` through `toupper_l`). In the current
locale data, byte results are shared by `C` and `C.UTF-8`; locale-sensitive
wide-character operations use the selected locale's Unicode data in standalone
mode. Standalone archives also provide `wcscoll`, `wcscoll_l`, `wcsxfrm`, and
`wcsxfrm_l` with code-point ordering in their `C` and `C.UTF-8` collation
modes; hybrid links use the host libc implementations.

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

### Use POSIX message queues

The archive includes `mqueue.h` functions for opening, sending, receiving,
configuring, and removing POSIX message queues. This small consumer links them
from the Nelibc archive:

```c
#define _GNU_SOURCE
#include <fcntl.h>
#include <mqueue.h>
#include <stdio.h>
#include <unistd.h>

int main(void) {
  char name[64];
  snprintf(name, sizeof name, "/nelibc-example-%ld", (long)getpid());
  struct mq_attr attr = { .mq_maxmsg = 4, .mq_msgsize = 128 };
  mqd_t queue = mq_open(name, O_CREAT | O_EXCL | O_RDWR, 0600, &attr);
  if (queue == (mqd_t)-1) return 1;

  const char message[] = "hello";
  if (mq_send(queue, message, sizeof message, 0) != 0) return 2;
  char buffer[128];
  ssize_t size = mq_receive(queue, buffer, sizeof buffer, NULL);
  if (size < 0) return 3;
  printf("received %.*s\n", (int)size, buffer);

  if (mq_close(queue) != 0) return 4;
  return mq_unlink(name) != 0;
}
```

Save it as `mq_example.c`, then run:

```sh
make lib-zig
zig cc -Iinclude -fno-builtin -pthread mq_example.c build/libnelibc-zig.a -o mq_example
./mq_example
```

The smoke test also covers timed waits, cancellation, queue attributes, and
`SIGEV_NONE`, `SIGEV_SIGNAL`, and `SIGEV_THREAD` notifications. Thread
notifications run the requested callback with the supplied `sigval` after a
message arrives.

The archive also provides the legacy aligned-allocation API through
`<malloc.h>`. For example, `memalign(128, 257)` returns storage aligned to
128 bytes; release it with `free`. Alignments must be powers of two. Values
smaller than pointer alignment are rounded up:

```c
#include <malloc.h>
#include <stdlib.h>

void *buffer = memalign(128, 257);
if (buffer == NULL) {
  /* handle allocation failure */
}
free(buffer);
```

### Try an existing application

For a larger check, the Spinel C compiler can itself link against Nelibc, then
compile a generated Ruby client against the same hybrid archive. This requires
a local Spinel checkout with its Prism dependency available:

```sh
NELIBC_DIR=/path/to/nelibc
SPINEL_DIR=/path/to/spinel
make -C "$NELIBC_DIR" lib-zig
NO_CCACHE=1 make -B -C "$SPINEL_DIR" bin/spinel CC='zig cc' \
  LDFLAGS="-Wl,-u,memcpy $NELIBC_DIR/build/libnelibc-zig.a -lcrypt"
"$SPINEL_DIR/bin/spinel" --cc='zig cc -Wl,-u,memcpy' \
  --link "$NELIBC_DIR/build/libnelibc-zig.a" \
  -e 'values = [3, 1, 4, 1, 5]; puts values.join(","); puts values.sort.join(",")' \
  -o /tmp/nelibc-spinel-client
/tmp/nelibc-spinel-client
nm --defined-only /tmp/nelibc-spinel-client | grep -E ' (memcpy|strlen|strtol)$'
```

Expected program output is `3,1,4,1,5` followed by `1,1,3,4,5`. The executable
retains Nelibc's `memcpy`, `strlen`, and `strtol`; glibc supplies the rest.
This is hybrid dogfooding, not a standalone-libc test.

Run the same generated client as a static musl executable with:

```sh
make dogfood-spinel-musl SPINEL_DIR=/path/to/spinel-checkout
```

This rebuilds Spinel's compiler with the glibc hybrid archive, builds a musl
version of its C runtime in a temporary directory, and links the generated
client against Nelibc's Zig/musl archive. The temporary runtime supplies
musl-compatible replacements for Spinel's optional backtrace hooks. It refreshes
Spinel's normal build outputs but does not change its tracked source files.

Compile the generated client with TinyCC and link it with Nelibc's TinyCC
archive using Zig cc:

```sh
make dogfood-spinel-tcc \
  SPINEL_DIR=/path/to/spinel-checkout \
  TCC=/path/to/tcc \
  TCC_LIBTCC1=/path/to/libtcc1.a
```

TinyCC compiles the generated C and Zig cc links its object with the TinyCC
runtime archive, Spinel runtime, and Nelibc hybrid archive. The target checks
that the executable runs and contains Nelibc's `memcpy`, `strlen`, and `strtol`.

Run both the static-musl and TinyCC application checks sequentially with:

```sh
make dogfood-spinel-matrix \
  SPINEL_DIR=/path/to/spinel-checkout \
  TCC=/path/to/tcc \
  TCC_LIBTCC1=/path/to/libtcc1.a
```

The current Nelua math slice exports binary32 and binary64 forms of `fabs`,
`copysign`, `trunc`, `floor`, `ceil`, `round`, `rint`, `finite`, `finitef`, `nan`, `nanf`, `lrint`, `lrintf`, `llrint`, `llrintf`, `lround`, `lroundf`, `llround`, `llroundf`, `nearbyint`, `exp`,
`log`, `log1p`, `log2`, `log10`, `exp2`, `exp10`, `pow10`, `pow10f`, `expm1`, `sin`, `cos`, `tan`, `sinh`, `cosh`, `tanh`, `hypot`, `sqrt`, `fmod`,
`remainder`, `drem`, `dremf`, `remquo`, `remquof`, `fmin`, `fmax`, `fdim`, `fma`, `fmaf`, `erf`, `erff`, `erfc`, `erfcf`, `tgamma`, `tgammaf`, `lgamma`, `lgammaf`, `lgamma_r`, `lgammaf_r`, `jn`, `jnf`, `yn`, `ynf`, `j0`, `j0f`, `j1`, `j1f`, `sincos`, `sincosf`, `y0`, `y0f`, `y1`, `y1f`, `cbrt`, `frexp`, `modf`, `ldexp`, `scalbn`,
`scalbln`, `pow`, `atan`, `atan2`, `asinh`, `acosh`, `atanh`, `ilogb`,
`logb`, `nextafter`, and `nexttoward`.
On x86-64, `nanl`, `fabsl`, `copysignl`, `fmal`, `sqrtl`, `fmodl`, `remainderl`, `frexpl`,
`modfl`, `hypotl`, `nextafterl`, `atanl`, `atan2l`, `asinl`, `acosl`, `logl`,
`log2l`, `log10l`, `log1pl`, `expl`, `exp2l`, `expm1l`, `sinl`, `cosl`,
`sincosl`, `tanl`, `tanhl`, `sinhl`, `coshl`, `asinhl`, `acoshl`, `atanhl`,
`powl`, `exp10l`, `pow10l`, `fminl`, `fmaxl`, `fdiml`, `cbrtl`,
`ilogbl`, `logbl`, `ldexpl`,
`scalbnl`, `scalblnl`, `truncl`,
`floorl`, `ceill`, `rintl`, `nearbyintl`, `roundl`, `lrintl`, `llrintl`,
`lroundl`, and `llroundl`
operate directly on the extended precision
`long double` representation; long-double error functions and gamma
functions remain incomplete.
`rint` and `rintl` observe the active rounding direction and may raise
`FE_INEXACT`; `nearbyint` and `nearbyintl` preserve the direction and suppress
newly raised inexact exceptions. `roundl` rounds halfway cases away from zero;
`lrintl` and `llrintl` use the active rounding direction for integer
conversion; `lroundl` and `llroundl` round halfway cases away from zero
without raising a new inexact exception. In
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
set the expected `errno`; `ldexp`/`scalbn` overflow and underflow to zero set
`ERANGE`. Standalone builds provide all standard `fenv.h` functions and
synchronize x87 and SSE exception flags and rounding modes. `FE_NOMASK_ENV`
also supports the GNU all-traps-enabled environment; smoke tests verify
`SIGFPE` delivery for each standard exception through x87 and SSE. Hybrid builds
keep the host `fenv_t` ABI and use the host fenv implementation. Less-common
environment edge cases and most of the C math API remain incomplete. Pass
`fesetenv` only a snapshot from `fegetenv`/`feholdexcept` or a defined macro
such as `FE_DFL_ENV`; `fenv_t` is opaque and arbitrary edited bytes are invalid.

`cabs*`, `creal*`, `cimag*`, `conj*`, `cproj*`, `carg*`, `csqrt*`, `cexp*`,
`clog*`, `csin*`, `ccos*`, `csinh*`, `ccosh*`, `ctan*`, `ctanh*`, `catan*`,
`catanh*`, `casin*`, `cacos*`, `casinh*`, `cacosh*`, and `cpow*` are the first C
complex-math replacements. The complex argument and
return ABI is handled in
`src/complex_abi.c`. Nelua computes magnitudes with `hypot*`, conjugates the
imaginary components, applies the projection rule for infinities, and computes
arguments with `atan2*`. Scaled formulas handle square roots, logarithms,
inverse sine and cosine, and large complex tangent inputs. Complex exponentials
and circular and hyperbolic trigonometric functions compose the real Nelibc
kernels. The inverse hyperbolic functions reuse the inverse sine and cosine
kernels through principal-branch identities; complex power composes the complex
logarithm and exponential kernels. `catan*` uses scaled quadratic expressions,
and `catanh*` reuses it through `atanh(z) = -i atan(i z)`. Run
`make complex-math-smoke-matrix` to exercise all three precisions in Zig/glibc,
GCC/glibc, Zig/musl, TinyCC-built Nelua archives, and standalone archives. The
TinyCC profile uses Zig cc for the caller and ABI adapter because TinyCC does
not parse C `_Complex`; the math implementation itself is still built by
TinyCC.

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
Zig on musl, and TinyCC on glibc. The TinyCC dogfood target was also rerun on
2026-10-24 with TinyCC 0.9.28rc (upstream snapshot `e23dabe`); the same build
also passed TinyCC math differential, standalone fenv trap, memory/stdio, and
syscall smoke targets. The sample program checks that selected Nelibc symbols
are linked into each executable and exercises allocation, memory, strings,
integer conversion, and the current Nelua math slice.

## Useful checks

- `make smoke-matrix` exercises the compiler, libc, syscall, standalone, and
  differential-test matrix.
- `make lib-consumer-matrix` runs a larger C program against the four hybrid
  archives, including `sem_clockwait`; this covers more APIs than the small
  quickstart example.
- `make qsort-r-smoke-matrix` dogfoods GNU `qsort_r` under Zig/glibc,
  GCC/glibc, Zig/musl, and TinyCC/glibc.
- `make pthread-standalone-matrix` exercises the standalone pthread subset with
  Zig, GCC, and TinyCC, including process-shared barriers and read-write locks.
  Its join checks include `pthread_tryjoin_np` returning `EBUSY` for a live
  thread, and `pthread_timedjoin_np` timing out without consuming the join.
  Condition-variable tests cover `pthread_cond_clockwait` with monotonic and
  realtime clocks, invalid clocks, and null deadlines.
  Read-write lock tests cover `pthread_rwlock_clockrdlock` and
  `pthread_rwlock_clockwrlock` using monotonic deadlines and invalid inputs.
  Thread-name checks cover self and remote names, the 15-byte name limit, and
  the minimum 16-byte get buffer.
  Thread-affinity checks read and reapply masks for the calling thread and a
  worker, and check the invalid zero-size case.
  `pthread_getattr_np` checks the main, Nelibc-managed, and caller supplied
  thread stack bounds and guard sizes.
  Default pthread attributes can be inspected or changed with
  `pthread_getattr_default_np` and `pthread_setattr_default_np`; the smoke test
  verifies that new threads use the updated stack and guard sizes.
- `make sem-clockwait-musl-smoke` compiles and runs a static musl consumer of
  `sem_clockwait`, including deadline, wakeup, and cancellation behavior.
- `make sem-clockwait-shootout-zig` compares Nelibc's immediate, empty-timeout,
  and blocked-wait paths against glibc and writes CSV output under `build/`.
- Link glibc applications using the hybrid `sem_clockwait` wrapper with
  `-pthread -ldl`.
- `make api-coverage` refreshes `build/api-coverage.csv`. It tracks declared
  APIs and archive exports; a listed export is not proof of full conformance.
- `make shootout-matrix` runs performance comparisons. Treat these as local
  microbenchmarks and compare runs on the same machine.

See [`core-roadmap.md`](core-roadmap.md) for known gaps and priorities.
