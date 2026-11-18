# Optimization contracts and the glibc comparison

The [subsequent gap-reduction candidate](gap-reduction-20261117.md) adds
bounded copy/fill paths, short comparisons and AVX2 string scans. The measured
candidate section below preserves the earlier purity/SIMD run as history.

The pinned comparison on the site measures implementation calls through
volatile direct-call wrappers. Correct purity metadata helps actual callers
remove unnecessary work; it does not replace optimizing the implementation
that remains after those transformations.

## Contracts

| Operation | C contract | Explicit Nelua marker |
| --- | --- | --- |
| Read-only byte/wide memory and string searches/comparisons | `cattribute '__pure__'`; `NELIBC_PURE` in public headers | No explicit `nosideeffect` on pointer-reading queries/imports |
| Integer `abs`/`labs`/`llabs`, `ffs` family, `isascii`/`toascii` | `cattribute '__const__'`; `NELIBC_CONST` in headers | `nosideeffect` on value-only definitions; imports remain conservative |
| Narrow character classification/conversion | `pure`, allowing locale reads in a fallback implementation | No stronger explicit promise |
| Copies, tokenizers, allocation, I/O | Observable writes/state; no pure/const promise | Preserve effects |
| Numeric parsing | End-pointer and errno writes; no pure/const promise | Preserve effects |
| General libm calls | errno, rounding mode and floating-point exception behavior; no blanket pure/const promise | Preserve effects |

`pure` permits reading mutable, non-volatile memory; `const` is stronger and
cannot describe `strlen`, `strcmp`, or `memcmp`. Public ctype declarations
stay `pure` even though the current narrow implementation is ASCII arithmetic:
the same headers can call a locale-dependent host libc in a hybrid consumer.
Locale switches, collation caches and error-string buffers are left conservative.
The attributes header becomes empty on compilers without GNU-style support.

Nelua `nosideeffect` is not an automatic translation of GCC `pure`. It controls
front-end effect analysis; C `pure` gives the backend an alias-aware memory
read contract. Keep C imports conservative in Nelua and place the C contract
on their declarations.

A diagnostic on pinned compiler `dabbd9ef`/GCC observes the overwritten text
in this nested-argument form, **with or without** an explicit `nosideeffect`:

```nelua
first(strcmp(text, "abc"), overwrite_text())
```

This does not isolate an annotation regression; the emitted C leaves the
argument order to the C compiler. `tools/purity_argument_order.nelua` retains
the diagnostic separately. The passing contract test instead uses explicit
sequencing:

```nelua
local before = strcmp(text, "abc")
overwrite_text()
```

`tools/purity_order.nelua` checks the saved result and a subsequent read of
modified memory. No new frontend purity marker is required for these queries.

`make purity-check` also checks optimized GCC/Clang callers: repeated queries
collapse, unused queries disappear, queries after writes/locale changes are
retained, and unused-return calls with output, errno or fenv effects remain.
GCC's [attribute documentation](https://gcc.gnu.org/onlinedocs/gcc/Common-Attributes.html)
defines the distinction between `pure` and `const`.

The legacy memory and string shootouts also keep callback pointer reads
volatile. A volatile result checksum alone does not retain repeated pure
calls: the backend can replace a loop of identical queries with one query
and a multiply. Before this fix the legacy GCC memcmp/memmem rows measured
0.00 ns; those rows are invalid implementation timings. The newer matched
consumer already uses volatile direct-call wrappers and is unaffected.

## First implementation change: strcmp

The old Nelua path compared one byte at a time. The x86-64 path now uses an
SSE2 first block and grouped comparisons, with runtime-dispatched AVX2 after
an equal 80-byte prefix. AVX2 reuses the existing CPUID/OSXSAVE/XGETBV checks;
short strings avoid that detection. Non-x86 builds retain the scalar path.

Each vector load stays within the current 4 KiB page in both inputs. A scalar
boundary path handles independent alignments and protected neighboring pages.
The AVX2 path aligns its left stream by revisiting at most 31 already compared bytes; it never backs up before either input. The first unequal or zero byte decides the unsigned-byte comparison result.
The forced `NELIBC_DISABLE_AVX2` build exercises the SSE2-only path.

`strcmp-guard-{gcc,zig-musl,cosmo}` exercise public archive symbols with
independent alignments, high bytes, early mismatches/NULs, multi-page strings
and guard pages. `tools/strcmp_bench.c` adds equal, first/middle/last mismatch
cases and several alignments. The original dated site measurements remain a
baseline until fresh recorded results replace them.

After strcmp, the measured glibc gaps are short/mid-size memset and memcpy,
small memcmp, and integer parsing. Tune size dispatch and entry overhead,
measure alignment and mismatch distributions, and retain the correctness
and non-AVX paths. The same compiler/options/input must be used on each side
of a comparison; single-host hot-buffer results do not establish a universal
fastest implementation.

## Measured candidate: 2026-11-17

Bak Incus `nelibc-writ`, unprivileged `build`, Intel Core i5-10400,
CPU 10 affinity, GCC 12.2/glibc 2.36. These are hot-buffer measurements on
one host. Seven alternating pairs use the same `-O2 -fno-builtin` C consumer;
the volatile direct-call wrapper retains implementation calls despite purity
metadata. Header annotations instead benefit real callers that can eliminate
or combine queries.

| Equal strcmp, bytes | glibc median ns | candidate median ns |
| ---: | ---: | ---: |
| 8 | 2.54 | 2.50 |
| 32 | 2.78 | 4.95 |
| 128 | 5.74 | 8.38 |
| 1024 | 15.08 | 18.44 |
| 4096 | 47.12 | 60.00 |
| 65536 | 1337.51 | 1376.88 |

The ratio of medians at 64 KiB is about 3% more time than glibc; the median
of the seven paired glibc/candidate speedups is 0.947. These statistics differ
and should not be interchanged. The earlier published 33344.79 ns Nelibc
measurement is a historical baseline, not a concurrent before/after sample.
The mixed parser remains slower than glibc: 254.17 versus 185.89 ns median.

The separate `tools/strcmp_bench.py` run compares the exact archived previous
GCC library with the candidate and glibc in rotating order. It covers eleven
lengths, four alignment pairs, and equal/first/middle/last mismatch positions,
with seven samples each. For offsets (0,0), equal 64 KiB takes 1395.15 ns
versus 20402.59 ns previously and 1278.95 ns for glibc: a median paired 14.73x
gain over the previous archive, still about 9% more time than glibc. A
first-byte mismatch takes 2.85 ns versus 1.96 ns previously (64 KiB input
case). Short and early-mismatch entry costs need work before claiming a
general win. Do not summarize only equal long strings.

All six matched consumer binaries passed 1995 assertions each. Each of the
three candidate public archives passed 848508 guarded strcmp comparisons;
the final assembly compiled with `NELIBC_DISABLE_AVX2` passed the same count.
GCC and Zig/Clang optimized-caller contracts and the explicitly sequenced
Nelua mutation check passed.

The independent pinned libc-test rerun passes 325/343 with glibc + Nelibc,
333/343 with musl + Nelibc, and 302/343 with Cosmopolitan + Nelibc. Every
individual test status, for both halves of all three pairs, matches the
previous published comparison. This candidate adds no conformance regression
relative to that candidate; the existing musl `sigprocmask-internal`
regression relative to plain musl remains.

Evidence is retained under
`/srv/ai/artifacts/nelibc-cosmo/pure-strcmp/` inside `nelibc-writ`:
`comparison/` contains commands, hashes, checks and 2394 raw timing rows;
`distribution/` contains the before/after mismatch and alignment samples;
`conformance/` contains the independent libc-test corpus and per-test logs.
The candidate GCC, musl and Cosmopolitan archive SHA-256 values are,
respectively:

```text
aa93d82e5887d195a8e3be1efadda15154996e932a0df742a8e6fe0f0ecc4009
7ccd41250ade2de3191a5c9b12a5ecf637fbcd51ba65a4502226c35005faac4d
648d276f382122d0fb1d3894a6600f6645ff563f09ac63ada34b3aee09acef91
```

At measurement time these source changes were unmerged; focused checks do not
substitute for the repository's full behavior-change gate. The subsequent
[full integration run](integration-gate-20261117.md) passed all 14 targets.
This section preserves the earlier candidate; the linked follow-up records
its successor.

## Next work, ranked

1. Reduce strcmp entry/page-window overhead for 16–128-byte strings and
   first-byte mismatches; keep guard-page and independent-alignment coverage.
2. Tune short/mid-size memcpy and memset dispatch. The published glibc ratios
   are about 0.60 for memcpy(128), 0.56 for memcpy(1024), and 0.53 for
   memset(128/1024); these functions must retain their writes.
3. Reduce small memcmp entry and result-selection costs (published ratios
   about 0.46 at 8 bytes and 0.56 at 128 bytes).
4. Optimize common strtol decimal inputs while preserving overflow, errno,
   base detection and end-pointer semantics; measure the mixed parser too.
5. Repeat on other CPUs and realistic input distributions before making a
   fastest-implementation claim. Keep raw paired measurements and conformance
   results with each candidate.
