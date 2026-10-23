# Core gaps and priorities

This is a working order for making Nelibc a dependable, general-purpose Linux
libc. “Implemented” in the API manifest means a symbol is present in the
archive; it does not certify every behavior required by C or POSIX.

## Priority order

| Priority | Gap | Why it blocks a compelling libc | Next milestone |
|---|---|---|---|
| P0 | Reliable build and consumer gate | Regressions in any compiler/libc profile undermine every replacement | Keep `make smoke-matrix` green; run `make dogfood-matrix` for an application link and verify selected Nelibc symbols are in each executable |
| P1 | C math library | The Nelua slice includes binary32/binary64 `sqrt`, `cbrt`, `fmod`, `remainder`, `fmin`, `fmax`, `fdim`, `frexp`, `modf`, `ldexp`, `scalbn`, `scalbln`, `ilogb`, `logb`, `nextafter`, `nexttoward`, `fabs`, `copysign`, `trunc`, `floor`, `ceil`, `round`, `rint`, `nearbyint`, `exp`, `log`, `pow`, `hypot`, `sin`, `cos`, `tan`, `sinh`, `cosh`, `tanh`, `asinh`, `acosh`, `atanh`, `atan`, and `atan2`; the arithmetic backend is x86-64-specific, and most math entry points remain open | Expand the math set; then long-double coverage and complete fenv semantics |
| P1 | Standalone threading completeness | Standalone pthread is a subset; cancellation and broader synchronization APIs are explicitly unimplemented | Keep the supported static-TLS ABI bounded and documented; prioritize pthread cancellation and synchronization semantics |
| P2 | Locale and character data | Standalone locale data is small (C plus selected test locales), while case folding is ASCII-only; hybrid mode often uses host locale data | Define supported locale data sources and implement locale-aware ctype, collation, numeric, monetary, and time behavior consistently |
| P2 | API-surface audit | The generated coverage table only sees declarations in current project headers and smoke references are locators, not proofs | Compare declared/exported/delegated APIs against a pinned musl release and label every intentional omission |
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

- The current API inventory has 589 declared names: 551 marked implemented,
  38 intentionally host-delegated across the hybrid profiles, and one
  implementation-only entry. This is an inventory signal, not a completeness
  percentage.
- Differential tests cover meaningful memory, string, integer, floating
  conversion, wide-string, tokenizer, collation, and syscall behavior across
  glibc, musl, GCC, Zig cc, and TinyCC profiles.
- Time APIs are already implemented across `src/unistd.nelua` and
  `src/stdlib.nelua`: clocks and sleeps, UTC/local conversion, `mktime`,
  `timegm`, `strftime`, `strptime`, and the `asctime`/`ctime` family. The
  syscall smoke checks UTC and negative epochs, leap days, POSIX TZ rules,
  TZif zone files, DST transitions, formatting bounds, and CPU clocks. Broader
  locale-specific time names and cross-libc differential coverage remain open.
- The current math slice in `src/math_core.nelua` exports 78 float and double
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
- Hybrid stdio adapts to host `FILE` implementations in several profiles. The
  standalone `FILE` implementation is Nelua-owned, but full standards and
  edge-case coverage remains a separate conformance task.
- Standalone startup reads the executable's `PT_TLS` image and initializes
  static TLS. New pthreads receive a private copy of that static image. This
  supports the local-exec TLS model used by current fixtures; it is runtime
  support in assembly/Nelua, not a Nelua compiler TLS implementation. Dynamic
  TLS modules and a DTV are not implemented, and are deferred until a target
  application needs them. Standalone pthread has no cancellation support or
  full POSIX synchronization coverage.
- The raw syscall and process-startup implementation targets Linux x86-64.

## Completion rule

Do not use a symbol count as a completion score. For each API, record whether
Nelibc implements it, intentionally delegates it, or has not implemented it;
then attach behavior tests, ABI checks, and the compiler/libc profiles that
passed. A family is ready to replace only when its tests run without relying
on the host implementation for the behavior under test.
