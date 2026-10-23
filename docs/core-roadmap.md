# Core gaps and priorities

This is a working order for making Nelibc a dependable, general-purpose Linux
libc. “Implemented” in the API manifest means a symbol is present in the
archive; it does not certify every behavior required by C or POSIX.

## Priority order

| Priority | Gap | Why it blocks a compelling libc | Next milestone |
|---|---|---|---|
| P0 | Reliable build and consumer gate | Regressions in any compiler/libc profile undermine every replacement | Keep `make smoke-matrix` green; run `make dogfood-matrix` for an application link and verify selected Nelibc symbols are in each executable |
| P1 | C math library | The Nelua slice includes binary32/binary64 `sqrt`, `cbrt`, `fmod`, `remainder`, `fmin`, `fmax`, `fdim`, `frexp`, `modf`, `ldexp`, `scalbn`, `scalbln`, `ilogb`, `logb`, `nextafter`, `nexttoward`, `fabs`, `copysign`, `trunc`, `floor`, `ceil`, `round`, `rint`, `nearbyint`, `exp`, `log`, `pow`, `hypot`, `sin`, `cos`, `tan`, `sinh`, `cosh`, `tanh`, `asinh`, `acosh`, `atanh`, `atan`, and `atan2`; the arithmetic backend is x86-64-specific, and most math entry points remain open | Expand the math set; then long-double coverage and complete fenv semantics |
| P1 | Standalone threading completeness | Standalone pthread remains a subset; `PTHREAD_PRIO_PROTECT` and several process/signal APIs are unsupported | Add CPU-clock and priority-ceiling APIs, then cover remaining POSIX gaps |
| P2 | Locale and character data | Standalone locale data is small (C plus selected test locales), while case folding is ASCII-only; hybrid mode often uses host locale data | Define supported locale data sources and implement locale-aware ctype, collation, numeric, monetary, and time behavior consistently |
| P2 | API-surface audit | The generated coverage table only sees declarations in current project headers and smoke references are locators, not proofs | Baseline comparison against musl 1.2.5 is recorded in `docs/musl-1.2.5-api-coverage.csv`; classify the 753 undeclared candidates and record explicit intentional omissions |
| P3 | Platform portability | The syscall and startup backend is Linux x86-64-specific | Separate the syscall/ABI layer and add a second architecture before claiming portable standalone support |

## Current evidence

### Practical completeness estimate

These are engineering estimates for a Linux x86-64 general-purpose libc, not
symbol-count ratios or standards-conformance claims:

| Area | Estimated completeness | Main remaining work |
|---|---:|---|
| Core memory, strings, allocation, integer conversion, process/startup, and common I/O | 75–85% | Edge-case conformance, ABI review, and wider consumer coverage |
| Core time and basic signals/process APIs | 65–80% | Cross-libc differential coverage, locale names, and less common POSIX behavior |
| Math | 31–38% | Most C math entry points, long double, fenv behavior, and quality/performance work |
| Standalone threading/TLS | 35–50% | Cancellation, synchronization breadth, and static-TLS ABI limits; dynamic TLS is deferred |
| Locale and international text | 15–30% | Locale datasets, Unicode case conversion, collation, and numeric/time conventions |
| Overall “core libc” for common Linux C programs | 65–75% | Math and threading gaps plus stronger compatibility evidence |

The core score gives extra weight to whether ordinary programs can build,
start, allocate, do I/O, handle strings/numbers, and use threads. Math and
threading are the largest capability gaps; locales and broader portability
matter more for completeness than for the initial common-program target.

### Standalone pthread gap audit

The generated API inventory currently records 93 pthread declarations, and all
93 export from the Zig, GCC, and TinyCC standalone archives. This verifies
symbol presence, not full behavior. The pinned musl 1.2.5 public-symbol baseline
adds 14 pthread candidates that Nelibc does not yet declare. Prioritize them by
portability impact:

| Priority | Missing APIs | Why they matter | Scope |
|---|---|---|---|
| P2 | `pthread_mutex_getprioceiling`, `pthread_mutex_setprioceiling` | Completes the priority-ceiling path; `PTHREAD_PRIO_PROTECT` currently returns `ENOTSUP` | Optional real-time priority protocol |
| P3 | `pthread_getaffinity_np`, `pthread_setaffinity_np`, `pthread_getname_np`, `pthread_setname_np`, `pthread_getattr_np`, `pthread_getattr_default_np`, `pthread_setattr_default_np`, `pthread_getconcurrency`, `pthread_setconcurrency`, `pthread_timedjoin_np`, `pthread_tryjoin_np` | Adds Linux/glibc and musl nonstandard thread controls and convenience calls | Implementation-specific extensions |

This is a baseline-derived priority list, not a complete POSIX inventory. The
`pthread_kill`, `pthread_getcpuclockid`, and `pthread_setschedprio` are
implemented in standalone mode and delegated to the host in hybrid mode.
`pthread_atfork` is implemented
alongside Nelibc's `fork()` path in hybrid and standalone archives. Also audit
against the current POSIX pthread index, including APIs absent from the pinned
musl export set.

- The current API inventory has 669 declared names: 566 exported in hybrid
  archives, 103 intentionally host-delegated across the hybrid profiles, and one
  implementation-only entry. This is an inventory signal, not a completeness
  percentage.
- The musl 1.2.5 public-symbol comparison finds 1,269 declarations that are
  also exported by the pinned release archives: Nelibc exports 426 in at least
  one hybrid profile, delegates 91 to a hybrid host, and does not yet declare
  752. The largest undeclared groups are pthread, complex math, wide
  characters, stdlib, unistd, and network database APIs. These are gap counts,
  not completion percentages; no API has yet been declared intentionally out
  of scope. See `docs/api-coverage.md` for the pinned source and regeneration
  command.
- Differential tests cover meaningful memory, string, integer, floating
  conversion, wide-string, tokenizer, collation, and syscall behavior across
  glibc, musl, GCC, Zig cc, and TinyCC profiles.
- Time APIs are already implemented across `src/unistd.nelua` and
  `src/stdlib.nelua`: clocks and sleeps, UTC/local conversion, `mktime`,
  `timegm`, `strftime`, `strptime`, and the `asctime`/`ctime` family. The
  syscall smoke checks UTC and negative epochs, leap days, POSIX TZ rules,
  TZif zone files, DST transitions, formatting bounds, and CPU clocks. Broader
  locale-specific time names and cross-libc differential coverage remain open.
- The current math slice in `src/math_core.nelua` exports 90 float and double
  functions through all four hybrid profiles and standalone archives. Its
  Nelua functions implement `fmin`/`fmax` NaN behavior and rounding rules;
  x86-64 hardware instructions implement correctly rounded square root and
  binary remainder. Bit-level Nelua implementations also provide `frexp` and
  `modf`, including subnormal decomposition and signed-zero behavior. x86-64
  `FSCALE` primitives implement `ldexp`/`scalbn`; overflow and zero results
  from underflow set `ERANGE`. It sets `EDOM` for negative square root and
  invalid remainder domains; floating-point exception/fenv reporting remains
  incomplete. Bit-level `ilogb`/`logb` cover normal/subnormal decomposition;
  `nextafter` advances binary32/binary64 encodings directly and handles signed
  zero and NaN values. `nexttoward` compares its long-double direction before
  narrowing, including when the direction lies between adjacent float or
  double values. Signaling-NaN and zero exception flags remain
  incomplete. Its
  Nelua `exp`/`expf` and `log`/`logf` use a range-reduced degree-20 exponential
  polynomial and an atanh logarithm series with split ln(2) constants. The
  differential run over edge values and 100,000 generated inputs reports a
  maximum observed error of 1 ULP for exp, 2 ULP for log, and 1 ULP for both
  float variants on the current host; the cross-libc matrix checks the same
  four-ULP bound on each profile. Range errno and overflow/underflow/pole/domain
  exception flags are also checked.
  `cbrt` reduces the binary exponent modulo three, seeds from the Nelua
  exponential/logarithm cores, then applies Newton refinement. Across the
  current edge set and 100,000 generated inputs per profile, observed error is
  at most 4 ULP for binary64 and 0 ULP for binary32; this is differential
  coverage, not a proof of correct rounding.
  `exp2` splits the input into an integer binary scale and a fractional
  remainder, evaluates the latter with a degree-20 series and split ln(2), and
  restores scale with `ldexp`; it passes the same four-profile 4-ULP
  differential gate over edge values and 100,000 generated inputs.
  `log2` extracts the binary exponent and evaluates the normalized mantissa
  with an atanh series; exact powers of two return their integer exponent.
  Differential checks compare normal inputs within four ULP. For binary64
  subnormal inputs, the harness also checks a finite in-range result because
  Zig's bundled compiler math returns inaccurate reference values there;
  GCC/glibc comparisons cover those inputs directly. The same bundled Zig
  reference issue affects `log10` subnormals; the harness checks result range
  there and uses full ULP comparisons with GCC/glibc. Targeted `log10` edges
  cover zero, true-min and min-normal inputs, powers of ten and their adjacent
  representable values, large magnitudes, and `DBL_MAX`; zero and negative
  inputs also check `ERANGE` and `EDOM`.
  `log1p` uses a compensated `log(1+x)` path for larger inputs and an atanh
  series for small values; `expm1` uses a Taylor series around zero and the
  exponential core for larger magnitudes. Float variants widen to binary64
  and narrow. Differential checks cover domain edges and generated values
  within four ULP. `log10` uses the natural-log core and log10(e). The GNU
  `exp10` extension splits its input into an integer decimal scale and a
  fractional remainder, then uses powers-of-ten constants and the exponential
  core; observed differential error stays within eight ULP across the matrix.
  `hypot`/`hypotf` scale by the larger input before squaring, prioritize infinity
  over NaN, and avoid underflow from insignificant operand ratios. Pairwise
  differential checks cover edge values and generated inputs.
  The shared `sin`/`cos` reducer multiplies the full significand by 1,200 fixed
  bits of 2/pi, then reconstructs a two-part remainder with split pi/2. Kernel
  polynomials and full-range argument reduction stay within 1 ULP on the tested
  edge values and 100,000 generated inputs for each profile.
  `rint`/`rintf` observe the current SSE rounding mode and report inexact;
  `nearbyint`/`nearbyintf` suppress only inexact while retaining prior flags.
  The differential harness checks those APIs under all four standard rounding
  modes, including half-way values, subnormals, infinities, signed zero, and
  retained exception flags. Signaling-NaN `FE_INVALID` reporting varies among
  host libcs, so cross-libc comparisons require a NaN result but permit that
  one flag to differ. The broader core harness also checks signed zero, subnormals, half-way rounding,
  infinities, quiet NaNs, and largest finite values, plus 100,000 generated
  scalar inputs and input pairs per profile against host libm. Signaling-NaN
  operations are exercised, with min/max result comparison relaxed where host
  runtimes differ. Long double and most transcendental functions remain outside
  this slice. `sinh`/`cosh`/`tanh` and their float forms use small-input series,
  range-safe exponential formulas, signed-zero and infinity handling, and
  overflow `ERANGE`; differential checks cover the near-overflow transition.
  `atan` uses fdlibm range reduction and its polynomial kernel; `atan2` handles
  signed zero and infinite quadrants before using bounded ratios. Their float
  variants widen to binary64, compute, and narrow. Differential checks cover
  edge values and generated inputs within four ULP.
  `pow` handles zero, infinite, NaN, negative-base integer parity, domain,
  overflow, and underflow cases explicitly. Integer exponents through magnitude
  64 use exponentiation by squaring; the general path evaluates log and exp in
  x86 extended precision before rounding to binary64. Its current differential
  sample reports a maximum of 1 ULP for both float and double over edge and
  100,000 generated input pairs on the four hybrid profiles.
  `asinh` preserves tiny inputs and uses a stable `log1p` form for moderate
  magnitudes plus a large-input logarithmic form;
  `acosh` handles its domain and large arguments explicitly; `atanh` uses a
  small-input series and reports its domain and pole errors. Their float forms
  widen to binary64 then narrow. Differential tests include edge and 100,000
  generated inputs per profile with an eight-ULP acceptance bound; this is
  validation evidence, not a correctly-rounded guarantee.
- Standalone static `-nostdlib` smoke paths exist for Zig, GCC, and TinyCC,
  including startup, TLS fixtures, allocation, stdio, and a pthread subset.
- Standalone pthread now supports deferred and asynchronous cancellation
  requests, cancellation state enable/disable, `pthread_setcanceltype`,
  `pthread_testcancel`, cleanup-handler push/pop for
  created threads, and cancellation of a thread blocked in `pthread_cond_wait`
  or `pthread_cond_timedwait`. Condition variables select realtime or monotonic
  clocks through their attributes. Mutexes support absolute realtime timed locks,
  Linux priority-inheritance futexes, and robust owner-death recovery through
  Linux robust lists. The smoke covers robust PI recovery both with and without
  a blocked waiter, PI timed-lock deadlines, and deferred cancellation while a
  PI waiter is blocked. Forked processes
  refresh their robust-list registration; process-shared robust recovery is
  exercised by a child that exits while holding the mutex. Spin locks, barriers,
  and read-write locks support process sharing. Reusable
  barriers use futex generations. Read-write locks support recursive readers,
  process sharing, try-locks, and absolute realtime timed locks; the reader-
  preference policy can starve writers under a continuous reader stream. It
  also interrupts blocking calls routed through the cancellable syscall
  wrappers, including read/write, socket I/O,
  poll/select, waits, sleeps, openat, and fsync. Condition-wait cancellation
  reacquires the mutex, then unwinds cleanup handlers before thread-specific
  data destructors. In deferred mode, ordinary CPU-bound code must call
  `pthread_testcancel`.
  Signal 32 is reserved internally and cannot be installed or blocked by
  applications. Asynchronous cancellation requires application code to stay
  within async-cancel-safe operations. The initial thread has its own pthread
  context; cancellation state, cleanup handlers, `pthread_exit`, and thread-
  specific destructors use the same path as created threads. The cleanup stack
  is bounded to 32 entries per thread.
- Standalone pthread attributes support configurable stack sizes, page-
  protected guard regions, and scheduler inheritance/policy/priority. Explicit
  scheduling is applied before a startup gate releases the worker; permission
  failures are returned from `pthread_create` without running the routine.
  `pthread_attr_getguardsize` reports the requested byte size, while stack
  mappings round the guard region up to whole pages. `pthread_getschedparam` and
  `pthread_setschedparam` query and change live Linux thread scheduling.
- Standalone unnamed POSIX semaphores now provide `sem_init`, `sem_destroy`,
  `sem_wait`, `sem_timedwait`, `sem_trywait`, `sem_post`, and `sem_getvalue`.
  They use Nelua-owned futex waits, including cancellation at blocking waits.
- Standalone named POSIX semaphores provide `sem_open`, `sem_close`, and
  `sem_unlink` through shared `/dev/shm` mappings and a small C varargs ABI
  bridge; initialization, mapping, wait, and lifetime tracking are Nelua-owned.
  The smoke covers eight concurrent creators, cross-process opens/posts, and
  unlinking while mappings remain open; 20 repeated Zig runs passed.
- Semaphore edge cases now follow POSIX immediate-acquisition rules for
  `sem_timedwait`, accept any nonzero `pshared`, and ignore `sem_open`'s mode
  and initial value when opening an existing name. `sem_clockwait` supports
  `CLOCK_REALTIME` and `CLOCK_MONOTONIC` using absolute futex deadlines.
  Hybrid glibc builds fast-path immediate acquisition before delegating blocked
  waits to glibc, preserving the POSIX invalid-deadline rule. Hybrid musl builds
  provide the API through musl's public semaphore calls; monotonic waits poll at
  1 ms intervals to avoid depending on musl's private `sem_t` layout. The musl
  smoke verifies expiration, wakeup, and cancellation while blocked.
- Hybrid stdio adapts to host `FILE` implementations in several profiles. The
  standalone `FILE` implementation is Nelua-owned, but full standards and
  edge-case coverage remains a separate conformance task.
- Standalone startup reads the executable's `PT_TLS` image and initializes
  static TLS. New pthreads receive a private copy of that static image. This
  supports the local-exec TLS model used by current fixtures; it is runtime
  support in assembly/Nelua, not a Nelua compiler TLS implementation. Dynamic
  TLS modules and a DTV are not implemented, and are deferred until a target
  application needs them. Standalone pthread supports deferred cooperative
  cancellation at explicit cancellation points and condition waits, with
  cleanup handlers on created threads. Asynchronous cancellation and full
  POSIX synchronization coverage remain incomplete.
- The raw syscall and process-startup implementation targets Linux x86-64.

## Completion rule

Do not use a symbol count as a completion score. For each API, record whether
Nelibc implements it, intentionally delegates it, or has not implemented it;
then attach behavior tests, ABI checks, and the compiler/libc profiles that
passed. A family is ready to replace only when its tests run without relying
on the host implementation for the behavior under test.
