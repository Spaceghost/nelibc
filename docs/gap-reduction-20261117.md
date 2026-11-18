# Measured libc gap reduction — 2026-11-17

This candidate reduces the implementation costs left after adding safe purity
metadata. Measurements use Bak's Intel Core i5-10400, CPU 10 affinity inside
`nelibc-writ`, GCC 12.2/glibc 2.36, Zig 0.17 bundled musl, and Cosmopolitan
4.0.2. The host is shared; affinity does not reserve the CPU.

## Concurrent before/after comparison

The three previous production archives were copied before changing source.
For each profile, seven rounds rotate three consumers: its libc alone, the
previous Nelibc archive, and the new archive. Compiler, flags, C source and
inputs are identical within the profile. Volatile direct-call wrappers retain
calls even for pure functions. Timings include loop and wrapper overhead.

Selected GCC/glibc cases, median nanoseconds per operation:

| Case | Previous Nelibc | New Nelibc | glibc alone | Median paired gain over previous |
| --- | ---: | ---: | ---: | ---: |
| memcpy, 128 bytes | 4.48 | 2.72 | 2.48 | 1.64x |
| memcpy, 1024 bytes | 16.87 | 9.16 | 9.40 | 1.83x |
| memset, 1024 bytes | 16.05 | 8.22 | 8.12 | 1.96x |
| memcmp equal, 8 bytes | 5.92 | 2.22 | 2.72 | 2.63x |
| memchr absent, 1024 bytes | 21.58 | 10.91 | 8.60 | 1.98x |
| strlen, 1024 bytes | 24.49 | 9.72 | 7.62 | 2.52x |
| strcmp equal, 32 bytes | 4.93 | 3.22 | 2.72 | 1.54x |

Other useful limits: memcpy(4096) is 31.66 ns versus glibc's 32.39 ns;
memset(65536) is 1097.94 versus 1106.80 ns; equal strcmp(65536) is 1333.80
versus 1328.75 ns. These hot-buffer measurements do not establish a universal
fastest implementation or DRAM bandwidth.

Keep the losses visible. memset(8) increases from 2.52 to 2.72 ns, while glibc
measures 2.71 ns. memchr-absent(8) increases from 2.97 to 3.30 ns, while glibc
measures 2.72 ns. Small strings, medium memcmp, conversion, formatting and
the mixed parser still have gaps. The parser is 250.30 versus glibc's
185.21 ns; the isolated memory wins do not imply the whole workload wins.
Observed paired ranges and every case are retained, including unrelated
functions and slower results.

## Implementation

* memcpy uses bounded head/tail loads and stores for short inputs, runtime
  AVX2 for medium copies, and REP MOVSB from 8192 bytes. Tiny copies avoid
  CPU-feature dispatch. Non-x86 code and the TinyCC public copy path remain.
* memset replaces scalar tails and repeated short-loop checks with bounded
  head/tail stores. Mid-size fills use runtime AVX2; fills from 2048 bytes
  use REP STOSB. Small SSE2 and non-x86 fallbacks remain.
* memcmp below 16 bytes uses bounded word loads and unsigned byte order;
  64–128-byte AVX2 comparisons combine equality masks before resolving the
  earliest mismatch. A comparison need only return the correct sign.
* strcmp admits a 64-byte prefix with one conservative page check and tests
  its vectors in order. An initial AVX2 block avoids exact page-window setup
  when it can resolve the comparison immediately. The first unequal/NUL byte
  still determines the result, and independent page boundaries remain guarded.
* strlen switches to AVX2 after an equal SSE prefix. Grouped loads begin on
  128-byte boundaries, so no group crosses a 4096-byte page. Bounded memory
  scans consider AVX2 from 256 bytes instead of 2048 bytes.

Runtime dispatch retains CPUID, OSXSAVE and XGETBV checks and reuses the
existing cache. AVX paths clear their upper vector state before returning
or transferring to SSE. `NELIBC_DISABLE_AVX2` retains the baseline paths.

## Verification and evidence

Each profile passed 848508 guarded strcmp comparisons and 10752 strlen
checks, including high bytes, independent alignments, exact boundary lengths
and protected adjacent pages. Bytewise memory/reference tests passed against
GCC, musl and Cosmopolitan archives. The forced SSE2 GCC archive passed the
memory tests and both string guards. Optimized GCC/Clang caller contracts
and the explicitly sequenced Nelua mutation check passed. All nine matched
consumer binaries passed 1995 assertions before timing.

The final independent libc-test run preserves every individual status from
the previous candidate: 325/343 pass with glibc + Nelibc, 333/343 with
musl + Nelibc, and 302/343 with Cosmopolitan + Nelibc. The existing musl
`sigprocmask-internal` regression relative to plain musl remains.

The alignment/mismatch distributions confirm that memcpy(128), memcpy(1024),
memset(1024), memcmp(8) and memcmp(128) improve across all four measured
alignment pairs. For offsets (0,0), first-byte strcmp mismatches improve from
about 2.83 to 2.21 ns at the 64 KiB input size. Equal 64 KiB strcmp in this
separate distribution run measures 1439.55 ns versus the previous 1361.52 ns
and glibc's 1280.40 ns. Long-string results vary between the two consumers;
retain both rather than claiming that every alignment or workload is faster.

Retained evidence inside `nelibc-writ`:
`/srv/ai/artifacts/nelibc-cosmo/gap-reduction-20261117/`.
`comparison-final/` contains 3591 raw timing rows, seven samples for each of
57 cases, three profiles and three implementations, plus exact commands,
source hashes and executable/archive hashes. `before/` contains the actual
previous archives. `strcmp-distribution/` and `memory-distribution/` add
alignment and mismatch-position cases. Earlier exploratory samples are
retained separately and are not substituted for this final run.

The new archive SHA-256 values are:

```text
GCC:    b4975eb427ce91c52bc6238d9660c97db973b022226cc559d47117140a7a04e1
musl:   f0166bc73f35bcb6693a7c67ef7c6322f38a558a8499c515fd98089ba8e5e35c
Cosmo:  b27b8345de288c5ced933e0cf8a9d20774618f91f26860bd6c645990b25d60b0
```

These changes are an unmerged candidate. Focused checks and the independent
corpus do not substitute for all behavior-change gates in AGENTS.md.
