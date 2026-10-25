# Core gaps and priorities

This is a working order for making Nelibc a dependable, general-purpose Linux
libc. “Implemented” in the API manifest means a symbol is present in the
archive; it does not certify every behavior required by C or POSIX.

## Priority order

| Priority | Gap | Why it blocks a compelling libc | Next milestone |
|---|---|---|---|
| P0 | Reliable build and consumer gate | Regressions in any compiler/libc profile undermine every replacement | Keep `make smoke-matrix` green; run `make dogfood-matrix` for an application link and verify selected Nelibc symbols are in each executable |
| P1 | C math library | The Nelua slice includes binary32/binary64 math, including `scalb`, `scalbf`, `significand`, and `significandf`, plus x86-64 long-double `sqrtl`, `nanl`, `fabsl`, `copysignl`, `fmal`, `tgammal`, `lgammal`, `lgammal_r`, `erfl`, `erfcl`, `exp10l`, `pow10l`, `fmodl`, `remainderl`, `remquol`, `frexpl`, `modfl`, `hypotl`, `nextafterl`, `nexttowardl`, `atanl`, `atan2l`, `asinl`, `acosl`, `asinhl`, `acoshl`, `atanhl`, `powl`, `fminl`, `fmaxl`, `fdiml`, `cbrtl`, `logl`, `log2l`, `log10l`, `log1pl`, `expl`, `exp2l`, `expm1l`, `sinl`, `cosl`, `sincosl`, `tanl`, `tanhl`, `sinhl`, and `coshl`, plus `ilogbl`, `logbl`, `ldexpl`, `scalbnl`, `scalblnl`, `truncl`, `floorl`, `ceill`, `rintl`, `nearbyintl`, `roundl`, `lrintl`, `llrintl`, `lroundl`, and `llroundl`; internal classification and sign-bit functions now cover binary32/64/80; all standard fenv entry points are implemented for standalone x86-64, while hybrid fenv delegates to the host | Expand long-double math coverage, fenv edge coverage, and math quality/performance |
| P1 | Standalone threading completeness | Standalone pthread remains a subset; several POSIX behaviors and process/signal APIs remain gaps | Continue POSIX pthread behavior coverage and audit process/signal interactions |
| P2 | Locale and character data | Standalone locale data supports C plus UTF-8 case/width/classification data; locale handles currently model only C and C.UTF-8 `LC_CTYPE`, while collation and broad numeric/time conventions remain limited | Add locale category state and thread-local locale selection; expand ctype, collation, numeric, monetary, and time data |
| P2 | API-surface audit | The generated coverage table only sees declarations in current project headers and smoke references are locators, not proofs | Corrected baseline comparison against musl 1.2.5 is recorded in `docs/musl-1.2.5-api-coverage.csv`; classify the 568 undeclared candidates and record explicit intentional omissions |
| P3 | Platform portability | The syscall and startup backend is Linux x86-64-specific | Separate the syscall/ABI layer and add a second architecture before claiming portable standalone support |

## Current evidence

### Practical completeness estimate

These are engineering estimates for a Linux x86-64 general-purpose libc, not
symbol-count ratios or standards-conformance claims:

| Area | Estimated completeness | Main remaining work |
|---|---:|---|
| Core memory, strings, allocation, integer conversion, process/startup, and common I/O | 75–85% | Edge-case conformance, ABI review, and wider consumer coverage |
| Core time and basic signals/process APIs | 65–80% | Cross-libc differential coverage, locale names, and less common POSIX behavior |
| Math | 42–50% | Remaining C math entry points, less-common fenv environment edges, and quality/performance work |
| Standalone threading/TLS | 35–50% | Cancellation, synchronization breadth, and static-TLS ABI limits; dynamic TLS is deferred |
| Locale and international text | 15–30% | Locale datasets, Unicode case conversion, collation, and numeric/time conventions |
| Overall “core libc” for common Linux C programs | 65–75% | Math and threading gaps plus stronger compatibility evidence |

The core score gives extra weight to whether ordinary programs can build,
start, allocate, do I/O, handle strings/numbers, and use threads. Math and
threading are the largest capability gaps; locales and broader portability
matter more for completeness than for the initial common-program target.

### Standalone pthread gap audit

The generated API inventory currently records 112 pthread declarations, and
all 112 export from the Zig, GCC, and TinyCC standalone archives. This verifies
symbol presence, not full behavior. The pinned musl 1.2.5 public-symbol baseline
has 2 pthread entries without project declarations, both internal cleanup
helpers. The GNU concurrency-hint calls are now implemented as standalone
stateful hints; musl 1.2.5 instead accepts only zero and reports `EAGAIN` for
positive values, so hybrid builds intentionally retain host behavior. The
standalone implementation follows glibc's observable behavior, including
`EINVAL` for negative levels:

| Priority | Missing APIs | Why they matter | Scope |
|---|---|---|---|
| Complete | `pthread_getconcurrency`, `pthread_setconcurrency` | Statefully stores the requested concurrency hint; it does not alter scheduling | GNU extension; hybrid mode delegates |

This is a baseline-derived priority list, not a complete POSIX inventory. The
POSIX.1-2024 `pthread_cond_clockwait` API is implemented in standalone mode;
it selects realtime or monotonic time per timed wait and shares the existing
condition-variable futex path. It is not in the pinned musl 1.2.5 baseline,
so hybrid musl continues to expose the host API set. A correct compatibility
implementation needs to select the clock inside musl's condition-wait path;
translating deadlines before calling `pthread_cond_timedwait` would not preserve
the requested clock if realtime changes while a wait is active.
POSIX.1-2024 `pthread_rwlock_clockrdlock` and `pthread_rwlock_clockwrlock` are
also implemented in standalone mode. They choose the futex clock per call and
preserve the existing realtime semantics of `pthread_rwlock_timed*lock`.
Standalone rwlocks use reader preference: new readers may acquire while writers
are queued. Nelibc reports `_SC_PRIORITY_SCHEDULING` and
`_SC_THREAD_PRIORITY_SCHEDULING` as unsupported because it does not implement
the full POSIX scheduling options, including priority-aware rwlock admission.
The standalone priority-ceiling attributes and mutex get/set operations are
implemented. `PTHREAD_PRIO_PROTECT` raises and restores scheduler priority for
held protected mutexes; on Linux, a `SCHED_OTHER` thread cannot acquire a
positive ceiling and receives `EINVAL`. Calls to `pthread_setschedparam` and
`pthread_setschedprio` while holding protected mutexes update the base priority
while preserving the active ceiling; `pthread_getschedparam` reports that base
priority. The smoke test checks nested protection locks: a higher ceiling may
be acquired while a lower ceiling is held, while a lower ceiling must be
rejected once a higher effective priority is active. It attempts this path when
the host permits `SCHED_FIFO` priority changes and skips it when the host denies
real-time scheduling. The protected-priority transition path passed on Fedora
and Bak with real-time scheduling permission. `pthread_tryjoin_np` is implemented in standalone mode;
it returns `EBUSY` while the target is live and reclaims resources after exit.
`pthread_timedjoin_np` accepts an absolute realtime deadline, returns
`ETIMEDOUT` without consuming the joinable thread, and succeeds when the thread
exits before a later deadline.
`pthread_getname_np` and `pthread_setname_np` support the calling thread and
other live threads through their Linux task names; names are limited to 15
bytes, and get-name buffers must hold at least 16 bytes.
`pthread_getaffinity_np` and `pthread_setaffinity_np` operate on live thread IDs
through Linux scheduler affinity syscalls; tests read and reapply the existing
mask for both a worker and the calling thread.
`pthread_getattr_np` reports the current thread's stack mapping or the recorded
usable stack, guard size, detach state, and scheduling attributes for Nelibc
threads. The initial thread's stack mapping is found through `/proc/self/maps`.
`pthread_getattr_default_np` returns the process defaults, and
`pthread_setattr_default_np` updates defaults used by later `pthread_attr_init`
calls and `pthread_create` calls with a null attributes pointer. The standalone
setter follows the Linux manual's replacement behavior; musl 1.2.5 instead
only raises its stack and guard defaults.
The `pthread_kill`, `pthread_getcpuclockid`, and `pthread_setschedprio` are
implemented in standalone mode and delegated to the host in hybrid mode.
`pthread_atfork` is implemented
alongside Nelibc's `fork()` path in hybrid and standalone archives. Also audit
against the current POSIX pthread index, including APIs absent from the pinned
musl export set.

- The regenerated API inventory has 953 declared names. Each of the four
  hybrid archives exports 785; 161 APIs are marked as host-delegated and 7
  have declared-but-unexported hybrid gaps. The manifest also contains one
  implementation-only entry. This is an inventory signal, not a completeness
  percentage. See
  `build/api-coverage.csv` after `make api-coverage`.
- The corrected musl 1.2.5 public-symbol comparison finds 1,471 declarations
  that are also exported by the pinned release archives: Nelibc exports 741
  in at least one hybrid profile, delegates 162 to a hybrid host, and does not
  yet declare 568. No declared API is missing from all four hybrid archives.
  The largest undeclared groups are wide characters (42), stdlib (33), network
  database APIs (32), and unistd (29). These
  are gap counts, not completion percentages; no API has yet been declared intentionally out
  of scope. See `docs/api-coverage.md` for the pinned source and regeneration
  command.
- Real-application dogfood passed on 2026-10-24: the Spinel C compiler was
  rebuilt with Zig cc and the glibc hybrid archive, then compiled and ran a
  Ruby client linked against that archive. The same client also ran as a static
  x86-64 musl executable linked against the Zig/musl hybrid archive. Both
  executables contain Nelibc's `memcpy`, `strlen`, and `strtol`. TinyCC also
  compiled the generated client, which Zig linked with Nelibc's TinyCC/glibc
  archive and TinyCC runtime. Reproduction steps are in `docs/quickstart.md`.
- Differential tests cover meaningful memory, string, integer, floating
  conversion, wide-string, tokenizer, collation, and syscall behavior across
  glibc, musl, GCC, Zig cc, and TinyCC profiles.
- `wcpcpy`, `wcpncpy`, and the GNU `wcswcs` alias are now implemented and
  declared. Their bounded-copy return values, terminator padding, and search
  result match the host in 256 wide-string differential cases per profile;
  consumer smoke tests also call all three in the four hybrid profiles and
  standalone Zig, GCC, and TinyCC builds.
- `wcscasecmp` and `wcsncasecmp` are implemented in Nelua and call `towlower`
  for locale-sensitive folding. Hybrid builds therefore follow glibc or musl;
  standalone supplies Unicode 16 simple `towlower` and `towupper` mappings for
  `C.UTF-8`, and ASCII mappings for `C`. ASCII comparison and bounded-prefix
  behavior pass the differential matrix and all seven consumer profiles;
  Unicode mapping smoke checks cover Greek, Cyrillic, Deseret, and polytonic
  Greek. The twelve standard `isw*` classifiers use generated Unicode letter,
  number, case, whitespace, and general-category data in `C.UTF-8`, with
  ASCII classifications in `C`. `wctype`/`iswctype`, all classifier `_l`
  forms, and `towupper_l`/`towlower_l` use locale handles. Standalone
  `newlocale` and `duplocale` return static handles for `C` and `C.UTF-8`;
  `freelocale` is a no-op for those handles. Other locale categories and
  thread-local `uselocale` remain unimplemented.
- `wcswidth` is implemented in Nelua and uses the selected host's `wcwidth` in
  hybrid mode. The standalone locale backend uses generated Unicode 16.0
  general-category, East Asian Width, and Default_Ignorable_Code_Point data
  for `C` and `C.UTF-8`; ambiguous characters have width one. Check the
  generated table with `make check-wcwidth-data`; regeneration requires Python
  with Unicode database version 16.0.0. `wcwidth` remains host-delegated in
  hybrid mode, where it follows the selected libc's locale data.
- Time APIs are already implemented across `src/unistd.nelua` and
  `src/stdlib.nelua`: clocks and sleeps, UTC/local conversion, `mktime`,
  `timegm`, `strftime`, `strptime`, and the `asctime`/`ctime` family. The
  syscall smoke checks UTC and negative epochs, leap days, POSIX TZ rules,
  TZif zone files, DST transitions, formatting bounds, and CPU clocks. Broader
  locale-specific time names and cross-libc differential coverage remain open.
- The current math slice in `src/math_core.nelua` exports 138 float and double
  functions plus x86-64 extended-precision `sqrtl`, `nanl`, `fabsl`, `copysignl`,
  `fmal`, `tgammal`, `lgammal`, `lgammal_r`, `erfl`, `erfcl`, `exp10l`, `pow10l`, `fmodl`, `remainderl`, `frexpl`, `modfl`, `hypotl`, `nextafterl`, `nexttowardl`, `atanl`,
  `atan2l`, `asinl`, `acosl`, `logl`, `log2l`, `log10l`, `log1pl`, `expl`,
  `exp2l`, `expm1l`, `sinl`, `cosl`, `sincosl`, `tanl`, `tanhl`, `sinhl`,
  `coshl`, `asinhl`, `acoshl`, `atanhl`, `powl`, `fminl`, `fmaxl`, `fdiml`,
  `cbrtl`, `remquol`, `ilogbl`, `logbl`, `ldexpl`, `scalbnl`, `scalblnl`,
  `truncl`, `floorl`, `ceill`, `rintl`, and
  `nearbyintl`, `roundl`, `lrintl`, `llrintl`, `lroundl`, and `llroundl`, plus
  binary32/64/80 internal classification and sign-bit functions, through all
  four hybrid profiles and standalone archives. Its
  complex-math coverage now includes `cabs*`, `creal*`, `cimag*`, `conj*`,
  `cproj*`, `carg*`, `csqrt*`, `cexp*`, `clog*`, `csin*`, `ccos*`, `csinh*`,
  `ccosh*`, `ctan*`, `ctanh*`, `catan*`, `catanh*`, `casin*`, `cacos*`,
  `casinh*`, `cacosh*`, and `cpow*`: a small C ABI shim extracts components and
  packages complex returns, while Nelua computes
  magnitudes, conjugates and projects values, evaluates arguments with
  `atan2*`, computes principal square roots with scaled intermediates, composes
  complex exponentials from the Nelua real `exp*`, `sin*`, and `cos*`, uses a
  scaled logarithmic modulus for complex logarithms, and composes complex
  circular and hyperbolic sine and cosine from the corresponding real functions.
  Complex tangent functions use scaled exponentials for large inputs. Inverse
  tangent uses scaled quadratic expressions, while inverse sine and cosine use
  scaled distances to their branch points. The inverse hyperbolic families reuse
  those kernels through principal-branch identities. Complex power composes
  Nelibc's complex logarithm and exponential kernels.
  All complex-math names in the pinned musl baseline now have declarations and
  exports; GNU extensions and broader behavioral conformance still need audit.
  Nelua functions implement `fmin`/`fmax` NaN behavior and rounding rules;
  x86-64 hardware instructions implement correctly rounded square root and
  binary remainder. `remquo`, `remquof`, and `remquol` reuse x87 `FPREM1` and return the signed low quotient bits with the remainder. `drem` is the GNU alias for `remainder`; `pow10` aliases `exp10`; `nan` constructors return quiet NaNs and `finite` uses bit-level classification. Nelua `asin` and `acos` use the stable endpoint product
  `(1-x)*(1+x)` and `atan2`; the shootout checks domain behavior and compares
  100,000 values in `[-1, 1]` within four ULPs on each profile. The Nelua
  `atanh` evaluates the endpoint region through `log1p` and preserves odd
  symmetry; the four-profile differential matrix checks 32 adjacent binary32
  and binary64 inputs approaching both poles. These checks found no result
  beyond eight ULPs. Bit-level Nelua implementations also provide `frexp` and
  `modf`, including subnormal decomposition and signed-zero behavior. x86-64
  `FSCALE` primitives implement `ldexp`/`scalbn`; overflow and zero results
  from underflow set `ERANGE`. It sets `EDOM` for negative square root and
  invalid remainder domains. Standalone x86-64 implements `feclearexcept`,
  `fegetexceptflag`, `feraiseexcept`, `fesetexceptflag`, `fetestexcept`,
  `fegetround`, `fesetround`, `fegetenv`, `feholdexcept`, `fesetenv`, and
  `feupdateenv` with x87/SSE state synchronization. Hybrid profiles retain the
  host fenv ABI and implementation. Standalone smoke tests verify SIGFPE
  delivery for all five standard exceptions through x87 and SSE trap paths;
  less common environment edge cases still need validation. Bit-level
  `ilogb`/`logb` cover normal/subnormal decomposition;
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
  `j0`/`j1` and `y0`/`y1` use Nelua power series below 12 and optimally truncated Hankel expansions above it; float APIs widen to double. Four-profile comparisons cover zero behavior, domain edges, roots, large arguments, and 100,000 generated inputs, gated at 2e-9 relative error for double and 2e-6 for float. `tgamma` and `lgamma` use the musl Lanczos rational coefficients with reflection for negative nonintegers; `lgamma_r`/`lgammaf_r` return the gamma sign, and `lgamma`/`lgammaf` update `signgam`. Differential checks span edge cases and 100,000 generated inputs, with relative-error gates of 2e-13 for double gamma, 2e-12 for double log-gamma, and 2e-5 for float variants. The GNU `sincos` pair API shares argument reduction and evaluates both kernels once; `sincosf` widens to double. Integer-order Bessel `jn`/`yn` now use forward recurrence for oscillatory inputs and a continued-fraction/backward recurrence for large orders; float APIs widen to double. Their four-profile differential matrix covers order sign, negative inputs, zero/pole behavior, transition regions, large arguments, and generated inputs. `fma` uses a portable 128-bit integer-product fallback to preserve fused rounding; `fmaf` uses a double-precision product/sum with a halfway correction. Exact-bit differential checks cover edge cases and 100,000 generated triples across compiler/libc profiles. `cbrt` reduces the binary exponent modulo three, seeds from the Nelua
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
  retained exception flags. `lrint`/`lrintf` and `llrint`/`llrintf` use SSE conversions that honor MXCSR rounding and report inexact; `lround`/`lroundf` and their long-long forms round halfway cases away from zero without raising inexact. Their matrix checks all four modes, integer results, exception behavior, and ties. Signaling-NaN `FE_INVALID` reporting varies among
  host libcs, so cross-libc comparisons require a NaN result but permit that
  one flag to differ. The broader core harness also checks signed zero, subnormals, half-way rounding,
  infinities, quiet NaNs, and largest finite values, plus 100,000 generated
  scalar inputs and input pairs per profile against host libm. Signaling-NaN
  operations are exercised, with min/max result comparison relaxed where host
  runtimes differ. Remaining long-double entry points and less-common fenv trap
  environment edge cases remain outside this slice. `sinh`/`cosh`/`tanh` and their float forms use small-input series,
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
  is bounded to 32 entries per thread. Internal allocator critical sections,
  the pthread-key and default-attribute locks, robust-list slot allocation,
  priority-protection bookkeeping, and the named-semaphore registry defer
  asynchronous cancellation until after releasing their locks, then retrigger
  a pending request. Blocking calls made while those locks are held remain a
  target for focused cancellation testing.
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
