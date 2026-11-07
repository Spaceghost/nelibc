# libc-test conformance runs

[musl's libc-test](https://git.musl-libc.org/cgit/libc-test) is a suite of
functional, regression, math and header (API) tests written against the C and
POSIX standards, not against any one libc. The API coverage manifest says
which symbols Nelibc exports; libc-test says which of them behave. It runs
against the hybrid archives, where a failure is either Nelibc's or the host
libc's (the glibc baseline below tells them apart), and against the
standalone archives, where every symbol and header is Nelibc's
("Standalone profile" below).

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
| Nelibc gcc standalone, branch `libc-test-standalone` on main `3ea86f3`, `make libc-test-standalone-gcc` | 2026-11-07 | 88 of 343 (`tools/libc_test_expected_standalone_gcc.txt`) |
| Nelibc zig standalone, same branch, `make libc-test-standalone-zig` | 2026-11-07 | 66 of 343 (`tools/libc_test_expected_standalone_zig.txt`) |

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

## Standalone profile

```sh
make libc-test-standalone-gcc LIBC_TEST=../libc-test
make libc-test-standalone-zig LIBC_TEST=../libc-test
```

`tools/libc_test_standalone.sh` does not use libc-test's Makefile, which
assumes a hosted toolchain (`-lpthread -lm -lrt`, dynamic binaries, `.so`
helpers). It copies `src/common`, `functional`, `regression` and `math` into
`build/libc-test-standalone-<cc>/` and builds each test itself:

- **Headers: Nelibc's only.** `-nostdinc` removes the host libc's headers;
  the compiler's own freestanding headers (`stddef.h`, `stdint.h`,
  `stdarg.h`, `limits.h`, `float.h`, and clang's `tgmath.h`) remain, as the
  task of a freestanding compiler. Nelibc's standalone headers include Linux
  UAPI headers (`<linux/fcntl.h>`, `<asm/ioctl.h>`, `<asm/socket.h>`,
  `<linux/mount.h>`, `<linux/xattr.h>`), so the driver makes a directory of
  symlinks to only `linux/`, `asm/` and `asm-generic/` (from `/usr/include`
  or `UAPI_INCLUDE`) and puts it last with `-idirafter`. Zig adds the native
  libc's include directories even under `-nostdinc`, so the zig run uses
  `-target x86_64-linux-none`, which adds only Zig's resource headers.
  The flags are libc-test's own (`-std=c99 -D_POSIX_C_SOURCE=200809L
  -fno-builtin -frounding-math -Werror=implicit-function-declaration` and
  the rest of `Makefile`/`config.mak.def`) minus the glibc-specific
  `-D_FILE_OFFSET_BITS=64`, plus the standalone smokes' `-DNELIBC_STANDALONE
  -ffreestanding -fno-stack-protector -fno-pie`.
- **Link: as the standalone smokes link.** `-nostdlib -static -no-pie
  -Wl,-e,_start`, Nelibc's own `_start`, the archive inside
  `--whole-archive`, no libgcc, no host libc. A binary with any `nm -u`
  symbol left counts as a link failure. `functional/tls_align` links its
  helper `tls_align_dso.o` statically, as libc-test's static build does; the
  other `*_dso` helpers are shared objects and are not tests.
- **Run: libc-test's own `runtest.c`.** Nelibc standalone has everything the
  runner needs (`fork`, `execv`, `getrlimit`/`setrlimit`, `sigprocmask`,
  `signal`, `sigtimedwait`, `kill`, `waitpid`, `getopt`, `strsignal`), so the
  runner is built and linked the same way and keeps libc-test's semantics: a
  forked child, `RLIMIT_STACK` of 100 KiB, a 5 s timeout, `FAIL` on a nonzero
  status, a signal or a timeout. A shell runner with `timeout(1)` would drop
  the stack limit, which already caught one Nelibc bug in the hybrid runs
  (slice 2's TZif reader). Before trusting it, the Nelibc-built runner was
  checked on a passing program, `/bin/false` (`[status 1]`), `sleep 3` with
  `-t 1` (`[timed out]`) and a script that kills itself with `SIGSEGV`
  (`[signal Segmentation fault]`). The runner itself must link with no
  undefined symbol, or the driver exits 2.

Each test is classified as `pass`, `build` (the first missing header,
undeclared function, identifier or type is recorded; GCC stops at the first
missing header, so later gaps in that file are hidden), `link` (undefined or
duplicate symbols recorded), `fail` (nonzero status), `timeout`, or `crash`
(signal). `results.tsv` holds one row per test, `summary.txt` the counts and
the missing headers and symbols ranked by the tests they block. The `FAIL`
marker is matched anywhere on a line, as in the hybrid driver. Binaries are
deleted after their run (`KEEP_BINARIES=1` keeps them): the zig ones carry
the 5 MB archive whole and the container disk is small.

The expected list holds `name [category]`. The gate compares names: a test
outside the list fails the run (exit 1), a listed test that passes is printed
for removal, and a listed test that still fails under another category
(`link` to `fail`, say) is printed for the list to be updated without failing
the run. Both checks were exercised by editing the list in the container:
removing `regression/ftello-unflushed-append` exited 1 with it as a new
failure, a fabricated `functional/string` entry was reported as now
passing, and `functional/spawn` recorded as `[link]` was reported as a
category change.

### What the first standalone runs showed (2026-11-07)

Container `nelibc-writ`, user `build`, libc-test `63e36e9`, GCC 12.2 and Zig
0.17.0, branch `libc-test-standalone` (main `3ea86f3` plus the harness; no
Nelibc source changed). Each run took under ten seconds of wall time and
gave the same counts every time it was run (four times each, the last two
from a fresh copy of commit `3a2254a` through the make targets, exit 0).

| Directory (tests) | gcc pass / build / link / fail / timeout | zig pass / build / link / fail / timeout |
| --- | --- | --- |
| `functional` (74) | 26 / 45 / 0 / 2 / 1 | 36 / 29 / 0 / 8 / 1 |
| `regression` (70) | 32 / 31 / 6 / 1 / 0 | 44 / 19 / 3 / 4 / 0 |
| `math` (199) | 197 / 2 / 0 / 0 / 0 | 197 / 2 / 0 / 0 / 0 |
| total (343) | 255 / 78 / 6 / 3 / 1 | 277 / 50 / 3 / 12 / 1 |

No test crashed with a signal in either run. Of libc-test's support code,
`common/utf8.c` (`t_setutf8`) does not compile in either profile
(`<langinfo.h>`), and `common/vmfill.c` (`t_vmfill`) does not compile with
GCC (`<limits.h>`); the tests that call them fail to link.

The two compilers differ in one thing that matters: GCC's `<limits.h>`
`#include_next`s the C library's `<limits.h>`, and Nelibc ships none, so
every test that includes it fails to build with GCC (32 tests, and
`t_vmfill`). Clang's freestanding `<limits.h>` stands alone, so the zig run
builds those tests and is the deeper view: of the 32, 18 pass, 9 fail at
run time (below), and 5 stop at the next gap (`PATH_MAX`, which no
freestanding `<limits.h>` provides; `M_PI`; `shm_open`; `<langinfo.h>`;
`<dlfcn.h>`). The 12 zig runtime failures are not
zig-specific: in the gcc run the same tests either fail the same way
(`spawn`, `tls_align`, `ftello-unflushed-append`, and `sem_open` times out)
or do not build.

#### Follow-up: `<limits.h>`, `<inttypes.h>`, signed zero (branch `standalone-headers`)

The first baseline's largest gap was a header, not code. Nelibc now ships
`include/limits.h`: in hybrid builds it is `#include_next` of the host's, in
the standalone profile it defines the C limits for x86-64 LP64 and the POSIX
values Nelibc enforces (`PATH_MAX`, `PTHREAD_KEYS_MAX` 128,
`PTHREAD_STACK_MIN` 16384, `ATEXIT_MAX` 32, `IOV_MAX`, the `_POSIX_*`
minimum maxima, ...), the same numbers `sysconf` reports; `ARG_MAX` is
omitted because it follows `RLIMIT_STACK` at run time. Writing it showed
`sysconf(_SC_THREAD_KEYS_MAX)` returning 1024 while the key table holds 128;
it now returns 128. `include/inttypes.h` no longer `#include_next`s in
standalone mode: it spells the `PRI*`/`SCN*` macros, choosing the length
modifier of each `FAST`, `MAX` and `PTR` type from the compiler's own
`<stdint.h>` limits (GCC and clang disagree on `int_fast16_t`).

The hybrid branch of `include/limits.h` first carried the same include
guard as the standalone one. GCC's `<limits.h>` includes `"syslimits.h"`,
whose `#include_next <limits.h>` restarts the search and lands in Nelibc's
header a second time; the guard made that inclusion empty, so GCC never
reached glibc's header and `PATH_MAX` and the other POSIX limits vanished
from every GCC hybrid build that includes `<limits.h>`. The gate's target
list does not run `libc-test-gcc`, which is where it showed:
`functional/argv` stopped building. Found and fixed on branch
`standalone-batch-2`: the guard now covers only the standalone branch, and
`make libc-test-gcc` is back to 30 failing, all expected (zig was never
affected: clang's `<limits.h>` reaches the next one directly).

Rerun on the branch, same checkout and method:

| | pass | build | link | fail | timeout |
| --- | --- | --- | --- | --- | --- |
| gcc | 278 (was 255) | 49 (was 78) | 3 (was 6) | 12 (was 3) | 1 |
| zig | 279 (was 277) | 48 (was 50) | 3 | 12 | 1 |

Two fixes ride on the same branch, both found by nebox's GNU comparisons
rather than by libc-test:

- `printf` took the sign of `%e`/`%f`/`%g` from `value < 0`, so `-0.0`
  printed `0.000000` and a negative NaN `nan`. It now reads the sign bit
  (`-0.000000`, `-0`, `-nan`, as glibc); `tools/lib_consumer_smoke.c` checks
  them.
- Local time read a zone file only when TZ contained `/` or began with `:`,
  so `TZ=Japan` or `TZ=EST5EDT` were parsed as POSIX strings, and `TZDIR` was
  ignored. As in glibc, any TZ value now names a file under `TZDIR`
  (default `/usr/share/zoneinfo`) first and is read as a POSIX string only
  when no such file exists. The first version of this leaked `ENOENT` from
  the failed probe of `TZ=UTC0` into `errno`, which `mktime`'s callers test
  when the result is -1; the standalone smoke caught it (exit 431). The
  zone-file reader now saves and restores `errno`, and the tzset cache counts
  "still no file" as unchanged instead of rerunning `tzset` on every call.
  `tools/syscall_smoke.c` checks `TZ=Japan` and `TZDIR` with `TZ=Tokyo`.
- An unknown zone (`TZ=Foo/Bar`, `TZ=:Foo`) printed `UTC`; glibc keeps the
  name's leading letters at offset 0 (`Foo`, `+0000`), and so does Nelibc now
  (follow-up on branch `tz-unknown-name`, checks 498 and 499).

Gate: the 155-target list on `8b286be`, Nelua `dabbd9ef`: 155/155 exited 0,
no warnings; logs in `/work/artifacts/gate-sh2-20261107-1110/`.

The gcc runtime failures rose from 3 to 12 because nine tests that did not
build now build and reach the bugs listed below (the same nine the zig run
already reported). The expected lists were rewritten from these runs. The
table below is the first baseline and keeps its `<limits.h>` and
`<inttypes.h>` rows for the record.

#### Missing headers and declarations

Tests blocked at compile time, GCC run (zig in parentheses where it
differs). Each item is a header or declaration Nelibc's standalone headers
lack; the "in archive" column says whether the standalone archive defines
the function anyway (`nm -g --defined-only`), which none of them does except
where noted.

| Missing | Tests | In archive |
| --- | --- | --- |
| `<limits.h>` | 32 (zig 0; `PATH_MAX` 1) | header only |
| `<regex.h>` | 8 | no |
| `<search.h>` | 4 | no |
| `<dlfcn.h>` | 3 (zig 4: `tls_get_new-dtv`, behind `<limits.h>` with GCC) | no |
| `<langinfo.h>` | 2 (zig 3), and `common/utf8.c` | no |
| `<iconv.h>`, `<libgen.h>`, `<pwd.h>`, `<resolv.h>` | 2 each | no |
| `<crypt.h>`, `<mntent.h>`, `<setjmp.h>`, `<sys/msg.h>`, `<sys/sem.h>`, `<sys/shm.h>`, `<sys/statvfs.h>`, `<sys/syscall.h>`, `<wordexp.h>` | 1 each | no |
| `<tgmath.h>` | 1 (zig 0: clang ships one) | header only |
| `<inttypes.h>` in standalone mode | 1 (`functional/qsort`) | a header bug: `include/inttypes.h` does `#include_next <inttypes.h>` even under `NELIBC_STANDALONE` |
| `random`, `srandom`, `initstate`, `setstate` (`<stdlib.h>`) | 1 | no |
| `lrand48`, `mktemp` (`<stdlib.h>`) | 1 each | no |
| `putc` (`<stdio.h>`) | 1 (`functional/memstream`) | no |
| `popen`, `pclose` (`<stdio.h>`) | 1 | no |
| `execl` and `vfork`, `execle` (`<unistd.h>`) | 1 each | no |
| `shm_open`, `shm_unlink` (`<sys/mman.h>`) | zig 1 (`pthread_cancel-points`) | no |
| `isnormal` (`<math.h>`) | 1 (`math/fpclassify`) | macro missing |
| `isgreater`, `isgreaterequal`, `isless`, `islessequal`, `islessgreater`, `isunordered` (`<math.h>`) | 1 (`math/isless`) | macros missing |
| `M_PI` (`<math.h>`, XSI; the test defines `_XOPEN_SOURCE 700`) | zig 1 (`functional/snprintf`) | macro missing |
| `st_atime`/`st_mtime`/`st_ctime` (`<sys/stat.h>`; only `st_ctim` exists) | 1 (`functional/stat`) | macros missing |

**Fixed** (branch `standalone-batch-2`): `<langinfo.h>`. `nl_langinfo` and
`nl_langinfo_l` answer from the locale data the standalone profile already
has, with musl's item numbering: `CODESET` is `UTF-8` when `LC_CTYPE` is
`C.UTF-8` and `ASCII` in the C/POSIX locale (musl's spelling; glibc says
`ANSI_X3.4-1968`; the standalone C locale rejects bytes above 0x7f, so it
is ASCII); `RADIXCHAR`/`THOUSEP` follow `LC_NUMERIC` (`.`/`""`, or
`,`/`.` for `de_DE.UTF-8`); the `LC_TIME` names and `D_T_FMT`, `D_FMT`,
`T_FMT`, `T_FMT_AMPM` are the ones `strftime` and `strptime` use (German
names and `%d.%m.%Y` for `de_DE.UTF-8`; `AM_STR`/`PM_STR` stay `AM`/`PM`
there because `strftime`'s `%p` prints them); `CRNCYSTR` is `""` in C and
`+€` for `de_DE.UTF-8` monetary; `LC_MESSAGES` is C only (`^[yY]`,
`^[nN]`, `yes`, `no`); the `ERA*` and `ALT_DIGITS` items and unknown items
are `""`. A per-thread locale from `uselocale` is honoured. Hybrid builds
`#include_next` the host's header. libc-test's `common/utf8.c`
(`t_setutf8`) now builds, and `functional/mbc` and
`regression/fgetwc-buffering` pass in both runs (gcc 288, zig 289).
`functional/clocale_mbfuncs` (`MB_CUR_MAX`) and `functional/swprintf`
(`M_PI`) also include `<langinfo.h>` and now stop at those identifiers.
Checks 78 to 80 in `tools/syscall_smoke.c`.

**Fixed** (branch `search-h`): `<search.h>`. `src/search_standalone.nelua`
follows musl 1.2.5: `hcreate`/`hsearch`/`hdestroy` and the GNU `_r` forms as
an open-addressing table with triangular probing that doubles at three
quarters full; `tsearch`/`tfind`/`tdelete`/`twalk`/`tdestroy` as an AVL tree
(`tdelete` returns the parent, or the root slot when the root goes);
`lsearch`/`lfind`; `insque`/`remque`. Hybrid builds include the host's
header. `functional/search_hsearch`, `search_insque`, `search_lsearch` and
`search_tsearch` pass in both runs (gcc 292, zig 293); checks 81 to 86 in
`tools/syscall_smoke.c` cover 257 shuffled keys with half deleted and an
in-order walk, a 100-key table grown from 8 slots, and the list helpers.

Gate for `standalone-batch-2` and `search-h` together: the 159-target list
(the 155 plus `libc-test-gcc`, `libc-test-zig`, `libc-test-standalone-gcc`,
`libc-test-standalone-zig`) on `9f3ba3e`, Nelua `dabbd9ef`: 159/159 exited
0, no warnings; logs in `/work/artifacts/gate-searchh-20261107-1323/`.

**Fixed** (branch `math-macros`): `<math.h>`, `MB_CUR_MAX`, wide `%s`.
- The standalone `isinf` and `isfinite` computed `(x) - (x)`, which raises
  `FE_INVALID` for an infinity, and the classification macros evaluated
  their argument more than once; C17 7.12.3 forbids both. With GCC and
  clang they are now the compiler builtins (`__builtin_fpclassify`,
  `__builtin_isinf`, ...), which evaluate once and raise nothing; TinyCC
  keeps the old expressions, documented as such. Added `isnormal`, the
  `isgreater` family, `float_t`/`double_t`, `math_errhandling`
  (`MATH_ERRNO | MATH_ERREXCEPT`: Nelibc's math sets both) and the XSI
  `M_*` constants and `MAXFLOAT`. `tools/math_core_standalone_smoke.c`
  checks that classifying infinities and NaNs leaves the flags clear
  (600-602).
- `MB_CUR_MAX` is `__ctype_get_mb_cur_max()`: 4 under a UTF-8 `LC_CTYPE`,
  1 in the strict ASCII C locale (checks 521-523).
- In `swprintf`, the precision and width of `%s` count wide characters
  (C17 7.29.2.1), and `vswprintf`'s sizing pass counted bytes while the
  writing pass counted characters, so `"%.1s"` of `"\xc3\x80!"` overran
  the byte buffer by its terminator and failed. Both passes count
  characters now; invalid input is `EILSEQ`.

`math/fpclassify`, `math/isless`, `functional/snprintf` and
`functional/swprintf` pass (gcc 307, zig 308, from 300 / 301).
`functional/clocale_mbfuncs` now builds and fails: Nelibc's C locale is
strict ASCII and `mbrtowc` rejects bytes 0x80-0xff, where musl (and POSIX
Issue 8's single-byte POSIX locale) maps every byte. glibc 2.36 fails it too.
This is a recorded divergence, not changed here.

Gate for `math-macros` and `pwd-grp` together (branch `gate-merge`, which
also declares `exit`/`_Exit`/`abort`/`_exit` noreturn and makes
`tools/libc_test.sh` drop its binaries after recording): 159/159 on
`0a069fc` with a cleared zig cache, no warnings;
`/work/artifacts/gate-merge3-20261107-1543/`. Standalone libc-test: gcc 309,
zig 310 of 343. The first run of this stack, on `b057047`, exited 0
everywhere but logged a `-Wreturn-type` warning that zig's object cache had
hidden in earlier gates: a gate should run with a cleared zig cache.

Gate coverage note (2026-11-07): AGENTS.md lists `libc-test-gcc` and
`libc-test-zig` in the gate, but the container's gate script ran only the
155 smoke, diff, host and shootout targets. The `<limits.h>` change in
`8b286be` broke `functional/argv` in the GCC hybrid run (fixed in
`7ee0613`) and passed that 155-target gate. The script now also runs
`libc-test-gcc`, `libc-test-zig`, `libc-test-standalone-gcc` and
`libc-test-standalone-zig` (159 targets); gate results recorded before
this note did not include libc-test.

**Fixed** (branch `regex-h`; gate 159/159 on `125f247` after merging main, no warnings, `/work/artifacts/gate-regex-20261107-1425/`): `<regex.h>`. `src/regex_standalone.nelua`
implements `regcomp`, `regexec`, `regerror` and `regfree` over the regex
engine of nebox (a Pike VM with leftmost-longest matching), with musl
1.2.5's parser rules, flag values, error codes and messages; hybrid builds
include the host's header. It supports BRE and ERE, `REG_EXTENDED`,
`REG_ICASE`, `REG_NOSUB`, `REG_NEWLINE` (`.` and non-matching lists never
match a newline, `^`/`$` also match after/before one), `REG_NOTBOL`,
`REG_NOTEOL`, `re_nsub` and unmatched groups as -1/-1, back-references in
BRE (in an ERE `\1` is the literal `1`, as in musl), musl's extensions
(`\|`, `\+`, `\?` in BRE; `\t \n \r \f \a \e \w \W \s \S \d \D \b \B \< \>`;
`\xHH`, `\x{HHHH}`) and `regerror`'s truncation (it returns the size
needed). Matching is byte-wise, as in the C locale. In a UTF-8 `LC_CTYPE` a
pattern that is not valid UTF-8 is `REG_BADPAT` and a multibyte literal is
one atom, but `.` and bracket expressions match single bytes, a multibyte
character inside brackets is `REG_ECOLLATE`, and `REG_ICASE` folds ASCII
only. Not implemented: `REG_STARTEND` (not in musl). Differences from musl,
all checked against musl and glibc in the container with a differential
harness of 666 pattern-and-flag pairs by 39 strings, with and without
`REG_NOTBOL|REG_NOTEOL` (not committed): `[.c.]` and `[=c=]` of one
character are accepted (musl: `REG_ECOLLATE`; glibc accepts them);
`RE_DUP_MAX` is 32767 as `<limits.h>` says (musl 255); subexpression spans
follow the GNU rule, so `(a|ab)(bc|c)` on `abc` gives `(0,1)(1,3)` as glibc
does (musl `(0,2)(2,3)`); a back-reference to a group that did not
participate fails (glibc; musl matches it as empty); and where musl misses a
match (`\(a*\)b\1` on `ab` and `x\(^\)*` on `x`, which glibc matches; ERE
`^*a` under `REG_NOTBOL`, which glibc rejects) Nelibc matches. As in musl, a
stale group from an earlier iteration (`((a)|b)*` on `ab`, group 2) is -1/-1
(glibc keeps `(0,1)`). Limits that return `REG_ESPACE`: groups nested over
256 deep, a syntax tree over 2048 high, a program over 262,144 instructions
(matching costs text length times program size), and, with back-references,
over 65,536 live threads at one position (`\(a*\)\1$` over 10,000 `a`s hits
it). Matching also copies every capture slot per thread, so thousands of
groups are slow (2,000 groups over 10,000 bytes: 11 s, GCC standalone
archive in the container). All eight `regression/regex*` tests pass in both
runs (gcc 292 to 300, zig 293 to 301); checks 87 to 91 and 504 in
`tools/syscall_smoke.c` cover the error codes, spans, back-references, the
flags, `regerror` and a UTF-8 locale.

**Fixed** (branch `pwd-grp`): `<pwd.h>` and `<grp.h>`.
`src/pwd_standalone.nelua` follows musl 1.2.5's `src/passwd`: `getpwnam`,
`getpwuid`, `getpwnam_r`, `getpwuid_r`, `setpwent`/`getpwent`/`endpwent`,
`fgetpwent`, `putpwent`; `getgrnam`, `getgrgid`, `getgrnam_r`, `getgrgid_r`,
`setgrent`/`getgrent`/`endgrent`, `fgetgrent`, `putgrent`, `getgrouplist`,
`initgroups`. They read `/etc/passwd` and `/etc/group` as bytes; there is no
NSS, nscd, or shadow (`<shadow.h>`) support, and hybrid builds include the
host's headers and use its NSS. As in musl: a field ends at `:` and the last
one (shell, member list) runs to the end of the line, so extra colons stay in
it; `#` starts no comment; a line with too few fields is skipped; a numeric
field is decimal digits only, empty reads as 0, and a sign, space or other
byte before the next `:` drops the line; a NUL byte ends the line; a missing
file (`ENOENT`, `ENOTDIR`) is "not found" (0 and a NULL result), any other
`fopen` failure or a read error is returned (`_r`) or left in `errno`; a
lookup that finds nothing leaves `errno` as it was; the non-`_r` functions
share static storage that the next call overwrites; `setpwent` closes the
stream and the next `getpwent` reopens the file; `getgrouplist` puts the
primary group first, adds each group naming the user in file order without
removing duplicates, and returns -1 with the needed count in `*ngroups` when
the array is short. Differences from musl, all on malformed files or sizes:
a last line without a newline keeps its last byte (musl drops it); an empty
name field is the empty name (musl folds the next field into it) and an
empty line is skipped (musl reads past the line's end); a number above
4294967295 saturates to 4294967295 as glibc does (musl wraps, so
`4294967296` reads as uid 0); empty members (`a,,b`, a trailing comma) are
skipped as glibc does (musl yields `a` and `,b` and reads past a trailing
comma); the `_r` functions return `ERANGE` exactly when the entry does not
fit (the line's bytes, plus for a group a pointer-aligned `gr_mem` array;
musl compares with its line buffer's capacity and asks 32 spare bytes for a
group); a line too long for memory is `ENOMEM`, not a truncated entry;
`initgroups` sizes its list from `getgrouplist` (musl stops at 32 groups),
and the kernel's `NGROUPS_MAX` (65536) applies. `regression/getpwnam_r-crash`
and `getpwnam_r-errno` pass in both runs (gcc 303 to 305, zig 304 to 306;
the 300/301 above predate the `thread-cancel` merge, which added three).
Checks 505 to 520 in `tools/syscall_smoke.c` read fixture files through the
test-only `__nelibc_set_passwd_path` and `__nelibc_set_group_path` (exported
only by the `NELIBC_TESTING` build of the module that the zig and gcc smokes
link; the TinyCC archive includes the module but its smoke is not run): every
rule above, `ERANGE` at the exact size (also from a misaligned buffer), a
directory as the database (`EISDIR`), a missing file and an `ENOTDIR` path,
enumeration and restart, `putpwent`/`putgrent` read back with
`fgetpwent`/`fgetgrent`, and `getgrouplist` and `initgroups` over 70 groups
(as the unprivileged `build` user `initgroups` fails in `setgroups` with
`EPERM`, which is what the check asserts; a successful `initgroups` was not
observed).

#### Missing symbols

Link failures, GCC run: `daemon` (`regression/daemon-failure`), `t_vmfill`
(3 tests, libc-test support code blocked by `<limits.h>`), `t_setutf8`
(`regression/fgetwc-buffering`, blocked by `<langinfo.h>`), and
`regression/flockfile-list`, which defines its own `malloc`, `calloc`,
`realloc`, `free` and `aligned_alloc`: with the archive linked whole, the
standalone allocator's exports collide (`multiple definition`). That test
checks behavior with an interposed malloc; it cannot run under the
whole-archive link convention, and Nelibc's static allocator is not
replaceable today.

Because a missing declaration stops compilation first, the link stage sees
few symbols. To map the functions behind the compile failures, each of the
84 build- and link-failing gcc tests was compiled once more against glibc
2.36's headers (compile only, never linked or run, not part of the gate) and
its undefined symbols compared with the archive's defined symbols. Dropping
glibc-internal names (`__isoc99_*`, `__xpg_basename`, `__sigsetjmp`,
`__pthread_register_cancel` and kin, `__sysv_signal`,
`__libc_current_sigrtmin`, `__ctype_get_mb_cur_max`,
`_GLOBAL_OFFSET_TABLE_`) and libc-test's own `t_*`, the functions the
standalone archive lacks, ranked by tests blocked:

| Function family | Tests |
| --- | --- |
| `regcomp`, `regexec` (8), `regerror` (6), `regfree` (2) | 8 |
| `dlopen`, `dlsym` (4), `dlerror` (2), `dlclose` (1) | 4 |
| `hsearch`/`hcreate`/`hdestroy`, `insque`/`remque`, `lsearch`/`lfind`, `tsearch`/`tfind`/`tdelete`/`twalk` | 4 |
| `ftok` with `msg*`, `sem*` (SysV), `shm*` (SysV) | 3 |
| `nl_langinfo` | 2, plus `t_setutf8` |
| `iconv_open`, `iconv`, `iconv_close` | 2 |
| `getpwnam_r` | 2 |
| `dn_expand` | 2 |
| `setjmp`/`longjmp`/`sigsetjmp`/`siglongjmp`; `random` family; `lrand48`; `mktemp`; `popen`/`pclose`; `putc`; `execl`; `execle`; `vfork`; `shm_open`/`shm_unlink`; `daemon`; `crypt`; `basename`; `dirname`; `wordexp`/`wordfree`; `getmntent`/`getmntent_r`/`endmntent`; `statvfs`; `syscall` | 1 each |

`dlopen` in a static standalone binary would at best return an error; the
four `dl*` tests are not expected to pass in this profile.

#### Wrong behavior

Run-time failures, with libc-test's own messages. Tests marked "zig" are
behind `<limits.h>` with GCC; the rest fail identically in both runs.

- `functional/tls_align`: `tls_align.c:16: name is not set for t[0]` (and
  1 to 3). The test's `__attribute__((constructor))` never ran: the
  standalone `_start` (`src/sys/start_linux_x86_64_standalone.S`) does not
  walk `.preinit_array`/`.init_array`, and nothing in `src/` mentions
  them. Static constructors are silently skipped in every standalone binary;
  `.fini_array` is likely skipped too (not tested here). **Fixed** (branch
  `init-fini`): `_start` now runs `.preinit_array` then `.init_array` with
  argc, argv and envp before `main`, and `exit` runs `.fini_array` in
  reverse after the atexit handlers and before the stdio flush, glibc's
  order. `functional/tls_align` passes in both runs (gcc 279, zig 280);
  `tools/syscall_smoke.c` checks that a constructor ran (501) and that a
  forked child's atexit handler runs before its destructor (503). `exit`
  reaches the `.fini_array` walker through `nelibc_exit_fini_hook`, which
  `_start` sets: the host profile keeps `exit` but not `_start`, and its
  `host-check` (no undefined symbols) caught a direct reference.

  Gate for the fixes recorded since the standalone-headers merge (unknown
  TZ names, constructors and destructors, `ftello` append, `sem_open`
  reuse, `pthread_create` `EAGAIN`, exit after the last `pthread_exit`):
  the 155-target list on `21463a5`, Nelua `dabbd9ef`: 155/155 exited 0, no
  warnings; logs in `/work/artifacts/gate-stack3-20261107-1208/`. An earlier
  run of the same stack on `9d54945` failed the five host targets on the
  undefined `nelibc_run_fini_array`, fixed in `21463a5`.
- `regression/ftello-unflushed-append`: `ftello is broken before flush: got
  3, want 7`. After `fdopen(fd, "a")` on a 4-byte file and a 3-byte
  `fwrite`, `ftello` ignores that appends go to the end of the file. **Fixed** (branch `ftello-append`): with buffered output on an
  `O_APPEND` descriptor (`F_GETFL`), `ftell`/`ftello` measure from
  `SEEK_END`, as musl does; gcc 280, zig 281.
- `functional/sem_open` (timeout): `sem_open.c:37: could not reopen sem: got
  0x7f..., want 0x7f...`. A second `sem_open` of the same name in the same
  process maps a new address instead of returning the existing `sem_t *`
  (POSIX requires the same address until `sem_close`); the test then
  deadlocks and is killed at 5 s. **Fixed** (branch `sem-open-reuse`): open named semaphores
  are registered by the file's device and inode with a reference count, so
  a second `sem_open` returns the same address and `sem_close` unmaps on the
  last reference, as musl does; the `flock` already held across `sem_open`
  serializes the lookup. gcc 281, zig 282, no timeouts left.
- `functional/spawn`: `posix_spawnp(&pid, "echo", &fa, 0, (char
  *[]){"echo","hello",0}, 0) failed, got 22 want 0`. Nelibc rejects a null
  `envp` with `EINVAL` (`src/spawn_standalone.nelua`,
  `nelibc_posix_spawn_common`). POSIX does not define a null `envp`; musl and
  glibc pass it to `execve`, which Linux treats as an empty environment.
  A compatibility gap rather than a conformance bug.
- `functional/fscanf` (zig): `fscanf(f, " %n%*d%n", &x, &y) failed (-1 !=
  0)` and `x failed (1 != 6)` on input `"      42"`: the return is `EOF`
  instead of 0, and `%n` after leading white space stores 1 instead of 6.
  **Fixed** (branch `standalone-batch-2`): a white-space directive read one
  character instead of all white space, and reaching the end of input there
  counted as an input failure; a conversion that completed at the end of
  input (`%*d` on `42`) also marked an input failure, so a format whose
  only conversions were suppressed returned `EOF`. As in musl and C17
  7.21.6.2, the end of input now fails only a directive that needs a
  character and gets none (a literal, or a conversion that matched
  nothing); the same rule now holds for `%s`, `%[`, `%ls` and `%l[`, narrow
  and wide.
- `functional/fwscanf` (zig): the same `L" %n%*d%n"` case (-1, `x` 1), and
  `ftell` one past the consumed input after a failed match (`13 != 12`,
  `5 != 4`, `3 != 2`, lines 61, 75, 110): the wide scanner does not push back
  the character that ended a conversion. **Fixed** (same branch): the
  scanner did push it back, but `ungetwc` kept the wide character in a slot
  of its own that `ftell` (and `fseek`, which never cleared it) ignored.
  `ungetwc` now pushes back the character's multibyte encoding, as musl
  does, so the next read decodes it again and `ftell` moves back by its
  length.
- `functional/ungetc` (zig): `unget failed after fscanf: -1 != 120` after
  `fscanf(f, "%[x]", a)` matched nothing; the following `fgetc` calls
  return `h`, `e` instead of `x`, `h`. **Fixed** (same branch): the scanner's
  own pushback of `h` occupied the one-byte `ungetc` slot, so the caller's
  `ungetc` failed. A byte pushed while that slot is full now goes in front
  of it into the 8-byte scan pushback area (musl keeps 8 bytes of unget room
  too), and is read first. Checks 74 to 77 in `tools/syscall_smoke.c`; gcc
  286, zig 287.
- `functional/pthread_cancel` (zig): `foo[0] == 1 failed (cleanup handler
  failed to run)` (lines 84, 92 to 95): cancellation does not run
  `pthread_cleanup_push` handlers. **Fixed** (branch `thread-cancel`): the thread trampoline called
  `pthread_testcancel()` before the start routine, so a cancel sent right
  after `pthread_create` ended the thread before its first
  `pthread_cleanup_push`. Thread start is not a cancellation point; the
  call is gone.
- `regression/pthread_cancel-sem_wait` (zig): `seqno == 1 failed` for
  uncontended and blocking `sem_wait` and `sem_timedwait` (lines 55, 65, 75,
  85): `sem_wait` and `sem_timedwait` are not cancellation points. **Fixed** (same branch): they were, but the test's thread was cancelled
  early by the trampoline check above, and `pthread_setcancelstate(ENABLE)`
  acted on a pending deferred cancel; it is not a cancellation point, so
  only asynchronous cancellation is acted on there now.
- `functional/pthread_mutex_pi` (zig): `PTHREAD_MUTEX_NORMAL relock did not
  deadlock, got Resource deadlock avoided` (line 145): relocking a normal
  priority-inheritance mutex returns `EDEADLK`; POSIX says a normal mutex
  deadlocks. **Fixed** (same branch): on `EDEADLK` from `FUTEX_LOCK_PI` for a
  non-error-checking mutex, the lock waits out its deadline (`ETIMEDOUT`) or
  forever, as musl does. gcc 295, zig 296. Gate: 159/159 on `82c4474`, no warnings,
  `/work/artifacts/gate-tcancel-20261107-1357/`.
- `regression/pthread_create-oom` (zig): `pthread_create should fail with
  EAGAIN but failed with 12 (Cannot allocate memory)` (line 24): POSIX
  specifies `EAGAIN` for lack of resources. **Fixed** (branch `pthread-create-eagain`): a failed
  object allocation or stack/TLS map now returns `EAGAIN`.
- `regression/pthread_exit-dtor` (zig): `atexit handler was not run after
  last thread exited (exited=1, signal=0, status=0, want exit status=1)`
  (line 64): when the last thread calls `pthread_exit`, the process exits
  with status 0 without running `atexit` handlers. **Fixed** (same branch): nelibc counts live threads; when the
  last one leaves through `pthread_exit` or by returning from its start
  routine, the process exits as if `exit(0)` were called (atexit handlers,
  destructors, stdio flush), as POSIX requires. The count resets to 1 in a
  fork child. gcc 283, zig 284.
- `functional/pthread_robust` (zig): `pthread_mutex_destroy(&mtx) failed:
  ... Device or resource busy (destroying mutex)` (line 61), for all four
  pshared/PI combinations. The test destroys a robust mutex it has just
  relocked; POSIX leaves destroying a locked mutex undefined and allows
  `EBUSY`, so this is a divergence from musl, not a conformance bug.

Every `math/*` test that builds passes in both runs (197 of 199), as in the
hybrid runs: the libm ports are the same code. The two that do not build
lack `<math.h>` macros (above).

### Next work (not done on this branch)

In order of tests unblocked per change: ship a `<limits.h>` with the POSIX
limits (32 gcc tests and `t_vmfill`); fix `<inttypes.h>` and add the
`<math.h>` classification/comparison macros, `M_PI`, the `st_*time` macros,
and `putc`, `popen`/`pclose`, `random`, `lrand48`, `mktemp`, `execl`/`execle`,
`vfork`; run `.init_array` in `_start`; the scanf `%n`/`EOF`/pushback bugs and
`ungetc` after a failed scan; `ftello` in append mode; `sem_open` reuse;
cancellation cleanup handlers and `sem_wait` as a cancellation point; then the
missing families (`<regex.h>`, `<search.h>`, `<setjmp.h>`, `<langinfo.h>`,
SysV IPC, `iconv`, `getpwnam_r`).

## Not run

- The static musl hybrid: libc-test's build expects a hosted toolchain.
- Static hybrid test binaries (see above).
- The standalone TinyCC archive: no `libc-test-standalone-tcc` target (the
  container's TinyCC 0.9.27 cannot build the TinyCC profile).
- libc-test's `api/` header tests and `musl/` internal tests in the standalone
  profile: the standalone driver runs only `functional`, `regression` and
  `math`.
- The standalone shared-object tests as libc-test intends them: the four
  `dlopen`/`tls_*_dlopen`/`tls_get_new-dtv` tests stop at `<dlfcn.h>`.
