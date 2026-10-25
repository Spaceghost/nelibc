# Core gaps and priorities

This is a working order for making Nelibc a dependable, general-purpose Linux
libc. “Implemented” in the API manifest means a symbol is present in the
archive; it does not certify every behavior required by C or POSIX.

## Priority order

| Priority | Gap | Why it blocks a compelling libc | Next milestone |
|---|---|---|---|
| P0 | Reliable build and consumer gate | Regressions in any compiler/libc profile undermine every replacement | Keep the seven-profile `make smoke-matrix` and four-profile `make dogfood-matrix` green as the source changes |
| P1 | C math library | The Nelua slice includes binary32/binary64 math, including `scalb`, `scalbf`, `significand`, and `significandf`, plus x86-64 long-double math, internal binary32/64/80 classification, and all standard standalone x86-64 fenv entry points; hybrid fenv delegates to the host | Expand long-double edge coverage, fenv environment edges, and math quality/performance |
| P1 | Standalone threading completeness | Standalone pthread remains a subset; several POSIX behaviors and process/signal APIs remain gaps | Continue POSIX pthread behavior coverage and audit process/signal interactions |
| P2 | Locale and character data | Standalone locale data supports C plus UTF-8 case/width/classification data; locale handles currently model only C and C.UTF-8 `LC_CTYPE`, while collation and broad numeric/time conventions remain limited | Add locale category state and thread-local locale selection; expand ctype, collation, numeric, monetary, and time data |
| P2 | API-surface audit | The generated coverage table only sees declarations in current project headers and smoke references are locators, not proofs | Corrected baseline comparison against musl 1.2.5 is recorded in `docs/musl-1.2.5-api-coverage.csv`; classify the 555 undeclared candidates and record explicit intentional omissions |
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

The generated API inventory currently records 115 pthread declarations, and
all 115 export from the Zig, GCC, and TinyCC standalone archives. The pthread
smoke passed with all three compilers; the Zig run passed 20 consecutive times.
This verifies symbol presence and the tested behavior, not full POSIX
conformance. The pinned musl 1.2.5 public-symbol baseline
has 2 pthread entries without project declarations, both internal cleanup
helpers. The GNU concurrency-hint calls are now implemented as standalone
stateful hints; musl 1.2.5 instead accepts only zero and reports `EAGAIN` for
positive values, so hybrid builds intentionally retain host behavior. The
standalone implementation follows glibc's observable behavior, including
`EINVAL` for negative levels:

The Unix cancellation smoke covers blocked `poll`, `ppoll`, `select`, and
`pselect`, plus socket `send`, `sendmsg`, `recv`, `recvfrom`, and `recvmsg`, in
addition to blocking scalar and vectored (`readv`, `writev`) I/O and process
calls. Both `nanosleep` and `clock_nanosleep`, plus the Linux `epoll_wait`,
`epoll_pwait`, and `epoll_pwait2` extensions, are tested as well. These cases
run in the hybrid glibc, musl, and TinyCC-compiled profiles and the standalone
Zig, GCC, and TinyCC profiles; `make smoke-matrix` includes the standalone
group. `getitimer` and `setitimer` use raw syscalls because POSIX does not list
them as cancellation points; this avoids introducing cancellation there.
`fcntl(F_SETLKW)` enters the cancellation path, and the smoke verifies that a
blocked record-lock request can be canceled while another process holds the
lock.
`waitid` is implemented over Linux `waitid` syscall 247, exposed through
`sys/wait.h`, and delegated to glibc's cancellation-aware entry point in hybrid
glibc builds. Smoke coverage checks both `siginfo_t` child-exit fields and
cancellation of a blocked `P_PID` wait. `sigwait`, `sigwaitinfo`,
`sigtimedwait`, and `sigsuspend` now use cancellation-aware kernel signal waits;
the smoke consumes a queued `SIGUSR1` through `sigwait` and cancels each blocked
wait form across all seven profiles.
`readlink` and `readlinkat` are also cancellation points; a self-canceling
worker verifies that a pending deferred request is acted on at `readlink`
across all seven profiles, without depending on filesystem latency.
`msync` is implemented over Linux syscall 26 as a cancellation point; the
same deterministic pending-cancellation check covers it in all seven profiles.
`tcdrain` uses Linux `ioctl(TCSBRK, 1)`, matching glibc's implementation, and
the smoke checks `EBADF` propagation and pending cancellation.
The message-queue slice implements `mq_open`, `mq_close`, `mq_unlink`,
`mq_getattr`, `mq_setattr`, `mq_send`, `mq_receive`, `mq_timedsend`, and
`mq_timedreceive`, plus kernel-backed `mq_notify`; `mq_open`'s variadic
argument handling is a small C ABI adapter, while the operations use Nelua
Linux syscall wrappers. `SIGEV_THREAD` notification starts a detached callback
thread and services the Linux netlink notification. The smoke checks queue
attributes, send/receive bytes and priority, an expired receive timeout,
`SIGEV_NONE` registration/removal, `SIGEV_SIGNAL` delivery, a callback using
caller-supplied thread attributes, and cancellation of blocked send and receive
calls. The current run passes all seven profiles: Zig/glibc, GCC/glibc,
Zig/musl, TinyCC/glibc, and standalone Zig, GCC, and TinyCC. The TinyCC
compiler used for the shootout was version 0.9.28rc (`e23dabe`).
The checks verify that deferred cancellation wakes each blocked thread and
that `pthread_join` reports `PTHREAD_CANCELED`. This remains targeted behavior
coverage, not a complete cancellation-point audit.

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
`pthread_mutex_clocklock` also selects realtime or monotonic deadlines inside
the mutex futex path for normal, robust, and priority-inheritance mutexes;
realtime PI timed locks retain the existing `FUTEX_LOCK_PI` path, while
monotonic PI waits use `FUTEX_LOCK_PI2`. The standalone smoke test checks
expired monotonic waits for normal, robust, PI, and combined robust+PI mutexes,
plus invalid clock and timespec inputs. All three standalone compiler profiles
passed on 2026-10-25; the Zig smoke passed 20 consecutive runs.
Standalone rwlocks use reader preference: new readers may acquire while writers
are queued by default. The GNU `pthread_rwlockattr_setkind_np` extension now
supports reader preference, the glibc-compatible ignored writer-preference
setting, and nonrecursive writer preference; the latter blocks new readers
while a writer is queued. The smoke verifies both writer-preference modes
against the default. This extension follows the behavior documented by the
[Linux pthread rwlock attribute manual](https://man7.org/linux/man-pages/man3/pthread_rwlockattr_setkind_np.3.html).
Process priority scheduling is now exposed: `_SC_PRIORITY_SCHEDULING` returns
200809, with Nelua wrappers for scheduler policy/parameter calls, policy
priority ranges, and the round-robin interval query. The standalone smoke
checks the wrappers and their error paths.

`_SC_THREAD_PRIORITY_SCHEDULING` now returns 200809. Per-priority atomic
reader and writer counts live in each rwlock; per-thread wait records let
`pthread_setschedparam` and `pthread_setschedprio` reprioritize queued waiters.
When a lock becomes available, a higher-priority blocked reader prevents a
lower-priority writer from overtaking it, while equal-priority writer/read
ties favor the writer. An aggregate real-time waiter count avoids scanning
priority buckets on ordinary lock operations. Shared rwlock state supports
process-shared locks without pointers to another process's thread context.
Timeout and asynchronous-cancellation paths remove reader and writer counts.
The real-time-gated smoke covers FIFO and RR priority changes, higher-priority
reader versus lower-priority writer admission, equal-priority reader/writer
precedence, process-shared writer ordering across a fork, and timeout and
cancellation cleanup. Zig and GCC passed 10 repeat runs each on Bak. The GNU
nonrecursive writer-preference attribute remains a separate policy.

Per-thread read ownership is tracked in standalone mode using four inline lock
slots plus dynamically allocated overflow entries. Recursive reads are
counted, and a thread that did not acquire a read lock gets `EPERM` when it
tries to unlock one. The Zig and GCC standalone pthread smoke profiles pass
this ownership test.
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

- The regenerated API inventory has 974 declared names. All four hybrid
  archives (Zig/glibc, GCC/glibc, Zig/musl, and TinyCC/glibc) export 803;
  all three standalone archives export 975 including one implementation-only
  entry. There are 161 host-delegated APIs and 10 declared-but-unexported
  Zig/musl archive gaps.
  `pthread_mutex_clocklock` is provided by the glibc host in hybrid mode and
  exported by all three standalone archives. This is an inventory signal, not
  a completeness percentage. See `build/api-coverage.csv` after
  `make api-coverage`.
- The corrected musl 1.2.5 public-symbol comparison finds 1,471 declarations
  that are also exported by the pinned release archives: Nelibc exports 754
  in at least one tested hybrid profile, delegates 162 to a hybrid host, and does not
  yet declare 555. No declared API is missing from the tested hybrid archives.
  The largest undeclared groups are wide characters (42), stdlib (32), network
  database APIs (32), and unistd (29). These
  are gap counts, not completion percentages; no API has yet been declared intentionally out
  of scope. See `docs/api-coverage.md` for the pinned source and regeneration
  command.
- `make smoke-matrix` passed on 2026-10-25 across Zig/glibc, GCC/glibc,
  Zig/musl, TinyCC/glibc, and standalone Zig, GCC, and TinyCC. This runs the
  ABI, libc-differential, syscall, thread, and math smoke suites; the binary
  math comparison checks 116 functions against 100,000 generated inputs.
- `make dogfood-matrix` passed on 2026-10-25 for all four hybrid profiles.
  Each incremental C consumer ran and the gate confirmed that the executable
  contains selected Nelibc symbols. The larger Spinel dogfood results below
  remain separate evidence.
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
  four hybrid profiles and standalone archives. The `nextafterl` edge checks
  now verify `FE_UNDERFLOW | FE_INEXACT` when stepping
  from zero to the least subnormal and `FE_OVERFLOW | FE_INEXACT` when stepping
  from the largest finite value to infinity. Crossing from the least normal to
  its subnormal predecessor raises underflow/inexact; stepping back to the
  least normal does not. The zero-to-subnormal path leaves `errno` unchanged;
  overflow `errno` differs between the tested glibc and musl hosts, so the
  differential test compares exception flags for that case.
  `fmal` also checks a halfway product at the subnormal boundary under all four
  rounding modes. The standalone check runs under Zig, GCC, and TinyCC.
  Complex-math
  coverage now includes `cabs*`, `creal*`, `cimag*`, `conj*`,
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
  from underflow set `ERANGE`. Long-double `powl` preserves exact integer
  powers of two through the binary80 subnormal range, avoiding false `ERANGE`
  on exact results; differential checks cover `powl(LDBL_MIN, 1)` and
  `powl(2, -16445)` under glibc and musl, with standalone coverage under Zig,
  GCC, and TinyCC. It sets `EDOM` for negative square root and
  invalid remainder domains. Standalone x86-64 implements `feclearexcept`,
  `fegetexceptflag`, `feraiseexcept`, `fesetexceptflag`, `fetestexcept`,
  `fegetround`, `fesetround`, `fegetenv`, `feholdexcept`, `fesetenv`, and
  `feupdateenv` with x87/SSE state synchronization. Hybrid profiles retain the
  host fenv ABI and implementation. Standalone smoke tests verify SIGFPE
  delivery for all five standard exceptions through x87 and SSE trap paths;
  they also verify `feupdateenv` merges newly raised flags into the restored
  environment and that an invalid rounding mode leaves the active mode intact.
  More trap-mask and environment corruption cases still need validation. Bit-level
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
  runtimes differ. The declared long-double math entry points are implemented;
  their broader edge and accuracy coverage plus less-common fenv trap
  environment cases remain open. `sinh`/`cosh`/`tanh` and their float forms use small-input series,
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
  or `pthread_cond_timedwait`. Async cancellation during a condition wait is
  deferred until after the mutex is reacquired, so cleanup handlers run with
  the required mutex ownership. Condition variables select realtime or monotonic
  clocks through their attributes. `pthread_cond_clockwait` cancellation is
  tested with a monotonic deadline; the waiter reacquires its mutex, runs its
  cleanup handler, and clears the condition wait count. Mutexes support
  absolute realtime timed locks, priority-inheritance futexes, and robust
  owner-death recovery through Linux robust lists. The smoke covers robust PI
  recovery both with and without a blocked waiter, PI timed-lock deadlines, and
  deferred cancellation while a PI waiter is blocked. Forked processes refresh
  their robust-list registration; process-shared robust recovery is
  exercised by a child that exits while holding the mutex. Spin locks, barriers,
  and read-write locks support process sharing. Reusable barriers use futex
  generations; async cancellation is deferred until the current generation
  completes, preventing a canceled arrival from stranding peers. Smoke coverage
  cancels a queued async waiter, completes the generation, then checks join and
  barrier destruction. Read-write locks support recursive readers,
  process sharing, try-locks, and absolute realtime timed locks; the reader-
  preference policy can starve writers under a continuous reader stream. It
  also interrupts blocking calls routed through the cancellable syscall
  wrappers, including read/write, socket I/O,
  poll/select, waits, sleeps, openat, and fsync. Condition-wait cancellation
  reacquires the mutex, then unwinds cleanup handlers before thread-specific
  data destructors. Semaphore wait retries check for deferred cancellation
  after waiter registration races; without that check, a cancellation wake
  could repeatedly change the sequence and spin instead of exiting. The
  semaphore smoke cancels a blocked waiter and verifies the semaphore remains
  destroyable. It also checks cancellation cleanup with a TSD value and verifies
  handler-before-destructor order. In deferred mode, ordinary CPU-bound code
  must call `pthread_testcancel`. Key deletion invalidates the generation, so
  another
  thread's stored value receives no destructor after deletion even when the
  key slot is reused; the smoke verifies this behavior. A separate TSD test
  re-registers its value on every destructor call and confirms exactly the
  four `PTHREAD_DESTRUCTOR_ITERATIONS` passes run.
  Signal 32 is reserved internally and cannot be installed or blocked by
  applications. Asynchronous cancellation requires application code to stay
  within async-cancel-safe operations. The initial thread has its own pthread
  context; cancellation state, cleanup handlers, `pthread_exit`, and thread-
  specific destructors use the same path as created threads. Once exit begins,
  cancellation is disabled through cleanup handlers and TSD destructors, so a
  cancellation point inside a destructor cannot recursively restart thread
  exit. `pthread-exit-standalone-matrix` covers a canceled request during a
  blocked TSD destructor and checks the original thread result and one destructor
  call under Zig, GCC, and TinyCC. The cleanup stack
  is bounded to 32 entries per thread. Internal allocator critical sections,
  the pthread-key and default-attribute locks, robust-list slot allocation,
  priority-protection bookkeeping, and the named-semaphore registry defer
  asynchronous cancellation until after releasing their locks, then retrigger
  a pending request. The standalone pthread smoke holds this deferral region
  across an async-cancel request while repeatedly exercising small allocation,
  direct mappings, `mremap`-backed `realloc`, and pthread-attribute operations,
  then verifies cleanup runs after release. It also cancels an async thread
  after confirming it is queued on a mutex futex, then checks that the owner can
  unlock and reuse the mutex. A separate async test cancels a waiter queued on
  a priority-inheritance futex while the owner still holds the mutex, then
  verifies unlock and reacquisition. The robust PI variant also verifies the
  surviving owner does not receive `EOWNERDEAD` after canceling its waiter.
  Robust non-PI waiters also have queued-cancel, owner-unlock, and mutex-reuse
  smoke coverage.
  Async-canceled rwlock writers leave `waiting_writers` clear and the lock
  usable. If an async-canceled thread exits inside a `pthread_once` initializer,
  cleanup resets the once control so a later caller can retry; a full cleanup
  stack defers cancellation until the initializer completes. Smoke coverage
  exercises cancellation, retry, and exactly-once completion. These checks run
  on Zig, GCC, and TinyCC. Blocking `pthread_join` and `pthread_timedjoin_np`
  check deferred cancellation while polling for target exit; cancellation
  releases the join claim. Smoke coverage cancels both forms while waiting,
  then confirms another caller can still join each target.
- Standalone pthread attributes support configurable stack sizes, page-
  protected guard regions, and scheduler inheritance/policy/priority. Explicit
  scheduling is applied before a startup gate releases the worker; permission
  failures are returned from `pthread_create` without running the routine.
  `pthread_attr_getguardsize` reports the requested byte size, while stack
  mappings round the guard region up to whole pages. `pthread_getschedparam` and
  `pthread_setschedparam` query and change live Linux thread scheduling.
- Standalone unnamed POSIX semaphores now provide `sem_init`, `sem_destroy`,
  `sem_wait`, `sem_timedwait`, `sem_trywait`, `sem_post`, and `sem_getvalue`.
  They use Nelua-owned futex waits. Async cancellation is deferred until the
  waiter count and cancellation wake address are cleared; smoke coverage checks
  that a canceled waiter leaves the semaphore destroyable.
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
