# API coverage manifest

Run `make api-coverage` to refresh `build/api-coverage.csv` from the public
headers and current archive exports. It compares Zig/glibc, GCC/glibc,
Zig/musl, TinyCC/glibc, and standalone Zig, GCC, and TinyCC archives. When
TinyCC is unavailable, `make api-coverage-available` refreshes
`build/api-coverage-available.csv` for Zig/glibc, GCC/glibc, and Zig/musl.
When all four hybrid toolchains are available, `make api-coverage-hybrid`
also includes TinyCC/glibc in `build/api-coverage-hybrid.csv`.

The manifest records declarations, source locations, per-profile exports, and
smoke or differential files that mention each API name. Smoke references are
only a locator for review; they do not prove complete behavioral coverage.
`missing-standalone-export` is an implementation gap. A function omitted by a
hybrid archive is marked `implemented;host-delegated:<profile>` only when the
build intentionally relies on the selected libc for that symbol. Other
unexplained hybrid omissions are marked `standalone-export;unexported:<profile>`
so they remain visible as gaps. `malloc` is marked `implemented` in all seven
archive profiles. Static musl links Nelua's public `malloc`; its `free` and
`realloc` calls remain routed through ownership-aware wrappers so host-owned
allocations from libc paths remain valid.

The static musl hybrid routes `ungetc`, `freopen`, `fread`, `fwrite`, `fflush`,
`fileno`, `feof`, `ferror`, `clearerr`, `fgetpos`, `fsetpos`, `fseek`, and
`ftell`, `fseeko`, `ftello`, `setvbuf`, `flockfile`, `ftrylockfile`,
`funlockfile`, `fclose`, `localeconv`, `setlocale`, `atexit`, `exit`, `free`, and
`realloc` through exported
`__wrap_*` entry points. The coverage scanner counts those as the standard APIs for that profile;
consumers must link with the matching `-Wl,--wrap` flags (provided by the
Makefile variable `MUSL_STDIO_WRAP_FLAGS`). The musl archive exports Nelua
`malloc` and uses the ownership-aware `free`/`realloc` wrappers to support
allocations from both Nelibc and musl paths.

The declaration scanner handles the C declaration forms currently used by
these headers. Review new or unusual declaration syntax when extending the
header set. The implementation scanner also recognizes public function names
passed as the first argument to `DEFINE_*` wrapper macros in C adapters.

## Pinned musl API comparison

`docs/musl-1.2.5-api-coverage.csv` is a checked-in comparison against musl
1.2.5, built from the upstream release archive
`https://musl.libc.org/releases/musl-1.2.5.tar.gz` (SHA-256
`a9a118bbe84d8764da0ea0d28b3ab3fae8477fc7e4085d90102b8596fc7c75e4`). The
CSV keeps one row for every function declaration recognized in the release
headers and present as a global text/weak symbol in its `libc.a` or `libm.a`.
The audit intersects declarations and exports so function-like header macros
and parser false positives do not become supposed libc functions.

Regenerate it by building/installing that pinned release, then running:

```sh
python3 tools/musl_api_audit.py \
  --musl-include /path/to/musl-install/include \
  --musl-archive /path/to/musl-build/lib/libc.a \
  --musl-archive /path/to/musl-build/lib/libm.a \
  --nelibc-profile zig_glibc=build/libnelibc-zig.a \
  --nelibc-profile gcc_glibc=build/libnelibc-gcc.a \
  --nelibc-profile zig_musl=build/libnelibc-zig-musl.a \
  --nelibc-profile tcc_glibc=build/libnelibc-tcc.a \
  --output docs/musl-1.2.5-api-coverage.csv
```

The current snapshot contains 1,471 APIs: 761 exported by Nelibc in at least
one tested hybrid profile, 163 intentionally delegated to the host in hybrid
profiles, and 547 not declared by Nelibc. Every declared API in this pinned
baseline is either exported by at least one hybrid archive or intentionally
delegated to its host. The separate seven-profile inventory tracks per-profile
gaps. The audit removes preprocessor
directives and C++ linkage wrappers before parsing prototypes, while retaining
inline-function closing braces. No baseline API is currently marked as an
intentional omission; the 555 entries remain visible as unimplemented
scope candidates until an explicit policy records why an API is excluded.
No real-valued `math.h` APIs in this pinned musl baseline remain undeclared.
The largest remaining groups are wide characters (42), stdlib (32), network
database APIs (32), and unistd (29). The pthread rows without Nelibc declarations are two private musl
cleanup helper symbols. This is a breadth comparison, not a behavioral
conformance score.

`clearenv` is implemented in Nelua alongside the environment mutation
functions. It clears `environ`, releases Nelibc-owned entries, and permits
subsequent `setenv` calls to build a fresh vector. The seven-profile consumer
matrix checks empty-environment behavior and inheritance by a new process.

The GNU `get_current_dir_name` extension is implemented in Nelua by growing an
allocated `getcwd` buffer on `ERANGE`. The seven-profile consumer matrix checks
that it returns the same current directory as `getcwd`.

The POSIX `swab` byte-pair routine is implemented in Nelua. The consumer matrix
checks pair order, odd-length handling, and nonpositive counts.

`execv`, `execvp`, and GNU `execvpe` are implemented in Nelua on top of
`execve`. PATH search preserves `EACCES` precedence, and `execvp`/`execvpe`
fall back to `/bin/sh` for executable text files that return `ENOEXEC`.
Consumer checks cover inherited and explicit environments, PATH lookup, and
the shell fallback across all seven profiles.

POSIX `fexecve` is implemented in Nelua through Linux `execveat` with
`AT_EMPTY_PATH`. The consumer matrix executes itself through an `O_CLOEXEC`
descriptor and checks that the supplied environment reaches the new image.

GNU `getdtablesize` returns the current soft `RLIMIT_NOFILE` limit through
Nelibc's `getrlimit`, with the Linux `OPEN_MAX` fallback if the query fails.
The consumer matrix compares it with the active resource limit.

The obsolete GNU/BSD `usleep` interface is implemented over Nelibc's
`nanosleep`. It accepts intervals below one second and reports `EINVAL` at or
above one second; the consumer matrix checks both a successful delay and the
invalid boundary.

Linux `getitimer` and `setitimer` are implemented directly over the x86_64
kernel syscalls. GNU/BSD `ualarm` uses `setitimer(ITIMER_REAL)` and returns the
previous timer's remaining microseconds. The consumer matrix checks setting,
reading, and disarming the real-time interval timer.

GNU `ppoll` is implemented over the Linux x86_64 syscall with the kernel's
8-byte signal-mask ABI. It filters Nelibc's reserved cancellation signal and
copies the timeout so the caller's timespec remains unchanged. The consumer
matrix checks readiness on a pipe and timeout preservation.

POSIX `pselect` uses Linux `pselect6` with its kernel signal-mask descriptor.
It applies the same reserved-signal filtering and timeout copy as `ppoll`.
The consumer matrix checks readiness, signal-mask use, and unchanged timeout
across hybrid and standalone profiles.

Linux `timerfd_create`, `timerfd_settime`, and `timerfd_gettime` are implemented
over the x86_64 timerfd syscalls. Standalone headers provide `itimerspec` and
the timerfd flags. The consumer checks nonblocking and close-on-exec creation,
arming, reading the remaining time, and disarming.

Linux `signalfd` creates or updates a descriptor for blocked signals using the
kernel's 8-byte signal-mask ABI. It filters Nelibc's reserved cancellation
signal. The consumer blocks `SIGUSR1`, queues it on the calling thread, reads
its `signalfd_siginfo` record, and restores the original signal mask.

POSIX `timer_create`, `timer_delete`, `timer_getoverrun`, `timer_gettime`, and
`timer_settime` use Linux x86_64 timer syscalls. A small C ABI adapter stores
the kernel timer ID in the selected libc's public `timer_t` representation;
the Nelua wrappers translate it back for each syscall. The standalone headers
provide the Linux x86_64 `sigevent` layout. The consumer covers creation with
`SIGEV_NONE`, arm/read/overrun, and deletion. The kernel supports signal and
no-notification timers; libc-style `SIGEV_THREAD` callback emulation is not
implemented.

The temporary-file family (`mkstemp`, `mkstemps`, `mkostemp`, `mkostemps`, and
`mkdtemp`) is implemented in Nelua for hybrid and standalone profiles. It uses
Linux `getrandom` for names, creates files with mode 0600 and directories with
mode 0700, retries collisions, and restores the `XXXXXX` template if creation
fails. The seven-profile consumer matrix exercises creation, suffix handling,
open flags, permissions, and malformed templates.

Standalone `wcwidth`, `towlower`, `towupper`, and the twelve standard `isw*`
classification functions use generated Unicode 16.0.0 data for the supported
`C` and `C.UTF-8` locales. Run `make check-wcwidth-data` to verify the generated
Nelua tables and case maps;
`python3 tools/gen_wcwidth_data.py` regenerates them when Python provides the
matching Unicode database version. Case conversion uses simple one-codepoint
mappings. Classification follows Unicode letter/number categories, casing,
whitespace, and printability properties, with ASCII-only classes in `C`;
`wctype`/`iswctype`, their `_l` forms, all wide classifier `_l` forms, and
`towlower_l`/`towupper_l` use explicit locale handles. Standalone `newlocale`
constructs static handles for supported combinations of `C`/`C.UTF-8`
`LC_CTYPE`, `C`/`de_DE.UTF-8` `LC_NUMERIC` and `LC_MONETARY`, plus
`C`/`C.UTF-8` `LC_COLLATE`. Standalone `uselocale` switches the calling
thread's ctype, numeric, monetary, and C-style collation state; floating
parsing, formatting, grouping, `strcoll`, `strxfrm`, and `localeconv`
observe the thread selection. `localeconv` results are kept in per-thread
storage. Non-C collation and thread-local time categories remain incomplete.

`scalb` and `scalbf` scale by integral exponents and report invalid fractional
exponents; `significand` and `significandf` normalize finite nonzero inputs so
the absolute result lies in [1, 2), preserving the input sign.
`lgammal` shifts positive inputs to a Stirling expansion with exact rational
correction coefficients; negative inputs use the reflection formula, with a
small-argument path that avoids subnormal sine loss. `tgammal` exponentiates the
logarithmic result. Differential checks use a relative bound of 4,096 binary80
epsilons for this composed calculation; correct rounding is not claimed.
`erfl` uses a power series near zero and `erfcl` uses a continued fraction in
the positive tail; the long-double functions are differentially checked but are
not claimed to be correctly rounded.
`fmal` is implemented with an exact binary80 significand accumulator and a
single final rounding step. Its accumulator uses 1,026 64-bit limbs to cover
the full binary80 product/addend exponent range, so this correctness-focused
software path is not optimized for speed. The x86-64 implementation also
updates x87 and SSE exception flags.
`exp10l` and its GNU-compatible `pow10l` alias reuse `powl` for ordinary and
special values. Near the binary80 underflow boundary they split `log2(10)` and
scale the fractional exponent separately, preserving representable subnormal
results; differential checks cover values from `10^-4930` through the
underflow boundary across the hybrid profiles. This is a targeted range fix,
not a claim of correct rounding for every long-double input.
The `__fpclassify*` and `__signbit*` compatibility symbols classify the
binary32, binary64, and binary80 layouts directly from their representations.

The report uses the same bounded declaration scanner as the project API
inventory and requires the headers and archives from the same musl release.
Review scanner changes against representative prototypes before refreshing the
snapshot.
Standalone pthread implementations, including barriers and read-write locks,
are classified as host-delegated in hybrid profiles because
`include/pthread.h` forwards to the selected host header there.
