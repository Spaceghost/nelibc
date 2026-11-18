# Valuable problem cases, easiest to hardest — 2026-11-17

This pass keeps three measured changes. The ordering favors common primitives
with simple correctness obligations before changes that need wider numerical
validation. Application profiles can refine the value ranking later.

| Order | Kept change | Scope |
| --- | --- | --- |
| 1 | Bounded SIMD short searches | memchr, 16–63 bytes; scalar first-byte exit |
| 2 | Constant-base overflow cutoffs | Narrow integer parsing, bases 8/10/16 |
| 3 | Constant-width sorting | qsort, 4- and 8-byte elements |
| 4 | Next: short-string and medium-search dispatch | Measure early exits and alignment before selecting a path |
| 5 | Harder: floating conversion and formatting | Preserve rounding, subnormals, errno and output semantics |

## Measured gains

Bak Incus `nelibc-writ`, unprivileged build user, Intel Core i5-10400,
CPU 10 affinity, GCC 12.2/glibc 2.36. Seven process rounds rotate the same
consumer linked to native libc, the saved previous production archive, and
the new archive. The shared host is not an isolated benchmark machine.
Times include loop and wrapper overhead. The full data retains every case,
paired ratio and observed range rather than only these selected wins.

Median nanoseconds per operation, GCC/glibc profile:

| Case | Previous Nelibc | New Nelibc | glibc alone | Median paired gain |
| --- | ---: | ---: | ---: | ---: |
| memchr absent, 32 bytes | 6.55 | 3.18 | 2.79 | 2.04x |
| strtol decimal | 21.39 | 13.90 | 15.87 | 1.51x |
| qsort, 128 integers including input setup | 3355.27 | 2015.59 | 2217.93 | 1.65x |

The mixed parser remains slower than glibc: 255.62 versus 190.90 ns.
The isolated gains do not establish a whole-application win. Cosmopolitan
continues to delegate integer parsing and sorting; its surrounding executable
timings are not credited as Nelibc implementation gains.

## Why these changes

The new short-search helper loads only bounded head/tail vectors and selects
the first matching byte, including overlapping blocks. The public memchr
retains its scalar first-byte exit and its scalar path below 16 bytes. The
existing large-buffer helper is separate, so it does not acquire another
short-size dispatch check. The read-only helper uses C pure metadata without
a stronger Nelua pointer-effect promise.

Integer parsing previously divided by a variable base before dispatching to
its decimal, octal and hexadecimal loops. Moving the cutoff/remainder
calculations into those branches makes the divisors literal constants. The
C backend can fold them or use reciprocal/shift operations. Base validation,
prefix/sign handling, overflow scanning, errno and end pointers retain their
existing rules. Other bases keep the generic divide. The wide parser is
outside this change.

The existing sort implementation is now an inline helper. Public qsort calls
it with literal widths 4 and 8 for those cases, and the actual width for the
generic case. Constant propagation removes byte-loop and address-multiply
overhead. Its introsort, depth limit and heapsort fallback are preserved.
qsort_r is outside this specialization.

Sorting gains cost code space: the GCC qsort symbol grows from 3795 to
10511 bytes. The matched GCC consumer grows from 669416 to 677776 file bytes;
the musl consumer grows from 3473208 to 3514440. These files include each
toolchain's metadata and are not measurements of runtime memory use.

## Rejected experiments

Aggressive short-strlen prefixes helped some lengths but penalized others;
tiny-fill and byte-swap-only sort variants did not produce a convincing
overall gain. They are not in this candidate. The original first short-search
variant accelerated absent bytes but slowed first-byte matches; it was
replaced by the scalar-early-exit version. The separate scan benchmark covers
five alignments and absent/first/middle/last matches, with steady-state CPU
dispatch warmed for every implementation.

The final scan run retains tiny-search performance below 16 bytes. At offsets
(0,0), first-byte matches at 32/63 bytes take 2.26/2.28 ns versus the previous
2.01/2.01 ns: a small remaining tradeoff rather than a claim that every case
is faster. Absent 32/63-byte searches improve by 2.18x/4.02x in that separate
consumer; middle and last matches improve too.

## Verification

All nine matched consumers passed 1995 assertions before timing. All three
profiles passed 848508 guarded strcmp comparisons and 10752 strlen checks.
Bytewise memory/reference tests passed for GCC, musl and Cosmopolitan. The
final forced-SSE2 GCC archive passed the memory tests and both string guards.
The final GCC and musl public archives match their respective native outputs
on the patterned/randomized qsort+bsearch suite. Context-comparator smokes
and the diagnostic depth-fallback tests passed earlier on the same sort code.

The final independent corpus preserves every individual status from the
previous candidate: 325/343 tests pass with glibc + Nelibc, 333/343 with
musl + Nelibc, and 302/343 with Cosmopolitan + Nelibc. Existing non-passes,
including musl's sigprocmask-internal regression relative to plain musl,
remain visible. These counts are test coverage, not standards certification.

## Evidence

Retained inside `nelibc-writ` at
`/srv/ai/artifacts/nelibc-cosmo/easy-first-20261117/`:

* `before/`: the actual previous production archives and source files.
* `comparison-final/`: 3591 timing rows, all nine consumer checks, exact
  commands, source hashes, archive hashes and per-case paired summaries.
* `scan-final/`: alignment and match-position measurements.
* `conformance-final/`: independent libc-test results and per-test logs.

The run-source archive is checked against every file hash in the timing
manifest before packaging. Exploratory samples and the rejected first search
version remain separate from the final evidence. These focused checks do not
replace the full integration gate in AGENTS.md; the source remains a candidate.
