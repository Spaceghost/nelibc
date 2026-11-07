# libc-test conformance runs

[musl's libc-test](https://git.musl-libc.org/cgit/libc-test) is a suite of
functional, regression, math and header (API) tests written against the C and
POSIX standards, not against any one libc. The API coverage manifest says
which symbols Nelibc exports; libc-test says which of them behave. It is run
against the hybrid archives, so a failure is either Nelibc's or the host
libc's; the glibc baseline below tells them apart.

## Running it

```sh
git clone https://git.musl-libc.org/git/libc-test ../libc-test   # or a mirror
make libc-test-gcc LIBC_TEST=../libc-test
make libc-test-zig LIBC_TEST=../libc-test
```

`tools/libc_test.sh` copies the checkout into `build/libc-test-<profile>/`,
builds every test with Nelibc's headers first on the include path and the
archive linked ahead of the host libc (the way a hybrid consumer links), runs
the suite, and compares the failing set with
`tools/libc_test_expected_<profile>.txt`. The target fails when a test outside
that list fails, and prints the tests in the list that now pass so the list can
shrink. A test that does not compile counts as a failure (`[build]`). Only
dynamic test binaries are built; static glibc plus a second archive defining
the same symbols is a different experiment and is not run.

The expected list is the list of known gaps, each one a conformance bug to
fix. Adding to it needs the same justification as any other documented
limitation.

## What the runs showed

Container `nelibc-writ` (Debian 12, glibc 2.36, GCC 12.2), libc-test
`63e36e9` (2026-11-01), as the unprivileged `build` user.

| Run | Date | Failing tests |
| --- | --- | --- |
| glibc 2.36 alone (baseline) | 2026-11-06 | 43 |
| Nelibc gcc hybrid, main `7c321ae`, archive linked, glibc headers | 2026-11-06 | 108 |
| Nelibc gcc hybrid, main `7c321ae`, archive linked, Nelibc headers | 2026-11-06 | no report: `strsignal` missing from `string.h` broke the test runner itself |
| Nelibc gcc hybrid, this slice, `make libc-test-gcc` | 2026-11-07 | 112 (`tools/libc_test_expected_gcc.txt`) |
| Nelibc zig hybrid, this slice, `make libc-test-zig` | 2026-11-07 | 118 (`tools/libc_test_expected_zig.txt`) |
| Nelibc gcc hybrid, slice 2, `make libc-test-gcc` | 2026-11-07 | 114 (`tools/libc_test_expected_gcc.txt`) |
| Nelibc zig hybrid, slice 2, `make libc-test-zig` | 2026-11-07 | 116 (`tools/libc_test_expected_zig.txt`) |
| Nelibc gcc hybrid, libm slice (`2db0322`, rebased on slice 2), `make libc-test-gcc` | 2026-11-07 | 38 (`tools/libc_test_expected_gcc.txt`) |
| Nelibc zig hybrid, libm slice (`2db0322`, rebased on slice 2), `make libc-test-zig` | 2026-11-07 | 42 (`tools/libc_test_expected_zig.txt`, 74 math tests removed) |
| Nelibc gcc hybrid, math details, `make libc-test-gcc` | 2026-11-07 | 31 (`tools/libc_test_expected_gcc.txt`, no math tests) |
| Nelibc zig hybrid, math details, `make libc-test-zig` | 2026-11-07 | 35 (`tools/libc_test_expected_zig.txt`, no math tests) |

Against the baseline, `7c321ae` fixed 8 tests glibc 2.36 fails (`strtol`,
`wcstol`, and the `fmin`/`fmax` families) and added 73 failures:

- 69 math tests. Almost all are floating-point exception flags, not values:
  `INEXACT`/`UNDERFLOW` missing or spurious on tiny inputs (`sin`, `atan`,
  `exp`, `expm1`), `INVALID` raised for `sqrt(NaN)`, missing `INVALID` for
  `ilogb(0)`/`ilogb(inf)`, missing `DIVBYZERO` for `pow(0, -y)`. `log`,
  `pow`, `lgamma` and `tgamma` also exceed 1 ulp on some inputs (1.5 to 2.2
  ulp observed). This is the P1 math-quality item in `core-roadmap.md`.
- `functional/inet_pton`: `inet_addr`/`inet_aton` rejected hex and octal
  parts, `inet_pton` accepted leading zeros.
- `functional/snprintf`: `%.1022f` returned -1 (precision was capped at 22),
  `%a` of zero printed `0x0p-1022`.
- `functional/strtod_simple`: a consequence of the `%.300f` cap above, not a
  `strtod` defect.
- `functional/strptime`: `%z` rejected `-06`.
- `regression/sigprocmask-internal`: `sigaddset` accepted signal 33.

With Nelibc's headers on the include path, the API tests found: `<poll.h>`
alone defined nothing (`sys/poll.h` tested `__GLIBC__` before `<features.h>`),
`<ctype.h>`, `<string.h>` and `<strings.h>` lacked `locale_t` and the `_l`
family under `_POSIX_C_SOURCE=200809L`, `<string.h>` lacked `strsignal`, and
`<inttypes.h>` used `wchar_t` without defining it.

This slice fixed every item above except the math flags and ulp errors, and
added `strsignal`, `strerror_l`, `strcasecmp_l`, `strncasecmp_l`, and (in the
standalone and freestanding profiles) `strcoll_l` and `strxfrm_l`; the hybrid
profiles leave the last two to the host, as their collation layer already
did. The 112 gcc failures that remain, now the expected list, are:

- 16 API header tests (compile-only). Each one fails against glibc 2.36's
  own headers with the same diagnostic: deprecated `readdir_r`, `gets`,
  `sighold` and friends under `-Werror`, missing `O_TTY_INIT`,
  `_POSIX_UIO_MAXIOV`, `NI_NUMERICSCOPE`, `setkey`,
  `_CS_POSIX_V7_THREADS_*`, `va_list` in `<wchar.h>`, and type mismatches in
  `<sys/sem.h>`, `<sys/shm.h>`, `<sys/socket.h>` and `<utmpx.h>`. In hybrid
  builds these headers are the host's or wrap it; `api/main` fails because
  its objects did not build.
- 10 functional tests, all of which glibc 2.36 also fails. Two exercised
  Nelibc's own code and were fixed in slice 2 (`strftime`, `fnmatch`, below).
  The scanf family and `swprintf` are the host's in the hybrid profiles: the
  hybrid stdio does not export them, so those four failures measure glibc.
  Nelibc's own `sscanf`/`fscanf`/`swprintf` (standalone profile) are checked
  against the same musl cases in `tools/syscall_smoke.c` instead; slice 2
  fixed `%8c` on short input there. `crypt`, `wordexp`, `clocale_mbfuncs`
  and `pthread_cancel-points` are the host's.
- 78 math tests: the exception-flag and ulp items above (the libm slice below
  reduced the math failures to seven, the math details
  follow-up to none).
- 8 regression tests, all of which glibc 2.36 also fails. One is
  Nelibc-specific in kind: `fpclassify-invalid-ld80` shows `%La` of an x87
  pseudo-denormal returning an error instead of formatting it.

The zig list differs in: four API tests clang rejects with
`-Wswitch-bool` (a compiler diagnostic, not a header gap); `math/sqrt` and
`math/sqrtl` pass with zig.

### Slice 2 (2026-11-07, branch `libc-test-slice-2`)

Fixed, now passing in both profiles: `functional/strftime` (POSIX
`%[flag][width]` on `%C %F %G %Y`, `-`/`_`/`0` padding flags, `+` on years past
9999 as POSIX's `%+4Y` reading of `%F` requires, and `%V` used a 0-based
ordinal day), `functional/fnmatch` (a backslash inside brackets is a member
and an unterminated `[` is literal, XCU 2.13.1 / XBD 9.3.5; glibc escapes
inside brackets, so a pattern like `[\]]` now reads differently from glibc),
`regression/fpclassify-invalid-ld80` (`%La` is formatted exactly from the
x87 bits, in musl's normalized form `0x1p+0`, not glibc's `0x8p-3`; a
pseudo-denormal normalizes and an unnormal prints `nan`, following the
FPU). Fixed on zig only because only that profile hit them:
`functional/strptime` and `functional/time` (the TZif reader kept over
400 KiB of arrays on the stack and clang inlined it into `localtime_r`;
musl's runner gives tests a 100 KiB stack; the file is now mapped and read
in place), `regression/getpwnam_r-crash` and `getpwnam_r-errno` (the
archive exported `malloc` but not `malloc_usable_size`, so glibc's NSS
module read a glibc chunk header off a Nelibc block; every profile that
exports `malloc` now exports `malloc_usable_size`).

Added to both expected lists: `math/erfcl`, `erfl`, `exp10l`, `modfl`,
`pow10l`. They failed before this slice too (checked by linking the tests
against the slice 1 archive): their reports print with `%La`, which returned
an error without a newline, so the runner's `FAIL` marker landed on the same
line as the truncated output and `grep '^FAIL'` missed it. The extraction
now matches the marker anywhere on the line. `modfl(-0)` returns `+0` for
the integral part; `exp10l(nan)` raises `INVALID`; `erfl`, `erfcl`,
`exp10l` are off by 2 to 11 ulp. These belong with the math work.

Since slice 2, `%Le`, `%Lf`, `%Lg` of a long double that is not a double are
formatted exactly from its x87 bits (see `docs/core-roadmap.md`, "Exact long
double decimal"); before, they returned -1 with `EINVAL`.

## The libm slice

The math failures were fixed by porting musl 1.2.5's algorithms into
`src/math_core.nelua` (attribution comments name each source file; no C math
code is linked). `tools/gen_musl_tables.py` regenerates the exp, pow, log and
log2 tables from a musl checkout, and encodes binary80 constants, which Nelua
would otherwise round through binary64.

- Exception flags: musl's `FORCE_EVAL` is the opaque asm identity
  `nelibc_fp_barrier_hw` (`f`/`l` variants) in `src/math_core_x86_64.S`;
  underflow, overflow, divide-by-zero and invalid results are produced by
  real operations instead of returned constants. `sqrt(NaN)`, `ilogb`,
  `logb`, `scalb`, tiny `sin`/`atan`/`exp`/`expm1`, subnormal float results,
  `atan2f`, `log1pf`, `roundl`, `expl`/`exp2l` (thresholds before the x87
  `fist` step), `atanhl(+-1)`, `powl(+-0, -y)` and the gamma poles raise what
  glibc raises.
- Values: `exp`, `exp2`, `exp10`, `expm1`, `pow`, `log`, `log2`, `log10`,
  `log1p`, `sinh`, `cosh`, `tanh`, `asin`, `acos`, `atan`, `atan2`, `tan`,
  `asinh`, `acosh`, `atanh`, `hypot`, `lgamma_r`, `tgamma`, `j0`, `j1`, `jn`,
  `y0`, `y1`, `yn`, their float variants, and `lgammal_r`/`tgammal` are
  musl ports.

Measured on `nelibc-writ` (gcc profile, libc-test `63e36e9`, unprivileged
`build` user, 2026-11-07). The per-function ULP rows are from the agent's run on the
slice before its rebase; the two libc-test rows are from commit `2db0322`, the
slice rebased onto slice 2 (logs `/work/artifacts/libm3-{gcc,zig}.log` in the container):

| Check | Result |
| --- | --- |
| `make libc-test-gcc` | 38 failing, all in the expected list; `math/*` failures are `fma`, `fmaf` and the five long double tests below |
| `make libc-test-zig` | 42 failing, all in the expected list; the same seven `math/*` tests; 74 math tests left the zig list |
| `make math-core-diff-gcc` | PASS: 116 functions matched on edge values and 100,000 generated inputs; max ULP exp/log 0/0 (float 1/1), sin/cos/tan 1/1/1 (float 1/1/1), pow 0 (float 1), erf/erfc 1/1 (float 1/2), cbrt 3 (float 1); hypot is gated at 4 ULP, not measured tighter |
| `make math-core-standalone-gcc` | PASS |
| libc-test math vectors | every counted vector (round-to-nearest, and the flag checks in all modes) within libc-test's 1.5 ulp tolerance; the directed-rounding ulp lines libc-test prints with `X` and does not count were not tallied |
| `lgammal`/`tgammal` sanity vectors | a probe (`printf` lacks `%La`, so the test's own value lines are unreadable) shows `lgammal` within 2 ulp and `tgammal` within 1 ulp of the binary80 expectations on the ten sanity vectors each, `lgammal(-2.5)` at 0.96 ulp |

Per-family counts of failing `math/*` tests on the way, each from a
`tools/libc_test.sh gcc` run on the streamed tree (commit messages carry the
run tags): 90 before the slice; 45 after exp/pow/trig and the flag discipline;
28 after lgamma/tgamma/Bessel; 6 after log/hypot and the long double flag
fixes; 2 after the ld80 gamma ports.

The libm slice left seven math tests failing in both profiles; the math
details follow-up (below) fixed all seven.

### Math details (2026-11-07, branch `math-details`)

- `fma(x, y, 0)` returned `x*y + 0`, which turns an underflowed `-0` into
  `+0` (`fma(-0x1p-1000, 0x1p-100, 0)` must be `-0`). With a zero addend the
  exact result is the product, so it now returns `x*y`. musl has the same
  shortcut.
- `fmaf` missed `UNDERFLOW` when the double sum was inexact but converted
  to a float subnormal exactly (`fmaf(0x1p-120, 0x1p-120, 0x1p-149)`), and
  double-rounded when the result was a float subnormal: the halfway test
  only knew normal floats. It now finds a float-grid midpoint by comparing
  with the neighbouring float, takes the exact error of the double sum
  (TwoSum) to pick the side, reads the SSE rounding mode (directed modes need
  no correction), and raises `UNDERFLOW` for tiny inexact results.
- `modfl` returned `+0` as the fraction of a negative integer or `-inf`; the
  fraction now carries the sign of the argument.
- `erfl` and `erfcl` are ports of musl's ld80 code (Moshier's coefficients,
  every one spelled as its binary80 bit pattern); the series and continued
  fraction they replace were 2 to 11 ulp off.
- `exp10l` (and `pow10l`) follow musl: integer powers from an exact table,
  other small arguments through `exp2l`, the range test on exponent bits so
  a NaN raises no `INVALID`. The split-constant path for results near the
  subnormal boundary is kept.

Result: gcc 31 and zig 35 expected failures, no `math/*` test among them.
Gate: the 155-target list, container `nelibc-writ`, Nelua `dabbd9ef`, on
`58f9caa`: 155/155 exited 0, no warnings; logs in
`/work/artifacts/gate-md-20261107-1024/`.
The zig
list differs from the gcc list only in four API tests clang rejects at build
time (`ftw`, `iso646`, `sys_stat`, `sys_wait`).

## Not run

- The static musl hybrid and the standalone profile: libc-test's build
  expects a hosted toolchain; a standalone harness is future work.
- Static test binaries (see above).
