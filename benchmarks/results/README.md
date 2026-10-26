# Memory shootout record — 2026-10-21

## Standalone collation stress — 2026-10-25

`2026-10-25-collation-standalone-{zig,gcc}.csv` times Nelibc `strcoll` and
`strxfrm` on one deliberately adversarial cluster containing representatives
from all 55 Unicode 16.0 nonzero combining classes, repeated to 55, 220, and
880 marks. Each row is the median of five samples. The C harness is built with
`-O2`; Nelua objects use the normal standalone profile build flags. On Fedora
44 / Linux 7.2.7 with an Intel Core i7-11700F, Zig cc measured 147 us at 55
marks and 2.30 ms at 880 marks for both functions. GCC measured 223 us and
3.60 ms for `strcoll`; `strxfrm` measured 201 us and 3.12 ms on the repeat run.
The near-linear scaling is expected from the bounded combining-class scan,
though the absolute cost is high for unusually long clusters. TinyCC was not
installed on this machine, so its standalone result is not available.
Reproduce with `make collation-shootout-standalone-matrix` where TinyCC is
installed, or run the Zig and GCC targets individually.

The complete four-profile `make shootout-matrix` run from 2026-10-22 is
preserved in `2026-10-22-full-shootout-matrix/`. It contains thirteen CSVs,
including syscall-heavy pathconf and vectored-I/O comparisons; the command
completed after restoring the float shootout's C-locale radix callback.

`2026-10-22-memory-shootout.csv` is the merged memory-function shootout from
`make shootout-matrix`. It contains 126 benchmark cases for each of Zig/glibc,
GCC/glibc, Zig/musl, and TinyCC/glibc, with host and Nelua implementations.
The run covers copy, move, compare, fill, scan, and substring-search cases;
values are seven-sample medians and machine-specific.
`2026-10-24-memory-shootout-labelled-cases.csv` is a fresh four-profile run
after the long-needle `memmem` case names were amended to include needle length.
It contains 63 host/Nelibc pairs per profile and seven-sample medians. This
un-pinned run was measured on Fedora 44, kernel 7.2.7, with an Intel Core
i7-11700F; TinyCC was 0.9.28rc (`e23dabe`). It is a same-machine comparison,
not a cross-machine performance claim.
`2026-10-24-tcc-memory-shootout-cpu2.csv` is the median of three additional
CPU-2-pinned TinyCC runs; each `repeatN` file preserves one run, with each row
already the median of seven timed samples. For 64 KiB periodic absent needles,
the median Nelibc/host ratio across nine needle lengths is 1.20 (geometric
mean 1.28); lengths 8 through 256 cluster near 1.20, while length 4 is 2.25.
This is a reproducible TCC substring-search performance target. Needle-tail
matches are faster overall, with a 0.50 median ratio but a wide 0.03–3.59
range, so the geometric mean alone is not representative. See the CSVs before
changing the algorithm.
`2026-10-22-memory-shootout-memmove-fastpath-repeat1.csv` and
`2026-10-22-memory-shootout-memmove-fastpath-repeat2.csv` measure the Nelua
fast path that delegates proven-disjoint ranges to `memcpy`. For 64-byte
destination-after-source copies, Zig/glibc improves from 12.11 ns to 2.95 and
2.74 ns, and Zig/musl from 11.79 ns to 2.91 ns in both repeats. The change also
keeps the overlap cases on their directional loops. All four compiler/libc
memory differential checks passed.
`2026-10-22-memory-shootout-tcc-sse2.csv` records the four-way matrix after
adding a TinyCC SSE2 fill for ranges up to 256 bytes; larger fills retain
`rep stosb`. The 64-byte TinyCC/Nelibc `memset` drops from 10.06–10.29 ns to
4.53–4.77 ns over three candidate runs. Two isolated repeats measure 4 KiB at
19.57 and 20.49 ns, and 64 KiB at 743 and 728 ns, close to the original
19.63 ns and 723 ns baseline. The full matrix run's 4 KiB and 64 KiB samples
were noisier at 23.51 and 770 ns. `repeat2.csv` and `repeat3.csv` preserve the
isolated TinyCC runs; each contains the full TinyCC memory benchmark rows.
`2026-10-22-memory-shootout-gcc-repmovsb-repeat1.csv` and
`2026-10-22-memory-shootout-gcc-repmovsb-repeat2.csv` measure the GCC-only
`rep movsb` path for `memcpy` ranges of 256 bytes and larger. At 4 KiB,
Nelibc improves from 58.79–59.32 ns to 18.33–18.68 ns, close to glibc's
17.28–18.27 ns. At 64 KiB it measures 1.048–1.049 us versus 1.042–1.050 us
for glibc. The updated four-way matrix is
`2026-10-22-memory-shootout-gcc-repmovsb.csv`; its GCC rows measure 19.11 ns
at 4 KiB and 1.060 us at 64 KiB, versus host results of 17.39 ns and
1.074 us.
`2026-10-22-memory-shootout-repmovsb-matrix.csv` records the subsequent
four-way run after extending the same path to Zig cc and TinyCC. At 4 KiB,
Zig/musl measures 19.22 ns against 54.71 ns for musl; Zig/glibc and GCC are
near their host results. TinyCC imports `memcpy` from Nelua and exports it
directly from the TCC-specific assembly object, removing the generated wrapper.
The direct entry uses four unaligned SSE2 moves for exactly 64 bytes and
`rep movsb` otherwise. The earlier Zig-built benchmark harness measured
1.86–1.89 ns at 64 bytes; the authoritative native TinyCC-driver shootouts
measure 2.47–2.51 ns across three runs. Host libc measures 2.27–2.30 ns, and
the direct `rep movsb` baseline measures 4.21–4.30 ns. The SSE2 path improves
the previous Nelibc implementation by about 41%, while host libc remains
8–10% faster. At 4 KiB, the native TCC-driver samples are 17.26–18.73 ns for
Nelibc and 17.70–18.45 ns for host.
The full native TCC-driver data is in
`2026-10-22-memory-shootout-tcc-memcpy-native-{libc,sse64,repmovsb}-repeat{1,2,3}.csv`;
the refreshed native matrix is
`2026-10-22-memory-shootout-tcc-public-sse64-native-matrix.csv`. Run
`make shootout-tcc-memcpy-variants` to reproduce the host, SSE2, and `rep movsb`
comparison. The differential check covers all 16 source and destination
alignments near both protected page edges and passes with TinyCC, Zig cc, and
GCC consumer builds. The previous direct `rep movsb` matrix remains in
`2026-10-22-memory-shootout-tcc-public-memcpy-matrix.csv`. `smoke-zig`,
`smoke-gcc`, `smoke-zig-musl`,
`smoke-tcc`, `lib-smoke-matrix`, `smoke-standalone-tcc`, and
`lib-consumer-standalone-matrix` pass with the helper linked.
`2026-10-22-memory-shootout-sse2-memset-matrix.csv` records the matrix after
adding a bounded SSE2 fill path for 16-byte-and-larger `memset` calls under
Zig and GCC. The 64-byte case improves from 5.04 to 2.43 ns with Zig/glibc
and from the earlier 39.87 ns per 4096-byte fill to 34.19 ns with GCC/glibc.
Zig/musl improves from 2.26 to 2.11 ns at 64 bytes and from 35.73 to 34.30 ns
at 4096 bytes. TinyCC retains its existing Nelua fill path.
`2026-10-22-mem-repeat-zig-glibc.csv` and
`2026-10-22-mem-repeat-zig-musl.csv` are a repeat of the seven-sample Zig
memory shootouts. The anomalous first-run glibc 64-byte `memcmp-last-diff`
result (8.54 ns) did not reproduce as a Nelibc regression; this repeat was
8.67 ns, though that short mismatch case remains noisy. At larger sizes,
Nelibc equal-buffer `memcmp` measured 46.92 ns vs glibc's 33.82 ns at 4 KiB,
and 1.47 us vs 1.00 us at 64 KiB. The rare four-byte `memmem` search remains
much faster than host at 64 KiB (113 ns vs 4.83 us). Musl host results for
several memory calls are highly variable or far slower than Nelibc across
repeats, so compare those with pinned, alternating runs before drawing
performance conclusions.
The earlier SSE2 small-copy candidate is rejected: two wrapper-based runs
measured 15.89 and 16.68 ns at 64 bytes versus 2.27 ns for host. Two runs of
SSE2 in the direct public entry were also slower, at 14.36 and 14.63 ns.
Their CSVs are
`2026-10-22-memory-shootout-tcc-sse-copy-rejected-repeat1.csv` and
`2026-10-22-memory-shootout-tcc-sse-copy-rejected-repeat2.csv`; direct-entry
results are in `2026-10-22-memory-shootout-tcc-public-sse-copy-rejected-repeat1.csv`
and `2026-10-22-memory-shootout-tcc-public-sse-copy-rejected-repeat2.csv`.

`2026-10-22-strtok-shootout-initial.csv` records the first three-token
`strtok` measurements. `2026-10-22-strtok-shootout.csv` is the follow-up after
inlining token advancement into `strtok`; TinyCC improves from 21.89 to 20.27
ns per token but remains slower than host. Both files measure scans across
Zig/glibc, GCC/glibc, Zig/musl, and TinyCC/glibc. The Nelua implementation is
faster in the three Zig/GCC builds in this run, but TinyCC's generated loops
make its Nelua path slower. `strtok_r` is included
as a control. Results are seven-sample medians and are machine-specific.

`2026-10-22-alloc-shootout-public-current.csv` measures host allocator entry
points, prefixed Nelua calls, and the public hybrid allocator symbols. At
64 bytes, public malloc/free medians are 11.40 ns with Zig/glibc, 10.19 ns
with GCC/glibc, and 28.50 ns with TinyCC/glibc, versus 5.02, 4.23, and 5.77 ns
for glibc. These rows predate public Nelibc `malloc` on static musl. The
prefixed allocator measured 7.74 ns versus 21.50 ns for musl. The lock-free slab registry reduces Nelua-pointer
ownership checks to 3.06–6.47 ns across profiles. Direct-map ownership checks
use a reader gate and measure 11.55–13.10 ns in the current run. The 32 KiB
Nelua bridge realloc/free case measures 2.36–2.40 us after
page-rounded mappings began reusing final-page slack, down from 15.4–15.9 us. The CSV contains all four profile rows, seven-sample
medians, and 2,000 calls per sample. Results are machine-specific.

`2026-10-22-alloc-public-zig-musl-shootout.csv` compares native musl with the
public Nelibc allocator in 24 alternating process pairs. Each invocation
reports seven-sample medians of 2,000 calls for malloc/free, calloc/free,
same-class and growing realloc, and aligned allocation across 64 B–32 KiB.
At 64 B, public Nelibc malloc/free is 10.55 ns versus 14.39 ns for musl.
Growing realloc is faster through 4 KiB; at 32 KiB it is 4.25 us versus
4.20 us after Nelibc added an `mremap` fast path for direct mappings. Results
are machine-specific. Run with `make alloc-public-zig-musl-shootout`.

`2026-10-22-alloc-shootout-pre-fastpath-guard.csv` is the earlier four-profile
prefixed allocator and bridge run, before reentrant thread-state setup enabled
the fast path for glibc/TinyCC public malloc interposition.

`2026-10-22-alloc-thread-shootout-fastpath-current.csv` reruns the four-profile
threaded allocator matrix with thread-local cursors enabled for the prefixed
allocator calls with guarded TSD initialization. At eight workers and 64 bytes,
Nelua measures 2.11 ns per pair with Zig/glibc, 1.04 ns with GCC/glibc, 1.61 ns
with Zig/musl, and 4.33 ns with TinyCC/glibc. This benchmark does not interpose
public malloc.

`2026-10-22-alloc-shootout.csv` is the baseline after integrating the prefixed
allocator into the hybrid archives. `2026-10-22-alloc-batched-shootout.csv`
measures a per-thread batch refill path. It improves 64–128-byte `malloc/free`
by 23–32% with Zig/glibc, GCC/glibc, and Zig/musl; TinyCC/glibc remains within
4% of baseline in the earlier run because pthread thread-specific lookup cost
more than batching saved. TinyCC now uses C `__thread` storage for direct state
access and keeps pthread TSD only for thread-exit cleanup. The refreshed single-
thread shootout records 19 ns per 64-byte malloc/free pair under TinyCC. The
threaded shootout in `2026-10-22-alloc-thread-shootout.csv` measures 1, 2, 4,
and 8 workers over seven samples; at eight workers and 64 bytes, TinyCC/Nelibc
is 4.23 ns per pair versus 0.99 ns for TinyCC/glibc. Unconsumed batch blocks
return to the shared pools at thread exit. Single-thread runs cover seven
samples of 2,000 calls across all four builds.

`2026-10-21-alloc-shootout.csv` is the initial host-vs-Nelua allocator
baseline, before reusable slabs. It shows 2.2–2.6 us 64–128-byte malloc/free
due to one mapping per allocation. `2026-10-21-alloc-pool-shootout.csv` records
the first size-class pool run. It exposed TinyCC's generated `calloc` byte
loop at 2.7 us for 4096 bytes. `src/alloc.nelua` now calls Nelibc's shared
`nelibc_memset` implementation; `2026-10-21-alloc-pool-memset-shootout.csv`
records the rerun under the same four compiler/libc pairs. It reduced TinyCC
`calloc(1, 4096)` from 2.7 us to 121 ns.

`2026-10-21-alloc-realloc-inplace-shootout.csv` measures double-size
reallocations and 16-byte same-class growth. `nelibc_realloc` keeps a pooled or
direct-mapped block in place when its recorded capacity fits. For 4096-to-4112
byte growth, Zig/glibc, GCC/glibc, and Zig/musl measure 14–16 ns;
TinyCC/glibc measures 112 ns. In this run, 4096-to-8192-byte growth used the
direct-map path because the largest pool class was 8192 bytes.

`2026-10-21-alloc-pool-memcpy-shootout.csv` records the 16384-byte class and
`realloc`'s use of the shared Nelibc `memcpy`. Zig/GCC pooled 64–4096-byte
malloc/free measures 13–18 ns; that TCC run ranged from 35 ns at 64 bytes to
94 ns at 4096 bytes. TinyCC `calloc(1, 4096)` measured 133 ns. The
4096-to-8192-byte growth used a pool block and measured 58 ns with Zig/glibc,
92 ns with GCC/glibc, 58 ns with Zig/musl, and 269 ns with TinyCC/glibc, down
from a 3.0–8.0 us direct-map path. Same-class growth remains in place.

`2026-10-21-alloc-tcc-branches-shootout.csv` is the current run. Replacing the
repeated pool-size array materialization with direct size-class branches cut
TinyCC malloc/free to 18 ns at 64–4096 bytes. TinyCC's 4096-byte double-size
realloc now measures 68 ns, and same-class growth 24 ns.

These are reference microbenchmark results from one x86_64 Linux machine:

`2026-10-22-rand-r-shootout.csv` compares host and Nelibc `rand_r` across
Zig/glibc, GCC/glibc, Zig/musl, and TinyCC/glibc, using seven median samples
of 1,000,000 calls each. Zig's bundled musl-target libc shim uses SplitMix64;
Nelibc now matches its sequence and measures 1.18 ns/call against 1.34 ns/call
for that shim in this run.

The original archived CSVs use the earlier 1 MiB-per-sample workload. The
current harness uses 64 MiB per function sample to keep short timing windows
from dominating small-call measurements. Do not compare the original CSVs
directly with the `2026-10-21-64m-*` or `2026-10-21-wordscan-*` runs.

- CPU: Intel Core i7-11700F, 2.50 GHz (16 logical CPUs reported)
- Compiler runs: Zig 0.16.0 `zig cc` and GCC 16.2.1
- Libcs: host glibc and Zig's static x86_64 musl target
- Flags: `-O2 -fno-builtin` for both the C harness and Nelua-generated object
- Each row is the median of seven samples; sizes are 64, 4096, and 65536 bytes
- Functions: memcpy, mempcpy, `memmove` with forward/backward overlap and
  disjoint destination placements, memccpy, memset,
  `memcmp` with equal/first/middle/last mismatch cases, memchr, memrchr, and
  memmem

Files:

`2026-10-21-memmove-backward-align32-isolated-pinned.csv` is the current five-round integrated shootout. The aligned backward path is out of line, so forward and disjoint copies retain the original helper code layout. It cuts 64 KiB backward-overlap time by 20–23% against the same-source control on all four toolchain/libc pairs. Results are 0.75–0.77 us against glibc (0.73–0.78 us), and 0.75 us against musl (1.07 us). Forward and disjoint-after times stay within 4% of control, except musl and TinyCC forward overlap, which improve; disjoint-before is unchanged or faster.

`2026-10-21-memmove-forward-align32-candidate-pinned.csv` records a five-round Zig/glibc shootout of a forward path that copies a byte prefix to align the source before the AVX2 loop. Its 64 KiB forward-overlap median was 963 ns, effectively unchanged from the 965 ns control and slower than glibc at 763 ns. Disjoint destination-after regressed from 1209 to 1229 ns, and destination-before regressed from 1194 to 1205 ns. The candidate passed the memory differential check but is rejected; the production path remains unchanged.

`2026-10-21-memmove-forward-block512-pinned.csv` compares the existing 256-byte AVX2 forward loop with the 512-byte loop already used by TinyCC, across five CPU-2-pinned rounds of Zig/glibc, GCC/glibc, and Zig/musl. At 64 KiB, forward overlap improves 4.2% for Zig/glibc (1007 to 965 ns), is flat for GCC/glibc (759 to 758 ns), and regresses 2.6% for Zig/musl (737 to 756 ns). Across all tested sizes and placements, geometric-mean differences are +0.45%, +0.32%, and -0.12%, respectively. Keep the shared 256-byte dispatch; the 512-byte loop is not a consistent cross-build improvement.

`2026-10-21-memmove-forward-schedule-pinned.csv` compares the current load-all-then-store schedule with sequential 32-byte load/store pairs and two groups of four loads/stores. Both candidates pass the memory differential check, but neither improves forward overlap consistently across Zig/glibc, GCC/glibc, and Zig/musl. The pair schedule notably regresses disjoint destination-after at 64 KiB (Zig/glibc 1225 to 1359 ns; Zig/musl 1207 to 1286 ns). Group-of-four is less costly but has no repeatable aggregate win. Retain the current schedule.

`2026-10-21-memscan-avx512-alignment-pinned.csv` records the AVX-512 four-vector scan candidate against AVX2 and glibc at base offsets 0, 1, 7, 31, and 63. The candidate aligns the scan edge with scalar checks before using 256-byte groups of 64-byte masked compares. The AVX-512 threshold is 16 KiB: 4 KiB inputs regress due to alignment setup, 8 KiB results are mixed, and 16 KiB geometric-mean time improves 22–38% across the long-match and absent cases. A few middle-match offset pairs are near even or regress by up to 6%. At 32 KiB it cuts long scan time 35–43% from AVX2; at 64 KiB the reduction is 37–44%.

`2026-10-21-memscan-avx512-threshold-edge2-pinned.csv` is the five-round threshold sweep of the final assembly path, across offsets, sizes, and match positions. Keep the 16 KiB dispatch threshold: it captures about 31% geometric-mean improvement at 16 KiB while avoiding the middle-match regressions seen when dispatch starts at 8 or 12 KiB. Thresholds of 20 and 24 KiB delay the long-scan gains, improving the 8–24 KiB long-match/absent aggregate by only about 15% and 6% versus AVX2, respectively; the 8 KiB threshold improves that aggregate about 30% but has individual regressions up to 38% at 8 KiB. The final assembly also checks aligned first/last bytes before ZMM setup to keep immediate matches cheap.

`2026-10-21-memscan-avx512-toolchain-pinned.csv` compares runtime AVX-512, forced AVX2, and the matching host libc across Zig/glibc, GCC/glibc, Zig/musl, and TinyCC/glibc. Five pinned rounds show 30–39% lower 64 KiB scan times than AVX2 for long match and absent cases across all four builds. The runtime dispatch checks AVX-512F, AVX-512BW, OSXSAVE, and the required XCR0 ZMM state; AVX2 and SSE2 fallbacks remain available. `make smoke-zig`, `make smoke-gcc`, `make smoke-zig-musl`, `make smoke-tcc`, plus forced AVX2-only and SSE2 differential runs passed.

`2026-10-21-memscan-avx512-final-toolchain-pinned.csv` reruns the integrated 64 KiB matrix after the final edge-check change. Nelibc's 64 KiB `memchr` is 308–321 ns and `memrchr` is 293–320 ns across Zig/glibc, GCC/glibc, Zig/musl, and TinyCC/glibc. Against the same-build libc, this is 36–98% faster for `memchr` and 34–98% faster for `memrchr`; the musl baseline is especially slow in this static harness.

`2026-10-21-memchr-short64-zig-glibc-pinned.csv` records five pinned comparisons of the 64-byte SSE2 path before and after checking each 16-byte vector in order. Median latency fell 13–18% for first, middle, last, and absent matches. `2026-10-21-memchr-short64-toolchain-pinned.csv` records the integrated 64-byte shootout across Zig/glibc, GCC/glibc, Zig/musl, and TinyCC/glibc; `2026-10-21-memchr-short64-baseline-toolchain-pinned.csv` is its pre-change control. `make smoke-zig smoke-gcc smoke-zig-musl smoke-tcc` passed with the new path. Short calls remain slower than host glibc in some builds, so further tuning is still open.

`2026-10-21-memscan-branchless-probe-pinned.csv` compares five CPU-2-pinned rounds of the current SSE2 mask decode with a candidate that packs four 16-byte masks into a 64-bit scalar and uses `bsf`/`bsr`. The candidate passes the memory differential but regresses the 64-byte aggregate by 3.3% across Zig/glibc, GCC/glibc, Zig/musl, and TinyCC/glibc; `memchr` is +2.2% and `memrchr` +4.4%. Absent cases do not offset the slower hit paths. The exact rejected source is preserved at `benchmarks/candidates/2026-10-21-memscan-branchless-candidate.S`; production assembly remains unchanged.

`2026-10-21-memmove-backward-align32-pinned.csv` preserves the first inline alignment experiment, which changed unrelated code placement. The isolated follow-up above supersedes it.

`2026-10-21-memmove-64k-libc-direction-pinned.csv` preserves the pre-alignment comparison against matching glibc/musl builds. Its backward-overlap gap motivated the 32-byte destination-alignment experiment documented above; use that newer file for tuned results.

`2026-10-21-memmove-erms-vs-avx2-pinned.csv` compares a direct `rep movsb` helper with each toolchain's current AVX2 Nelua path over three pinned rounds. ERMS loses badly on all overlapping 64 KiB cases (about 14.4–14.8 us vs 0.75–1.00 us for AVX2), and does not improve disjoint moves consistently. Reject it as the general `memmove` path; retain direction-specific AVX2.

`2026-10-21-memmove-tcc-256-vs-512-integrated.csv` measures the actual TinyCC Nelua dispatch after adding a separate 512-byte helper entry. At 64 KiB, three pinned rounds showed 512 bytes improving forward overlap (0.75 vs 0.98 us) and disjoint destination-before (0.99 vs 1.23 us), while backward overlap was 1.3% slower and destination-after 1.5% faster. The combined median improves about 13%; TinyCC now uses the 512-byte path, while other compilers retain the 256-byte path. `smoke-tcc`, `smoke-zig`, `smoke-gcc`, and `smoke-zig-musl` passed with long move differential coverage.

`2026-10-21-memmove-block-128-256-512-pinned.csv` records three CPU-2-pinned rounds across Zig/glibc, GCC/glibc, Zig/musl, and TinyCC/glibc. It includes sizes around the 2 KiB dispatch and 64 KiB, with overlap and disjoint cases. The 128-byte loop loses clearly at 64 KiB. The 512-byte loop improves TinyCC by about 12% across the four 64 KiB cases, is near-even on GCC and musl, and is about 2% slower on Zig. The shared path remains 256 bytes; TinyCC has a separate 512-byte entry.

`2026-10-21-memmove-threshold-2k-vs-4k-pinned.csv` records three paired CPU-2-pinned rounds of 2 KiB vs 4 KiB AVX2 dispatch, with sizes from 2047 through 4097 bytes and all four overlap/disjoint placements. Keep the 2 KiB threshold: the median across tested 2–4095-byte cases was 18–54% faster than the SSE2 fallback for all four toolchain/libc pairs. Individual placements varied; TinyCC had one 5.7% regression at 3584 bytes. At 4096+ the thresholds converge because both select AVX2, so the 4 KiB threshold has no compensating win.

`2026-10-21-memmove-avx2-pinned.csv` records a fresh CPU-2-pinned shootout after adding a 2 KiB AVX2 bulk path to the x86-64 Nelua `memmove` helper. Long overlap correctness now covers 2047/2048/2049 bytes through 64 KiB. The candidate improved GCC/glibc 64 KiB backward overlap from the prior 2.07–2.52 us range to 0.98 us, but remains slower than glibc at 0.72 us; Zig/glibc measured 1.00 us vs 0.77 us, and TinyCC/glibc 0.99 us vs 0.88 us. Musl results were favorable (0.95 us vs 1.13 us). Results vary by move direction, so the CSV retains all four placement cases and sizes.

- `2026-10-21-current-*.csv` contains the latest sequential shootout run after
  the current `memchr`/`memrchr` and `memmove` tuning. At 64 KiB, Nelua's
  `memchr`/`memrchr` remain slower than the glibc baselines (3.06/3.94 us vs
  0.51/0.50 us with Zig, and 8.09/9.35 us vs 0.53/0.49 us with GCC). This
  keeps scan optimization on the open worklist; the CSVs include all sizes and
  both implementations for each toolchain/libc pair.

- `2026-10-21-64m-*.csv` captures the 64 MiB sample workload before the latest
  scan experiment.
- `2026-10-21-wordscan-*.csv` records a byte-packed word-mask experiment for
  `memchr`/`memrchr`. It regressed 64 KiB scans to 14–17 microseconds across the
  tested builds, so the candidate was reverted.
- `2026-10-21-bitmask-pinned-*.csv` and `2026-10-21-byte-scan-pinned-*.csv`
  compare the branch-reduced scan with the original while pinned to CPU 2.
- `2026-10-21-conditional-scan-pinned-*.csv` records the compiler-selected
  `memchr` path. At 64 KiB, the Zig compiler path improved from 8.02 to 2.86
  microseconds with glibc and from 8.00 to 3.00 microseconds with musl; GCC
  stayed near 8.1 microseconds on the original byte scan. `memrchr` retains its
  original scan because the bitmask candidate regressed it.
- Three additional pinned repeats confirmed the `memchr` result: medians were
  2.84 microseconds for Zig/glibc, 3.08 microseconds for Zig/musl, and 8.15
  microseconds for GCC/glibc, versus original byte-scan baselines of 8.02,
  8.00, and 8.21 microseconds. See `2026-10-21-conditional-scan-repeat*-pinned-*.csv`.
- `2026-10-21-memrchr-*-pinned-*.csv` compares reverse chunk sizes. The
  compiler-selected 2 KiB chunk scan is retained: three repeats measured
  medians of 4.04 microseconds with Zig/glibc and 3.85 microseconds with
  Zig/musl at 64 KiB, versus byte-scan baselines near 9 microseconds. GCC keeps
  the original reverse loop. The forward-recording and 256/1 KiB/4 KiB chunk
  candidates were slower than 2 KiB or the original in their measured cases.

- `2026-10-21-scalar-*.csv` records the original bytewise scans.
- `2026-10-21-unrolled-*.csv` records eight-byte direct comparisons.
- `2026-10-21-aggregate-*.csv` records the combined-difference experiment.
- The unprefixed files measure the current implementation: Clang uses direct
  comparisons and scalar `memmove`; GCC uses a 32-byte move block; other
  compilers use the combined-difference block and eight-byte `memmove` copies.
  Scan paths use eight-byte `memchr` and `memrchr` scans and `memmem`'s
  first-byte search through `memchr`.
- `2026-10-21-before-memmove-unroll-*.csv` and
  `2026-10-21-memmove-unrolled-*.csv` preserve the overlap benchmark before
  and after unrolling, before compiler-specific selection was added.
- `2026-10-21-mempcpy-pinned.csv` records the `mempcpy` shootout pinned to CPU
  2 across Zig/glibc, GCC/glibc, and Zig/musl.

Each file reports a libc baseline and the Nelua implementation linked into the
same C harness. The compiler and libc are labeled separately. These timings
are machine-specific and are not a substitute for correctness tests; CPU
frequency, scheduling, and cache state were not controlled.

`2026-10-21-memmem-twoway-shootout.csv` adds 32-, 128-, and 256-byte tail
matches and periodic absent matches across GCC, Zig/glibc, Zig/musl, and
TinyCC. The Nelua Two-Way path passed differential checks through 512-byte
needles on all four builds. For long tail matches at 64 KiB, GCC/Zig Nelua
measured 0.51–1.07 microseconds versus 1.87–9.61 microseconds for libc;
TinyCC Nelua measured 2.17–4.71 microseconds versus 1.83–9.58 microseconds.
Periodic absent scans took about 0.19–0.26 milliseconds for Nelua and
0.21–0.52 milliseconds for libc, consistent with linear work over the haystack.
These measurements use 256 calls per sample and are machine-specific.

`2026-10-21-memmem-threshold-shootout.csv` extends the needle sweep to 4, 8,
12, 16, 24, 32, 64, 128, and 256 bytes. The current threshold of 8 is retained:
the 4-byte periodic absent case uses the short search and costs about 0.29 ms
for GCC/glibc Nelua, while the 8-byte Two-Way case costs about 0.19 ms. At
shorter non-periodic tail matches, the short search is also cheaper than the
Two-Way setup. TinyCC and musl results are recorded in the same file; this is
a workload-dependent cutoff, and these timings are machine-specific.

`2026-10-21-strstr-twoway-shootout.csv` records the Two-Way `strstr` shootout
for tail matches and periodic absent needles at 64 bytes, 4 KiB, and 64 KiB,
using needle lengths 4, 8, 16, 64, and 256. The follow-up
`2026-10-21-strstr-hybrid-pinned.csv` reruns that matrix pinned to CPU 2 with
the long-string `strlen` path below. The same harness times `strlen` from 0
through 64 KiB at four alignments and checks strings of lengths 0–512 ending
at a guard page.
Nelua now switches from its 128-byte SSE2 scan to page-safe 256-byte blocks
for long strings. Three CPU-pinned rounds across Zig/glibc, GCC/glibc,
Zig/musl, and TinyCC/glibc are in
`2026-10-21-strlen-hybrid-pinned.csv`. At 64 KiB, Nelua measures 0.81–0.89
microseconds versus 0.46–0.52 microseconds for glibc. The linked Zig/musl host
`strlen` baseline is about 22 microseconds in this harness. At 4 KiB Nelua
measures 45–65 ns versus 24–31 ns for glibc. Short-string results remain
compiler- and alignment-dependent: 31-byte Nelua calls measure 1.9–4.0 ns,
and 64-byte calls measure 2.1–4.8 ns; glibc measures 1.5–1.9 ns and
1.9–2.4 ns, respectively. Before the SSE2 helper, Nelua's scalar path
contributed roughly 20 microseconds to each long `strstr` search. In the
follow-up, 64 KiB tail matches for needles up to 16 bytes measure 1.7–1.9
microseconds on Zig, GCC, and TinyCC, near their glibc baselines. The 64- and
256-byte matches measure 1.4–1.8 microseconds on GCC/Zig and 3.3–3.8
microseconds on TinyCC. The 256-byte periodic absent case measures 0.19–0.25
milliseconds on GCC/Zig and 0.34 milliseconds on TinyCC, versus about
0.22–0.23 milliseconds for glibc. Linked musl remains slower and noisier at
0.55 milliseconds. These timings are machine-specific.

The pinned `mempcpy` shootout showed unusually high Zig/static-musl baseline
times. Disassembly explains the result: that binary's `mempcpy` is a
byte-at-a-time loop, while its `memcpy` uses Zig's optimized compiler runtime
implementation. Treat this as a comparison of the linked implementations, not
as an indication that musl `mempcpy` is generally slow.

The early direct-difference `memcmp` measurements above were superseded by the
32-byte Nelua scan and then the SSE2 helper described below. The newest
CPUID-dispatched AVX2 results are recorded separately after that history.

The byte-safe `memcpy` unroll first reduced GCC/glibc Nelua from 17 to about
2 microseconds at 64 KiB. A GCC-only 32-byte block now measures 1.18–1.20 us
at 64 KiB, 57–59 ns at 4 KiB, and 2.02 ns at 64 bytes, versus 2.09–2.10 us,
130–135 ns, and 4.51 ns before. GCC compiles the block to two SSE loads and
stores per loop. `2026-10-21-gcc-glibc-memcpy-unroll32-repeat1.csv` and
`...-repeat2.csv` preserve these runs. Zig/glibc remains near 0.98 us at 64
KiB; `memset` was already vectorized by GCC and showed no material change.
GCC `mempcpy` now copies up to 64 bytes inline instead of calling `memcpy`:
64-byte calls measure 1.84 ns versus 2.43–2.44 ns before and about 2.03 ns for
glibc. Larger calls keep using the shared `memcpy` implementation; 4 KiB and
64 KiB remain slower than glibc. The two runs are recorded in
`2026-10-21-gcc-glibc-mempcpy-small-inline-repeat1.csv` and `...-repeat2.csv`.

At 64 KiB, the GCC-only 32-byte `memmove` block improves GCC/glibc by 35–49%
across the four placement cases, measuring 1.08–1.60 microseconds versus
2.07–2.52 microseconds before. It compiles to two SSE loads and stores per
loop. The Clang path remains scalar because the earlier unroll was slower.
`2026-10-21-gcc-glibc-memmove-unroll32-repeat1.csv` and `...-repeat2.csv`
preserve the shootouts.

Reproduce with `make shootout-zig`, `make shootout-gcc`, and
`make shootout-zig-musl`.

For `memccpy`, `2026-10-21-before-memccpy-unroll-*.csv` preserves the scalar
implementation, `2026-10-21-memccpy-unrolled-*.csv` preserves the eight-byte
scan, and the unprefixed files measure the current `memchr` + `memcpy`
composition. On the 64 KiB case, the unrolled scan measured 14.4–15.0
microseconds for Nelua across Zig/glibc and Zig/musl; the composition measured
9.5–11.2 microseconds. The current libc baselines were 1.6–18.6 microseconds,
with Zig/musl notably noisy. The composition improved all three Nelua runs in
this sample, while still trailing libc. Results are machine-specific.

The 32-byte `memcmp` scan is recorded in the current unprefixed CSVs;
`2026-10-21-before-memcmp32-*.csv` preserves the preceding eight-byte path.
the prior Nelua-only equal-buffer medians were 1.63 microseconds with
Zig/glibc, 2.77 microseconds with GCC/glibc, and 1.64 microseconds with
Zig/musl. The subsequent shared SSE2 helper reduced those to about 1.5–1.6
microseconds.

`2026-10-21-memcmp-avx2-dispatch-pinned.csv` is the latest focused shootout,
produced by `make memcmp-shootout-matrix`. It has three CPU-pinned rounds for
64-byte, 4 KiB, and 64 KiB comparisons, with equal, first-, middle-, and
last-byte cases. Its three implementations are host libc, the SSE2 baseline,
and the current dispatch. The Nelua helper checks CPUID, OSXSAVE, XCR0, and
AVX2 support on the first call of at least 4 KiB, caches the result, and uses
AVX2 for those large buffers; other CPUs retain the SSE2 path. At 64 KiB, the
dispatch measures 0.98–1.01 microseconds across Zig, GCC, TinyCC, and Zig/musl,
versus 1.57–1.58 microseconds for the SSE2 baseline. It is near glibc's
0.99–1.02 microseconds on this host. At 4 KiB it measures 39–44 ns, versus
69–73 ns for SSE2 and 33–34 ns for glibc. The linked Zig/musl baseline remains
much slower at about 20 microseconds for 64 KiB. Long-buffer differential
coverage checks equal data and mismatch positions across offsets 0, 1, 7, and
15 through 64 KiB; `make smoke-matrix` and `make smoke-tcc` run that coverage.
These measurements are machine-specific.
`2026-10-22-memcmp-current-zig-glibc-pinned.csv` is a fresh CPU-2-pinned Zig
run of the same cases, including host, SSE2 baseline, and the pre-256-byte
AVX2 dispatch. It confirms that dispatch essentially matched glibc at 64 KiB
(983 ns vs 978 ns) and improves the SSE2 baseline from 1.54 us. At 4 KiB,
the prior dispatch measured 39 ns, 20% slower than glibc's 33 ns, while still
improving substantially over the 69 ns SSE2 baseline. This isolates a smaller
medium-size gap; the older global shootout's apparent 64 KiB gap was sample
noise.
`2026-10-22-memcmp-avx2-256-dispatch.csv` records the updated helper across
Zig/glibc, GCC/glibc, and Zig/musl. A 256-byte AVX2 unroll is selected for
4 KiB to below 32 KiB inputs; inputs at or above 32 KiB retain the 128-byte
loop. Three alternating
CPU-pinned 4 KiB runs improved equal-buffer medians from 41.31 ns to 31.84 ns.
At 16 KiB, three-run medians improved from 139.87 ns to 122.44 ns. At 32 and
48 KiB, the 256-byte loop was 2–4% slower, which sets the dispatch boundary.
The profile shootout measures 4 KiB equal comparisons at 29.79 ns for
Zig/glibc, 36.26 ns for GCC/glibc, and 29.45 ns for Zig/musl. At 64 KiB the
updated helper measures 984 ns, 1.00 us, and 953 ns, respectively. The long
memory differential check passes through 64 KiB for four base offsets and
mismatch positions.

`2026-10-21-avx2-memscan-pinned.csv` records three CPU-pinned rounds of
`memchr` and `memrchr` before lowering the SSE2 cutoff for short calls.
`2026-10-21-memscan-threshold-block-sweep-pinned.csv` compares the 2/4/8 KiB
AVX2 thresholds with 128- and 256-byte blocks across 1–64 KiB scans, four
match positions, and three CPU-pinned rounds. It selected a 2 KiB threshold
and 256-byte blocks: 4 KiB absent scans improved from 32–36 ns to 22–25 ns
for both directions, near glibc's 23–26 ns. The current cross-toolchain run is
`2026-10-21-avx2-memscan-final-pinned.csv`; reproduce it with
`taskset -c 2 make memscan-shootout-matrix`; reproduce the threshold sweep
with `taskset -c 2 make memscan-threshold-shootout`. The pinned scan files
compare glibc, static musl, and TinyCC/glibc against Nelua using the 64
MiB-per-sample memory harness. The AVX2 scanner uses 256-byte blocks from
2 KiB, then 128-byte AVX2
blocks for the remainder; it uses cached CPUID, OSXSAVE, XCR0, and AVX2
detection, with SSE2 fallback on unsupported CPUs. At 64 KiB, the original
three-round 128-byte-block run measured
0.49–0.52 microseconds for Nelua across all four builds, versus 0.47–0.57
microseconds for glibc. Under static musl, Nelua measures about 0.52
microseconds versus 14–17 microseconds for the linked host scans. At 4 KiB,
the 128-byte path measured 38–41 ns versus 23–29 ns for glibc; the 256-byte
candidate brings it to 23–33 ns across the four builds. In the latest
cross-toolchain run, 64 KiB scans measure 0.48–0.55 microseconds across the
four builds, near glibc's 0.48–0.53 microseconds; the linked musl scans remain
14–17 microseconds. With the 64-byte SSE2 cutoff, the final pinned run
measures 2.4–2.9 ns for Zig/GCC Nelua calls at 64 bytes, near glibc's
1.8–2.5 ns. `tools/memory_diff.c` checks long forward and
reverse scans across offsets 0, 1, 7, and 15, including vector and AVX2
dispatch boundaries and first/last match ordering. Those checks pass in the
Zig/glibc, GCC/glibc, Zig/musl, TinyCC/glibc, and forced-SSE2 fallback builds.
These results are machine-specific.

## Float parser shootout — 2026-10-21

`2026-10-21-float-noinit-shootout.csv` records the current float parser after
marking bigint scratch records `noinit`. The previous generated object zeroed
those 8 KiB arrays at each local declaration. In this run, GCC/glibc medians
were 398 ns (`strtof`), 498 ns (`strtod`), and 643 ns (`strtold`); Zig/glibc
measured 494/645/818 ns. These remain slower than glibc, while Zig/musl was
near its host parser at 473/618/769 ns. Each value is a seven-sample median;
repeat runs showed similar Nelua timings but different host-libc timings under
musl, so compare only within the same compiler and libc environment.

`2026-10-21-float-binary-scale-shootout.csv` measures power-of-two scaling by
exponentiation by squaring after bounded 512-bit chunks. `strtold` improved
14–16% versus the prior `float-noinit` run: GCC/glibc 643 to 542 ns,
Zig/glibc 818 to 704 ns, and Zig/musl 769 to 646 ns. Other Nelua parser
medians stayed within a few percent. The chunks preserve the x87 subnormal
range without constructing an unrepresentable `2^16384` factor.

`2026-10-21-float-shifted-subtract-shootout.csv` captures the fused shifted
compare/subtract implementation. Against the binary-scale run, Zig/glibc
`strtod` and `strtold` improved by about 9% and 7%; GCC/glibc regressed by
about 5% and 4%, while Zig/musl stayed within about 2%. The implementation is
retained for the primary Zig/glibc target; this compiler-sensitive trade-off
remains on the float performance worklist.

`2026-10-21-float-fused-append-shootout.csv` measures one-pass bigint digit
appends and small multiplications. Relative to the shifted-subtract run, the
Nelua medians fell across all three environments: about 12–17% for Zig/glibc,
2–5% for GCC/glibc, and 10–11% for Zig/musl. These are sequential shootout
runs, not a pinned A/B; the CSV preserves this run for later comparison.

`2026-10-21-float-precomputed-divisor-length.csv` preserves a rejected
quotient-loop candidate that computed shifted divisor length from a single
precomputed bit length. It improved GCC/glibc medians by 7–9%, but regressed
Zig/glibc by 9–12% and Zig/musl by 8–17%; the candidate was reverted.

`2026-10-21-float-speculative-subtract.csv` records a rejected speculative
subtract/add-back quotient candidate. It removed the separate compare pass but
was slower than the fused-append baseline in both repeats, especially for
`strtod`/`strtold` under GCC/glibc and Zig/musl. The compare-first implementation
was restored.

`2026-10-21-float-small-divisor-shootout.csv` records a specialized path for
1–2-limb divisors. It passed the float differential matrix but showed no
cross-target performance win: Zig/glibc was near baseline, GCC/glibc
`strtod`/`strtold` were slower, and musl `strtold` was slightly slower. The
specialized path was removed.

`2026-10-21-float-knuth-divide-repeat1.csv` and
`2026-10-21-float-knuth-divide-repeat2.csv` record a base-2^32 Knuth quotient
divider. It passes the float differential matrix and cuts nelibc `strtod` and
`strtold` medians by about 50–57% versus the fused-append reference across
Zig/glibc, GCC/glibc, and Zig/musl. Both repeats show the gain. Musl `strtof`
was about 1% slower in repeat 2 than repeat 1, but remained about 14% below
the reference. The quotient divider is retained, with the bitwise
shifted-subtraction loop as a bounded fallback.

`2026-10-21-float-tcc-shootout.csv` adds TinyCC/glibc to the float parser
shootout before the specialized digit loop. TinyCC/Nelibc measured 888 ns
(`strtof`), 864 ns (`strtod`), and 1047 ns (`strtold`) against 176 ns, 87 ns,
and 89 ns for the host parser. This exposed the TinyCC code-generation gap.
Rows use seven-sample medians of 4,000 calls.

`2026-10-21-float-inline-digit-shootout.csv` is the current four-way compiler
matrix after expanding bigint digit appends into the decimal and hexadecimal
parse loops. `2026-10-21-float-inline-digit-tcc-ab.csv` records five
alternating TinyCC baseline/candidate pairs. Comparing each run's Nelibc time
to its host-libc time gives median relative improvements of 3.5% for `strtof`,
2.7% for `strtod`, and 3.0% for `strtold`. The candidate passed all 1,242
floating conversion differential cases under Zig/glibc, GCC/glibc, Zig/musl,
and TinyCC/glibc. Other toolchain shootouts were flat or improved within this
run's noise.

`2026-10-21-float-long-decimal-shootout.csv` adds two roughly 600-digit decimal
inputs to the compiler/libc comparison. Nelibc measures about 14.1–16.4 us per call
with Zig/GCC and 46.9–48.1 us with TinyCC. On this host's static musl build,
host parsing is about 15.9–16.6 us, close to Nelibc. The 10-pair TinyCC
comparison in `2026-10-21-float-long-rational-compare-ab.csv` found the
no-copy exponent comparison candidate within roughly 1.5% of baseline in either
direction, so the simpler copy-and-shift code remains.

`2026-10-21-float-knuth-divmod-ab.csv` records 20 alternating long-decimal
baseline/candidate pairs for TinyCC, GCC, and Zig cc. Deriving the Knuth
remainder from its quotient removes a second 64-bit divide in TinyCC output.
Host-normalized TinyCC medians improve 3.4% for `strtod` and 4.8% for
`strtold`; `strtof` is within noise. GCC and Zig cc results are near flat with
some noisy regressions. The candidate passes all 1,244 floating conversion
cases on Zig/glibc, GCC/glibc, Zig/musl, and TinyCC/glibc. Its normal and long
matrix shootouts are in `2026-10-21-float-knuth-divmod-shootout.csv` and
`2026-10-21-float-knuth-divmod-long-shootout.csv`.

`2026-10-21-float-knuth-borrow-ab.csv` records 20 alternating long-decimal
pairs for a branchless limb borrow calculation, compared with the retained
branching subtraction. Paired host-normalized medians are nearly flat under
TinyCC and Zig cc, and improve 0.7% for GCC `strtod` and 2.2% for GCC
`strtold`. Raw Nelibc times improve 0.3–1.8% across the three parsers under
GCC, and stay within 1% under TinyCC/Zig cc. The change also removes a data
dependent branch in every Knuth limb iteration. The candidate passes the
1,244-case float differential matrix. Its compiler/libc shootouts are in
`2026-10-21-float-knuth-borrow-shootout.csv` and
`2026-10-21-float-knuth-borrow-long-shootout.csv`.

`2026-10-21-float-knuth-underflow-frequency.csv` records temporary counters for
the Knuth quotient-estimate decrement and add-back paths. Across 100,000
randomized 80–300-digit decimals per parser, the estimate decremented 47 times
for `strtof`, 35,896 times for `strtod`, and 87,626 times for `strtold`, across
TinyCC/glibc, GCC/glibc, Zig/glibc, and Zig/musl. The two fixed long inputs
did not fire in any of these calls, so it remains conditional.

`2026-10-21-float-knuth-estimate-unroll-ab.csv` records a rejected unroll of
the quotient-estimate correction loop. Twenty paired TinyCC long-input runs
regressed raw Nelibc time by 2.7% for `strtof`, 3.4% for `strtod`, and 1.6%
for `strtold`; host-normalized medians also regressed. Cross-toolchain results
were mixed, so the original conditional loop remains. The candidate passed the
float differential matrix. Its standard and long shootouts are in
`2026-10-21-float-knuth-estimate-unroll-shootout.csv` and
`2026-10-21-float-knuth-estimate-unroll-long-shootout.csv`.

`2026-10-21-float-knuth-product-reuse-ab.csv` records a rejected attempt to
update the estimate product by subtraction after each quotient decrement.
Twenty paired TinyCC long-input runs regressed all three parsers by about
2.7–3.2% in raw Nelibc time. The four-way matrices also showed consistent
`strtold` regressions, so direct multiplication remains in the loop. The
standard and long candidate shootouts are in
`2026-10-21-float-knuth-product-reuse-shootout.csv` and
`2026-10-21-float-knuth-product-reuse-long-shootout.csv`.

`2026-10-21-float-knuth-packed-compare-ab.csv` records a rejected split
high/low comparison intended to avoid packing the right-hand side into 64 bits.
TinyCC emitted extra compare branches and spills; paired long-input runs
regressed raw time by 3.8–4.8% across all three parsers. The packed comparison
remains. The candidate's standard and long matrices are in
`2026-10-21-float-knuth-packed-compare-shootout.csv` and
`2026-10-21-float-knuth-packed-compare-long-shootout.csv`.

`2026-10-21-float-knuth-highlimb-ab.csv` records a candidate that calculated
the high-limb difference once before the underflow branch. It passed the float
differential matrix, but 20 paired TinyCC runs were essentially flat with
small `strtof`/`strtod` regressions; compiler matrices were mixed. The original
branched high-limb assignment remains. Candidate matrices are in
`2026-10-21-float-knuth-highlimb-shootout.csv` and
`2026-10-21-float-knuth-highlimb-long-shootout.csv`.

`2026-10-21-float-divide-round-inplace-ab.csv` records in-place doubling of
the division remainder for midpoint rounding, removing the copied BigUInt.
TinyCC's `big_divide_round` stack frame shrinks from 8,304 to 96 bytes. Twenty
paired long-input runs stay within 1% raw time across TinyCC, GCC, and Zig cc.
The change passed the 1,244-case float differential matrix and full smoke
matrix. Its standard and long shootouts are in
`2026-10-21-float-divide-round-inplace-shootout.csv` and
`2026-10-21-float-divide-round-inplace-long-shootout.csv`.

`2026-10-21-float-tcc-bigmul-helper-ab.csv` records a rejected attempt to
expand `big_mul_small` inside `big_mul_power5`. Ten alternating, host-normalized
TinyCC pairs gave 1.0% improvement for `strtof`, 2.0% regression for `strtod`,
and 1.7% improvement for `strtold`; this was not a consistent win, so the
generic helper remains. The experimental source passed the float differential
matrix before being reverted.

`2026-10-21-float-tcc-optimization-flags.csv` records the pre-specialized
`make float-shootout-tcc-opt` run, which rebuilt both the Nelua object and C
harness under TinyCC `-O0` through `-O3`. Nelibc medians ranged from 880–906 ns for
`strtof`, 853–870 ns for `strtod`, and 1022–1061 ns for `strtold`; no level
showed a consistent improvement. The four Nelua objects were byte-identical.
TinyCC 0.9.28rc documents `-On` as setting `__OPTIMIZE__` for `n > 0`; with
the same benchmark label, the `-O0` and `-O3` executables were also byte-
identical. The parser slowdown therefore needs source/code-generation work,
not a different `-O` level.
`2026-10-22-float-tcc-deferred-biguint-shootout.csv` records five repeated
TinyCC/glibc runs after deferring BigUInt construction until a parsed digit no
longer fits the 64-bit significand. The strict 1,250-case float differential
suite passes for Zig/glibc, GCC/glibc, Zig/musl, and TinyCC/glibc. Against the
`2026-10-22-float-shootout.csv` baseline, host-normalized Nelibc time improves
by about 4% for `strtof`, 9% for `strtod`, and 6% for `strtold`. The new
four-way matrix is `2026-10-22-float-deferred-biguint-matrix.csv`. Relative to
its same-day baseline, Zig/glibc and GCC/glibc improve 7–13% across the three
parsers. The first musl run showed a `strtod` slowdown, but seven repeated
pairs in `2026-10-22-float-musl-deferred-biguint-shootout.csv` are flat: the
Nelibc/host median ratios are 1.005 for `strtof`, 0.994 for `strtod`, and 0.977
for `strtold`.
`2026-10-22-float-one-limb-divide-candidate-matrix.csv` records a rejected
specialized division path for small numerators and one-limb denominators.
`2026-10-22-float-radix-cache/` records the subsequent optimization that caches
the locale radix before scanning input digits. Eight paired Zig/glibc and
TinyCC/glibc runs show 23–50% lower parser medians; the four-profile float
differential matrix passes.
Results are mixed: TinyCC `strtof`/`strtold` regress, GCC `strtod` improves, and
most other changes are within noise. The general Knuth divider remains.
`2026-10-22-float-power5-square-tcc-ab.csv` contains 12 alternating TinyCC
baseline/candidate pairs for exponentiation-by-squaring when building small
powers of five. Raw Nelibc medians were flat to 0.5% slower, despite apparent
host-normalized gains; the candidate matrix is
`2026-10-22-float-power5-square-matrix.csv`. The generic multiply path remains.
`2026-10-22-float-32x64-divide-tcc-ab.csv` tests a specialized normalized
3-limb/2-limb Knuth quotient step in 12 alternating TinyCC pairs. Median
host-normalized changes were -1.6% for `strtof`, -0.9% for `strtod`, and +0.5%
for `strtold`, so the common case remains on the generic Knuth divider. The
four-way candidate matrix is `2026-10-22-float-32x64-divide-candidate-matrix.csv`.
`2026-10-22-float-virtual-remainder-compare-{tcc,gcc,zig-musl}-ab.csv` tests
replacing remainder doubling with a virtual shifted-limb compare. Twelve
alternating TinyCC pairs improve host-normalized `strtof`, `strtod`, and
`strtold` by 2–3%; GCC regresses 1–2%, while musl is near-flat. The optimization
is therefore enabled for TinyCC only. All 1,250 float differential cases pass
on all four builds. The final four-way matrix is
`2026-10-22-float-virtual-remainder-compare-matrix.csv`.
Eight additional TinyCC pairs on roughly 600-digit inputs show raw median
improvements of 0.3% (`strtof`), 0.7% (`strtod`), and 1.3% (`strtold`); results
are in `2026-10-22-float-virtual-remainder-long-tcc-ab.csv`.

`2026-10-21-tcc-glibc-mem-shootout.csv` is the pre-optimization TinyCC/glibc
baseline. `2026-10-21-tcc-glibc-mem-rep-movsb.csv` records the first x86-64
`rep movsb` helper pass; `...-repeat1.csv` and `...-repeat2.csv` are two runs
with small `memmove`/scan calls kept on the Nelua paths. Across both repeats,
TinyCC/Nelibc 64 KiB `memcpy` is about 1.06 us and `memset` 0.71–0.75 us,
close to glibc at 1.05 and 0.72 us. At 4 KiB, those functions are about 23–24
and 21–22 ns versus 20 and 21 ns for glibc. The 64-byte SSE2 `memmove` unroll
measures 1.27–1.31 us at 64 KiB (glibc: 0.74–0.78 us), 62–66 ns at 4 KiB
(glibc: 35–37 ns), and 5.0–5.3 ns at 64 bytes. It improves the prior
one-vector loop by about 4–11% at 64 KiB and 31–39% at 4 KiB. A 128-byte
candidate regressed large calls to 1.31–1.36 us; a `rep movsb` candidate
regressed them to 14.6–14.9 us,
so both were discarded. The current TCC x86-64 path uses SSE2 helpers in
`memory_x86_64.S`, assembled with Zig cc and linked with TinyCC-generated
objects. The earlier 128-byte `memcmp` loop measured 1.69–1.74 us at 64 KiB and
93 ns at 4 KiB, versus 1.87 us and 98 ns for the 64-byte loop. The larger
loop cost about 5.1 ns at 64 bytes versus 4.4–4.5 ns for the 64-byte loop;
glibc measured 2.7 ns. A four-vector scan with an OR-reduced no-match check now
measures 1.00–1.01 us for both `memchr` and `memrchr` at 64 KiB, versus glibc's
0.47–0.54 us. This is about twice as fast as the prior `memchr` and 40–45% faster
than the one-vector `memrchr`; at 4 KiB both scans take 67–69 ns versus glibc's
25–30 ns. At 64 bytes, calls take 3.7–3.9 ns versus glibc's 2.2–2.6 ns. Before
the SSE2 paths, TCC scan calls took 40–46 ns. The fused
SSE2 `memccpy` path measures 2.25–2.30 us at 64 KiB, 156–157 ns at 4 KiB,
and 7.0–7.2 ns at 64 bytes; glibc measures 1.71 us, 54.8 ns, and 4.8 ns.
This improves on both the earlier fused C candidate and the two-pass
scan-and-copy implementation. TCC smoke passes, and Zig/GCC/musl builds
succeed.
`memory_x86_64.S` contains x86-64 SSE2 helpers used by TinyCC and by GCC/Zig cc
`memcmp`, `memchr`, and `memrchr` paths; other compiler targets keep the Nelua
implementation.

The earlier mismatch-position rows show where the wider TCC compare loop exits:
at 64 KiB, first-byte mismatch takes 2.7–2.8 ns, middle mismatch 0.77–0.81 us,
and last-byte mismatch 1.57–1.59 us. The corresponding glibc times are about
2.4 ns, 0.51 us, and 0.98–1.02 us. At 4 KiB, middle mismatch takes 51–52 ns
for Nelibc vs 17–20 ns for glibc. The 128-byte unroll improves equal-buffer
time, while early and middle mismatches remain slower than glibc.

`2026-10-21-gcc-glibc-memcmp-compact-fallback-repeat1.csv` and
`...-repeat2.csv` record the GCC-only compact ordered fallback for a differing
32-byte block. Equal 64 KiB comparisons measured 2.77 us in both repeats,
versus 0.97–1.02 us for glibc; middle and last-byte mismatches measured
1.37–1.40 us and 2.74–2.81 us. This was a small equal-buffer improvement over
the earlier 2.85 us result and removed the large stack spill frame generated
by explicit ordered checks. A 64-byte GCC block regressed equal comparisons
to about 15 us and was discarded.

`2026-10-21-gcc-glibc-memcmp-sse2-repeat1.csv` and `...-repeat2.csv` record the
initial x86-64 GCC SSE2 path. The paired
`2026-10-21-gcc-glibc-memcmp-and128-firstfast-repeat1.csv` and
`...-repeat2.csv` records show the adopted 128-byte mask aggregation:
it checks the first vector immediately, combines equality masks for the next
112 bytes, and rescans in address order only on a mismatch. GCC equal-buffer
64 KiB time is 1.55–1.64 us, similar to the previous helper; at 4 KiB it falls
from 90–94 ns to 67–68 ns. First-byte mismatches remain around 1.2–1.5 ns.

`2026-10-21-tcc-glibc-memcmp-and128-firstfast-repeat1.csv` and
`...-repeat2.csv` record the same adopted helper in TinyCC. Equal 64 KiB time
falls from the previous 1.79–1.81 us to 1.51–1.58 us; at 4 KiB it falls from
94–98 ns to 68–69 ns. First-byte mismatch remains about 2.6–2.8 ns. Both
compiler paths retain bytewise ordered mismatch results and pass memory
differential tests, guard-page checks, ABI smoke, and freestanding/standalone
link checks where applicable. Zig keeps the Nelua path below 128 bytes and uses
the shared SSE2 helper for larger comparisons.
`2026-10-21-gcc-glibc-memcmp-and128-candidate-repeat1.csv` and
`...-repeat2.csv` preserve the first version, which improved bulk scans but
made first-byte mismatches slower; the adopted version checks that vector
before aggregation. The paired
`2026-10-21-tcc-glibc-memcmp-and128-firstfast-candidate-repeat1.csv` and
`...-repeat2.csv` are pre-integration shootouts;
the final TCC records above were collected after integration.
`2026-10-21-gcc-glibc-memcmp-group64-candidate-repeat1.csv` and
`...-repeat2.csv` preserve a candidate that combines four SSE2 equality masks
before branching. Equal 64 KiB times were 1.616 us in both runs, tied with the
current helper within benchmark noise; its 64-byte call cost increased to
about 3.8 ns. The grouped candidate was discarded.

`2026-10-21-zig-glibc-memcmp-sse-threshold-repeat1.csv` and
`...-repeat2.csv`, plus the corresponding `zig-musl` files, record the helper
with the 128-byte threshold. On glibc, equal 4 KiB comparisons take 67.5–67.8
ns (previous Nelua path: 75–76 ns) and equal 64 KiB comparisons take
1.53–1.55 us (previous: about 1.65 us). On musl, equal 4 KiB comparisons take
65–69 ns (previous: 70–71 ns), and equal 64 KiB comparisons take 1.50 us
(previous: 1.62–1.67 us). 64-byte calls keep the Nelua path. Full Zig glibc and
musl smoke matrices pass with the helper linked into host, freestanding, and
standalone targets.

`2026-10-21-gcc-glibc-memchr-memrchr-sse128-repeat1.csv` and
`...-repeat2.csv` plus the equivalent `tcc-glibc`, `zig-glibc`, and `zig-musl`
files record routing x86-64 GCC/Zig cc calls of 128 bytes or more through the
shared SSE2 scanners. GCC equal 64 KiB scans fall from roughly 8–9 us to
0.96–1.12 us for `memchr` and 0.96–1.03 us for `memrchr`. Zig/glibc falls from
2.77 us and 3.88 us to 0.95–0.97 us and 0.98–0.99 us. At 4 KiB, Zig/glibc
`memchr` improves from about 186 ns to 67–69 ns, and `memrchr` from about
732 ns to 66–68 ns. Calls below 128 bytes keep their existing Nelua paths.
The full compiler/libc smoke matrix passes, including guard-page differential
checks for the shared assembly helpers.

`2026-10-21-tcc-glibc-mem-qword-memmove-repeat1.csv` and
`...-repeat2.csv` record the intermediate word-sized backward `memmove`.
`2026-10-21-tcc-glibc-mem-word-scan-repeat1.csv` and
`...-repeat2.csv` record the 64-bit scan implementation. Its TCC helper is
compiled with `-O2` for the shootout, matching the benchmark's optimization
level.
`2026-10-21-tcc-glibc-mem-wordscan-unroll4-repeat1.csv` and
`...-repeat2.csv` record the intermediate four-word C unroll. The current SSE2
path supersedes that C loop. `2026-10-21-tcc-glibc-mem-wordscan-unroll2-candidate-
repeat1.csv` and `...-repeat2.csv` preserve the rejected two-word/16-byte
unroll, which was 5–15% slower in large scans.
`2026-10-21-tcc-glibc-mem-sse2-scan-repeat1.csv`/`...-repeat2.csv` and
`2026-10-21-tcc-glibc-mem-sse2-all-sizes-repeat1.csv`/`...-repeat2.csv`
record the SSE2 implementation, before and after removing the small-size
thresholds. `2026-10-21-tcc-glibc-mem-sse2-memmove-repeat1.csv` and
`...-repeat2.csv` record the SSE2 `memmove` replacement.
`2026-10-21-tcc-glibc-mem-sse2-memmove-unroll64-repeat1.csv` and
`...-repeat2.csv` record the retained 64-byte unroll. The larger unroll and
`rep movsb` candidates are preserved in their correspondingly named CSVs.
The newer `memmove` rows separate backward overlap, forward overlap, and
disjoint destinations on either side of the source. Across two TCC repeats at
64 KiB, Nelibc takes 1.35–1.40 us for overlap and 1.51–1.72 us for disjoint
moves; glibc takes 0.75–0.76 us and 1.10–1.12 us. The GCC/glibc Nelua path
remains the largest gap at 2.07–2.52 us across the four cases. All compiler and
libc results are in the `2026-10-21-memmove-modes-*-repeat*.csv` files.
`2026-10-21-tcc-glibc-mem-sse2-memcmp-unroll64-repeat1.csv` and
`...-repeat2.csv` record the intermediate 64-byte comparison loop.
`2026-10-21-tcc-glibc-mem-sse2-memcmp-unroll128-repeat1.csv` and
`...-repeat2.csv` record the current 128-byte loop and its small-call tradeoff.
`2026-10-21-tcc-glibc-memccpy-threshold-repeat1.csv` and `...-repeat2.csv`
record the small-call C specialization. `2026-10-21-tcc-glibc-memccpy-fused-
candidate.csv` preserves the rejected all-sizes fused C path.
`2026-10-21-tcc-glibc-mem-sse2-memccpy-repeat1.csv` and `...-repeat2.csv`
record the current fused SSE2 `memccpy` implementation.
`2026-10-21-tcc-glibc-memcmp-position-repeat1.csv` and
`...-repeat2.csv` record the TCC position-specific rows.
`2026-10-21-tcc-glibc-mem-sse2-scan-or64-repeat1.csv` and
`...-repeat2.csv` record the four-vector OR-reduced scan. It checks a combined
mask on no-match blocks, then examines per-vector masks in address order only
when a target was found.
`2026-10-21-memcmp-position-zig-gcc-musl.csv` records the updated benchmark
across Zig/glibc, GCC/glibc, and Zig/musl.

`2026-10-21-tcc-simple-loop-candidate.csv` preserves a rejected TCC-specific
byte-loop version of `memcpy`, `memmove`, `memset`, and `memcmp`. It passed
`smoke-tcc`, but regressed `memcpy` by 10–15% and roughly doubled `memset` and
`memcmp` times at 64 bytes; the unrolled source was restored.

`2026-10-21-float-rational-input-copy-ab.csv` records 20 alternating long-input
comparisons after removing two redundant BigUInt input copies in
`exact_rational_to_double`. This saves 16,400 bytes from TinyCC's function
stack frame (49,440 to 33,040 bytes); long-input timing is flat to slightly
slower across the compiler/libc pairs, so the change is retained for the
substantial stack reduction. The candidate passed the 1,244-case float
matrix. Standard and long shootouts are in
`2026-10-21-float-rational-input-copy-shootout.csv` and
`2026-10-21-float-rational-input-copy-long-shootout.csv`.

`2026-10-21-float-rational-virtual-compare-ab.csv` records 10 alternating
comparisons for a virtual shifted-limb comparator in `exact_rational_to_double`.
It removes the two 16 KiB comparison BigUInt temporaries. TinyCC's stack frame
falls another 16,400 bytes (33,040 to 16,640); the long-input timing is mixed,
from about 3% faster on GCC to 3% slower on TinyCC, while other results are near
flat. The change passed all 1,244 float differential cases and is retained for
the stack reduction. Candidate standard and long shootouts are in
`2026-10-21-float-rational-virtual-compare-shootout.csv` and
`2026-10-21-float-rational-virtual-compare-long-shootout.csv`.

`2026-10-21-float-rational-scale-inplace-ab.csv` records 10 alternating
comparisons after scaling the caller-owned numerator/denominator in place in
`exact_rational_to_double`. Both call sites discard these temporary values
immediately after conversion. TinyCC's stack frame falls from 16,640 bytes to
240 bytes; timing is near flat with mixed small changes across compiler/libc
pairs. The change passed the 1,244-case float differential matrix and is
retained for its stack reduction. Candidate standard and long shootouts are in
`2026-10-21-float-rational-scale-inplace-shootout.csv` and
`2026-10-21-float-rational-scale-inplace-long-shootout.csv`.

`2026-10-21-float-parser-input-inplace-rejected-ab.csv` records a rejected
change that removed the parser-to-converter BigUInt copy in
`exact_decimal_to_double` and `exact_binary_to_double`. Although it reduced
TinyCC's frame by about 8 KiB per function, 30 alternating GCC rounds showed a
repeatable 3–7% slowdown, so the caller-owned parser values are copied as
before. Candidate standard and long shootouts are retained in
`2026-10-21-float-parser-input-inplace-rejected-shootout.csv` and
`2026-10-21-float-parser-input-inplace-rejected-long-shootout.csv`.

`2026-10-21-float-knuth-normalize-inplace-ab.csv` records 20 alternating
comparisons for in-place normalization in `big_divide_knuth_u64`. It reuses the
caller-owned numerator and denominator, which are both local conversion
scratch values, and leaves them normalized together so remainder rounding and
the quotient fallback use the same scale. Prechecks return before mutation
when normalization would overflow or the quotient would exceed 64 bits. The
candidate passed the 1,244-case float differential matrix. Standard-input
latency improved 9–22% across all four compiler/libc pairs; long-input latency
was near flat or improved up to 4%. TinyCC's divider frame fell from 16,688
to 320 bytes. Candidate standard and long shootouts are in
`2026-10-21-float-knuth-normalize-inplace-shootout.csv` and
`2026-10-21-float-knuth-normalize-inplace-long-shootout.csv`.

The float differential suite now includes decimal and hexadecimal values around
the 80-bit long-double midpoint immediately below 2 (predecessor, tie, and
above-tie cases). The expanded 1,250-case matrix passes on Zig/glibc,
GCC/glibc, Zig/musl, and TinyCC/glibc.

`make float-divide-precheck-matrix` now directly exercises the internal Knuth
quotient precheck at `2^64 - 1`, `2^64`, and `2^64 + 1` using divisors that
exercise normalization shifts of 0, 1, 30, and 31. The quotient-boundary
cases use divisor 3 and a normalized three-limb numerator. The below-limit
case verifies the quotient, zero remainder, and normalized divisor; both
rejected cases verify that the numerator and denominator remain unchanged for
the bitwise fallback. A second case reaches the 2,048-limb storage boundary
with a quotient that fits, confirming that the capacity rejection also leaves
every used limb unchanged. This
probe passes on Zig/glibc, GCC/glibc, Zig/musl, and TinyCC/glibc. It compiles a
test-only source copy with an internal wrapper, leaving the production ABI
unchanged.

`2026-10-22-waitpid-shootout.csv` compares host and Nelibc
`waitpid(-1, NULL, WNOHANG)` when no child exists. Seven samples of 200,000
calls show the cancellation-aware wrapper adds 2.35–6.50 ns per call
(1.6–4.5%) across Zig/glibc, GCC/glibc, Zig/musl, and TinyCC/glibc. This
isolates the immediate `ECHILD` path; it does not measure blocked waits or
cancellation delivery.

`2026-10-22-eventfd-shootout.csv` measures seven samples of 100,000
eventfd create/close operations and counter write/read pairs under the same
four compiler/libc combinations. In this run Nelibc create/close measured
2–5% slower on all four builds. Counter round trips were 2% faster with
Zig/musl and 2–10% slower on glibc. Across repeated runs, create/close varied
more than the round-trip case. The helper benchmark includes Nelibc's
cancellation-aware `read`/`write` wrappers; removing that public-function
delegation did not produce a consistent improvement across toolchains, so the
composed implementation remains.

`2026-10-22-process-id-shootout.csv` measures `getpid`, `getppid`, and
`gettid` over seven samples of 500,000 calls. Nelibc is within a few percent
of host on all three calls under glibc and on `getpid`/`getppid` under musl.
Musl's host `gettid` is about 1 ns versus 96 ns for Nelibc; inspection of the
static host binary shows musl reads the TID from its thread control block,
while Nelibc uses the raw syscall. This exposes an optimization opportunity;
thread lifetime and fork behavior need care before adding a Nelibc cache.
Nelibc's `fork` uses a direct syscall and cannot rely on host `pthread_atfork`
handlers; the syscall smoke now checks that the child immediately reports
`gettid() == getpid()`.

`2026-10-22-getentropy-shootout.csv` measures seven samples of 100,000 calls
requesting 32 and 256 bytes. Both sizes are within 5% of host across the
matrix: the 32-byte case ranges from 4% slower to 2% faster, and the 256-byte
case from 2% slower to 2% faster. The implementation uses the raw `getrandom`
syscall to keep `getentropy` out of the cancellation-point path.

`2026-10-22-dirent-shootout.csv` measures raw directory open/close,
`opendir`/`closedir`, a 64-entry scan on a reused stream, and a full
`opendir`/`readdir`/`closedir` scan over 64 cached entries. Each case uses
seven samples of 1,000 calls. The reused-stream scan is 12–19% faster with
Nelibc across the four compiler/libc pairs. Full scans are 10–19% faster in
this run. `opendir`/`closedir` alone is 8–15% faster. `fdopendir` validates the descriptor
with `F_GETFL` and `fstat` without consuming directory entries, and `opendir`
uses its already validated `O_DIRECTORY` descriptor directly. The stream and
its 8 KiB `getdents64` buffer share one allocation. The kernel guarantees
NUL-terminated names in `getdents64` records, so `readdir` validates record
bounds without rescanning each name.

`2026-10-22-stdio-locking-baseline.csv` records the initial host-versus-Nelibc buffered `putchar`, `puts`, `fputc`, and `fputs` shootout, with stdout directed to `/dev/null`. `2026-10-22-stdio-shootout.csv` records the rerun after adding a glibc single-thread lock fast path. Each row is the median of seven 200,000-call samples. On Zig/GCC glibc, the fast path cut Nelibc `putchar` and `fputc` latency by 18–26%; TinyCC changed by 0–6%. These wrappers remain 1.7–2.7 times host latency. Static musl has no matching public single-thread indicator, so it retains FILE locks; Nelibc `putchar` and `fputc` remain about 11.5 times host latency there.

`2026-10-22-io-shootout.csv` compares host and Nelibc scalar `read`/`write`,
`readv`/`writev` with one, two, and four one-byte vectors using `/dev/zero` and
`/dev/null`, plus `pread`/`pwrite`, `preadv`/`pwritev`, and flags-zero
`preadv2`/`pwritev2` on an unlinked temporary file. Each row is the median of
seven 200,000-call samples. Reproduce with `make io-shootout-matrix`; syscall
error and behavior coverage is included in the four-way `make smoke-syscall-*`
targets, including `RWF_APPEND` for `pwritev2`.
This run includes the hybrid pthread cancellation bracket around Nelibc I/O;
Nelibc measured 0.9–12.9% slower than host wrappers across these cases.

`2026-10-22-strverscmp-shootout.csv` compares host and Nelibc version-string
ordering with Zig/glibc, GCC/glibc, and Zig/musl. Each result is the median of
seven one-million-call samples over eight version-like string pairs, after
checking expected ordering for numeric runs and leading zeros. Nelibc measured
5.90 ns/call vs 7.73 ns for Zig/glibc, 5.31 vs 7.88 ns for GCC/glibc, and
5.95 vs 5.35 ns for Zig/musl; TinyCC measured 24.67 vs 8.90 ns. Reproduce
with `make strverscmp-shootout-matrix`.

`2026-10-22-strverscmp-digit-checks.csv` records a later matrix after replacing
the repeated `nelibc_is_digit()` calls in `strverscmp` with direct byte-range
checks. TinyCC/Nelibc measured 18.46 ns/call against 8.80 ns for host glibc,
down from the previous recorded 24.67 ns; Zig/glibc and GCC/glibc remained
faster than host, while Zig/musl remained slightly slower. Timings vary between
runs; reproduce with `make strverscmp-shootout-matrix`.

`2026-10-22-sysconf-shootout.csv` records seven-sample comparisons for 40
fixed and dynamic selectors under Zig/glibc, GCC/glibc, Zig/musl, and
TinyCC/glibc. Fixed
values and stable limits match glibc. `_SC_AVPHYS_PAGES` is live: Nelibc counts
free plus buffered RAM, so its result can differ slightly from another libc's
sample. Musl returns different values for
implementation-specific limits such as `_SC_ARG_MAX`, `_SC_NGROUPS_MAX`, and
thread limits; these differences are recorded per host. Nelibc's resource-limit
queries are 6–15% faster here. CPU count queries match all three hosts at 16
CPUs. The affinity-based Nelibc query takes about 0.24 us with Zig, roughly ten
times faster than glibc's sysfs query. The TinyCC build returns matching values
and is slower per call than Zig/GCC in these fixed-limit paths. Reproduce with
`make sysconf-shootout-matrix`.


`2026-10-22-float-knuth-limb-profile-tcc.txt` records instrumented quotient
digit and multiply/subtract limb counts for TinyCC's short and 600-digit float
benchmarks. Long inputs exercise denominator lengths from 32 limbs upward and
perform 306,800 multiply/subtract limb iterations across the benchmark; no
add-backs or bitwise fallbacks occurred. The short-input profile is dominated
by denominators under 16 limbs. The counters are diagnostic and the instrumented
timings are not comparable to the normal shootouts.

`2026-10-22-float-knuth-limb-unroll2-tcc-ab.csv` and
`2026-10-22-float-knuth-limb-cached-pointers-tcc-ab.csv` contain alternating
TinyCC shootouts for two rejected loop candidates. Two-limb unrolling slowed
the short cases and had mixed long-input results. Caching the limb-array
pointers measured within 0.9% of baseline across all cases in 12 pairs, with
no consistent benefit. Neither candidate is retained. Both candidates passed
the TinyCC float differential cases before timing.

`2026-10-22-float-knuth-borrow32-matrix-ab.csv` records 12 alternating
baseline/candidate pairs for replacing Knuth's 64-bit subtraction and borrow
extraction with a 32-bit subtraction and explicit comparisons. The candidate
passed the float differential corpus on TinyCC/glibc, Zig/glibc, GCC/glibc,
and Zig/musl, but timing was mixed across the four builds and both input sizes;
it is rejected. The summary is in
`2026-10-22-float-knuth-borrow32-profile.txt`.

`2026-10-22-float-knuth-tcc-codegen.txt` records the TinyCC disassembly finding
for the Knuth divider: its per-limb loop spills most temporaries to stack slots.
It also documents a rejected candidate that narrowed carry/borrow state to 32
bits while preserving 64-bit subtraction. The candidate passed the float
corpus on all four compiler/libc combinations, but the 12-pair matrix in
`2026-10-22-float-knuth-narrow-carry-matrix-ab.csv` shows compiler-dependent
regressions, including 1.9% on long TinyCC inputs and 1.9% on short GCC inputs.

`2026-10-22-float-knuth-rolling-pointers-ab.csv` records 12 alternating pairs
for rolling numerator and denominator pointers in the Knuth multiply/subtract
loop. TinyCC improves on every short and long parser case by 1.6–4.1%; the
other three builds are mostly flat with small mixed changes, so the pointer
path is selected only for TinyCC with `ccinfo.is_tcc`. The implementation
passed the four-way 1,250-case float differential matrix, the four-way Knuth
quotient precheck, and the standalone TinyCC smoke build. Details and the
post-change shootout run are in `2026-10-22-float-knuth-rolling-pointers.txt`.
The final host-versus-Nelibc results for the TCC-specialized implementation are
preserved in `2026-10-22-float-knuth-rolling-pointers-final-short.csv` and
`2026-10-22-float-knuth-rolling-pointers-final-long.csv`.

`2026-10-22-float-tcc-double-remainder-compare-ab.csv` records 12 alternating
TinyCC pairs for a specialized `2 × remainder` comparison used by midpoint
rounding. It avoids generic shifted-limb helpers that otherwise perform fixed
shift division/modulo work. Twelve paired runs show 2.1–3.2% gains on all short
parser rows; long-input results are mixed and noisy. The helper is TCC-only;
four-way differential and quotient-precheck
matrices passed, as did the standalone TCC smoke build. See
`2026-10-22-float-tcc-double-remainder-compare.txt` and the final
`2026-10-22-float-tcc-double-remainder-final-short.csv` /
`2026-10-22-float-tcc-double-remainder-final-long.csv` shootouts.

`2026-10-22-float-tcc-roundcarry-ab.csv` records 12 alternating pairs for a
rounding-increment rewrite that detects carry by checking whether q wraps to
zero. It passed the TinyCC float differential corpus but produced mixed timing:
short `strtof` regressed 1.4%, while other parser/input combinations ranged
from flat to 1.4% faster. The existing rounding code is retained; see
`2026-10-22-float-tcc-roundcarry.txt`.

`2026-10-22-float-tcc-bitwise-fallback-profile.txt` records zero TinyCC
bitwise-fallback entries across the 1,250-case float differential corpus and
both short and long shootouts. The fallback's shifted-subtract helper has a
128-byte TCC stack frame and can be called up to 64 times per division, but the
measured workloads never enter it; it remains unchanged as a cold correctness
path.

`2026-10-22-float-power5-scaling-profile-tcc.txt` profiles the long-decimal
power-of-five loop: 800 calls per parser, average exponent 898.5, 69 `5^13`
chunk multiplies per call, and growth from one to about 65.5 BigUInt limbs.
Rolling pointers in TinyCC's `big_mul_small` reduce measured scaling cycles by
18–23%. Twelve alternating full-parser pairs show 0.3–1.7% short-input gains
and 2.2–3.0% long-input gains; the implementation is TCC-only. The paired data
is in `2026-10-22-float-power5-mulptr-ab.csv`. Four-way float and quotient
prechecks plus standalone TCC smoke passed. Final short and long matrices are
`2026-10-22-float-power5-mulptr-final-short.csv` and
`2026-10-22-float-power5-mulptr-final-long.csv`.

`2026-10-22-float-power5-codegen-followup.txt` records the remaining TinyCC
spill pattern in `big_mul_small` after pointer addressing: product and carry
still spill around each limb multiply. Inlining the `5^13` chunk loop removed
repeated helper calls but was mixed in 12 alternating pairs and is rejected;
see `2026-10-22-float-power5-inline-chunk-rejected-ab.csv`.
A product-word union was also tested to avoid the mask/shift loads around the
64-bit result. Aggregate initialization emitted a `memcpy` call; no-init direct
assignment removed that call but remained slower on long inputs. Both variants
are rejected; see `2026-10-22-float-power5-product-union-rejected-ab.csv`.

`2026-10-22-float-long-current-hotspots-tcc.txt` measures the current TinyCC
long-parser functions with cycle timers. `big_mul_power5` runs once per parse
and costs about 9,900–10,200 cycles per call; the Knuth quotient loop costs
about 550–1,850 cycles per parse. Together with the correction-loop counts in
`2026-10-22-float-knuth-correction-counts.txt`, the profile keeps attention on
repeated power-of-five scaling and its `big_mul_small` limb work.

A direct x86 `mull` output-variable probe is recorded in
`2026-10-22-float-power5-codegen-followup.txt`: TinyCC expanded the frame from
48 to 64 bytes and introduced more stack traffic, so the current high-level
multiply remains. The malformed output-to-memory constraint probe was discarded.

`2026-10-22-float-power5-mul-asm-macro-ab.csv` records 48 alternating TinyCC
pairs for a TCC/x86-64 direct-site `mull` macro. It removes the product spill
and improves short parsing by 0.7–2.0%, but long parsing regresses by 0.3–1.3%,
so production retains the current high-level multiply. The function-only power5
cycle profile is in `2026-10-22-float-power5-mul-asm-macro-cycle-ab.csv`; its
improvement did not carry through to full long parses. Details are in
`2026-10-22-float-power5-codegen-followup.txt`.

`2026-10-22-float-power5-mul-threshold.txt` records limb-length counts for
`big_mul_small` and two TCC/x86-64 macro gates. All short-case limb work was at
16 limbs or fewer, but the `<=16` gate had mixed parser timing and the `<=8`
gate regressed short parsing. Neither gate is retained; counts and paired data
are in `2026-10-22-float-power5-mul-limb-counts.csv` and the two `le16`/`le8`
A/B CSVs.

`2026-10-22-float-decimal-append-pointer.txt` profiles the exact decimal digit
append path at 15.6 million BigUInt limb iterations on the 600-digit corpus.
A TCC-only rolling-pointer loop reduces long parser time by 13.4–14.5% in 24
alternating pairs, with 0.7–2.0% short-input gains. Four-way float differential
and Knuth precheck matrices passed, as did standalone TinyCC smoke. Final
short/long matrices and raw profile counts are linked from that note.

`2026-10-22-float-hex-append-pointer.txt` profiles 18.7 million BigUInt limb
iterations per parser benchmark on 600-digit hexadecimal inputs. A TCC-only
rolling-pointer append loop improves long-hex parsing by 18.4–18.8% in 24
alternating pairs, with short timings within 0.3% of baseline. Float differential,
custom long-hex correctness, quotient-precheck, compiler fallback, and standalone
TinyCC checks passed. Raw profile counts, paired data, and final matrices are
linked from that note.

- `2026-10-22-float-hex-shift-profile.md` records BigUInt shift and comparison call counts for the 600-digit hexadecimal workload; raw counters are in the adjacent CSV and text files.
- `2026-10-22-float-hex-trailing-zero-shift.md` documents the exact hexadecimal conversion optimization and its TinyCC/glibc shootout; paired timings are in the adjacent `*-ab.csv` files.
- `2026-10-22-float-hex-shift-zig-ab.csv` and `2026-10-22-float-hex-shift-gcc-ab.csv` extend the combined-shift shootout to the primary Zig cc and GCC compiler paths.
- `2026-10-22-float-decimal-shift-profile.md` records BigUInt shift and comparison counts for the 600-digit decimal workload; raw counters are in the adjacent CSV and text files.
- `2026-10-22-float-decimal-compare-loop-visits.csv` gives exact `big_compare_shifted` loop visits for the long-decimal cases; the shift profile note incorporates the result.
- `2026-10-22-float-decimal-leftshift-profile.md` records actual `big_shift_left` loop visits and explains why the fused-loop candidate was rejected; its six compiler/input paired shootouts are linked from that note.
- `2026-10-22-float-decimal-power5.md` records the `5^26` wide-chunk optimization, limb-work profile, and 24-pair shootouts under TinyCC, Zig cc (glibc and musl), and GCC.
- `2026-10-22-float-decimal-division-profile.md` records Knuth quotient-digit work and fallback use for the two long decimal cases.
- `2026-10-22-float-decimal-qdigit-profile.csv` records how often the Knuth quotient estimate needs correction on long decimal inputs; it is summarized in the division profile note.
- `2026-10-22-float-decimal-divloop-shootout.md` compares universal pointer walking and narrowed carry/borrow state in the Knuth multiply/subtract loop across TinyCC, Zig cc, and GCC.
- `2026-10-22-float-decimal-divloop-assembly.md` records the GCC scalar versus Zig cc two-limb-unrolled assembly for the Knuth multiply/subtract loop.
- `2026-10-22-float-divunroll-gcc.md` records the GCC `-funroll-loops` and source-level two-limb unroll shootouts; neither candidate was retained.
- `2026-10-22-float-divcarry-combine-gcc.md` records the GCC carry/borrow combination candidate, its assembly, and mixed shootout results; it was not retained.
- `2026-10-22-float-decimal-divtime.md` compares timed Knuth division against full long-decimal parse time for TinyCC, Zig cc, and GCC on glibc, plus Zig cc on musl.
- `2026-10-22-float-decimal-bignumtime.md` directly compares timed `big_mul_power5` and Knuth division shares in one long-decimal run across TinyCC, Zig cc, and GCC on glibc, plus Zig cc on musl.

`2026-10-22-float-decimal-tail-batching.csv` records the first four-profile
shootout after batching exact decimal tail digits into groups of nine before
updating the BigUInt. Long-parser `strtod` medians fell from roughly 35.6 to
12.8 microseconds under TinyCC, 16.3 to 6.4 under Zig cc/glibc, and 13.0 to 4.9
under GCC/glibc; Zig cc/musl improved from 16.2 to 5.6 microseconds. The
1,250-case float differential matrix, including long decimal inputs, passed on
Zig/glibc, GCC/glibc, Zig/musl, and TinyCC/glibc. Timings are single-run
medians rather than paired A/B samples.

`2026-10-22-float-decimal-stages-tcc.txt` profiles the batched parser with
cycle timers around digit scanning/accumulation, exact decimal conversion,
power-of-five scaling, and grouped BigUInt appends. On long `strtod`, scanning
and accumulation measured about 24.0k cycles, exact decimal conversion 12.6k,
and power-of-five scaling 8.9k per parse. The group timer averaged 175 cycles
over 66 groups per parse; it includes timer-call overhead, which also inflates
the enclosing scan timer, so those values should not be directly subtracted.
A 12-pair TinyCC classifier probe was effectively flat and was rejected; raw
paired measurements are in `2026-10-22-float-decimal-classifier-probe.csv`.

The TinyCC branch of the grouped append now narrows the 64-bit product directly
to `cuint` when storing its low word. TinyCC had emitted three extra mask/shift
instructions for the explicit low-32-bit mask. In 12 alternating pairs, the
direct cast improved TinyCC `strtof`/`strtod` medians by 4.6%/2.9%; `strtold`
improved 1.0%. An unconditional version was mixed under GCC and regressed
0.4–1.7% under Zig cc, so the narrowing cast is limited to the TCC-specific
pointer loop. Four-profile float differential and long-decimal matrices passed.
Raw paired results are in `2026-10-22-float-decimal-narrow-store-ab.csv`; the
current end-to-end matrix is `2026-10-22-float-decimal-tail-batching.csv`.
A TCC/x86-64 union split of the product into low/high words was rejected after
36 pairs: `strtof` was flat, `strtod` improved 0.1%, and `strtold` improved
1.2%. The extra representation is not retained; raw timings are in
`2026-10-22-float-decimal-product-union-rejected.csv`.
A separate TCC multiply-add helper was also rejected after 36 pairs; its call
overhead outweighed the reduced work in the parser frame, regressing
`strtof`/`strtod`/`strtold` by 3.0%/2.3%/1.5%. See
`2026-10-22-float-decimal-helper-rejected.csv`.
A 32-bit carry variable passed the TinyCC differential corpus but regressed
all three long parsers by 2.2–3.5% in 36 pairs, so the 64-bit carry remains;
paired data is in `2026-10-22-float-decimal-carry32-rejected.csv`.

TinyCC's decimal parser now evaluates the 64-bit significand-fit division only
while it can still store a significand digit. Once the 19/20-digit significand
is full, the long tail cannot be stored there, so that per-byte division was
unneeded. In 12 alternating long-input pairs, the TCC-specific change improved
`strtof`/`strtod`/`strtold` by 8.9%/7.4%/7.2%; every `strtof` pair and 11 of 12
pairs for each other parser improved. The unconditional version regressed
under Zig cc/glibc, GCC/glibc, and Zig cc/musl, so other compilers retain the
original expression. Four-profile float differential and long-decimal matrices
passed. Paired data is in `2026-10-22-float-decimal-tcc-significand-check-ab.csv`
and `2026-10-22-float-decimal-div-skip-universal-rejected.csv`; the final
matrix is `2026-10-22-float-decimal-tail-batching.csv`.

The decimal scanner now derives fractional digit count once after the mantissa
scan instead of incrementing it for every fractional digit. In 12 alternating
long-input pairs this modestly improved TinyCC (`strtof`/`strtod`/`strtold`
1.9%/2.3%/2.4%) and GCC (3.2%/3.0%/1.9%). Zig cc/glibc and Zig cc/musl were
near-flat, with changes within about 1%. The four-profile float differential
matrix and long-decimal shootout passed; paired and final matrix data are in
`2026-10-22-float-decimal-fraction-count-ab.csv` and
`2026-10-22-float-decimal-fraction-count-shootout.csv`.

A cursor-derived integer-tail exponent count was tested against the retained
per-digit exponent update. It records the first dropped integer digit and adds
the tail length once after scanning. In 12 alternating pairs, paired medians
showed TinyCC `strtof` 0.8% faster but `strtod`/`strtold` 1.0%/2.3% slower;
GCC regressed 2.9–5.9% across all three parsers. Zig cc/glibc changes were mixed
(-0.5% to +2.0%), and Zig cc/musl stayed within 0.8%. The candidate was
rejected and production source restored. Paired data is in
`2026-10-22-float-decimal-integer-tail-count-rejected.csv`.

The `skipped_digits` fallback path was profiled under TinyCC/glibc with 14,800
nonzero integer digits followed by `e-9999`. Exact accumulation failed once per
parser at its 1,400-limb parse limit; the scanner then performed 1,307–1,308
per-digit skip-count updates. The ordinary 600-digit shootout input never
reaches this path. This counter-only temporary build is not used for timings.
`tools/float_diff.c` now includes a 14,800-digit all-nonzero decimal followed by
`e-9999`, exercising the nonzero skipped-tail case. `make float-diff-matrix`
passed 1,251 cases under Zig/glibc, GCC/glibc, Zig/musl, and TinyCC/glibc.
Counts are in `2026-10-22-float-decimal-skipped-profile.csv`.


Once `skipped_nonzero` becomes true during fallback scanning, later nonzero
digits were redundantly tested and assigned true on every byte. The parser now
checks the flag first and only tests/stores a later digit until the first
nonzero is found. A 24-pair alternating shootout used the 14,800-digit all-one
input and a 13,400-one/1,400-zero tail control under TinyCC, Zig cc/glibc, GCC,
and Zig cc/musl. Aggregate paired medians improved 1.16%/0.21% under TinyCC,
0.66%/0.97% under Zig cc/glibc, 0.22%/-0.10% under GCC, and 0.64%/0.19% under
Zig cc/musl (all-one/zero-tail). The candidate is retained; the four-profile
1,251-case float differential matrix passed. Paired results are in
`2026-10-22-float-decimal-skipped-nonzero-ab.csv`.

A temporary TinyCC counter profile measured decimal trailing-zero bookkeeping.
On the ordinary 600-digit fraction/integer inputs, the pending-chunk counter
updates 581–601 times and the exact counter updates 19–20 times per parse. On a
14,800-digit all-zero input, the exact counter updates 14,800 times even though
parsing returns zero; on 14,800 leading zeroes followed by one, it updates
14,801 times and the first significant digit resets the count. A guard intended
to skip these useless leading-zero updates regressed Zig cc/glibc and
Zig cc/musl by roughly 4–7% on long inputs, so neither tested guard form was
retained. Paired results for both variants are in
`2026-10-22-float-decimal-trailing-zero-guards-rejected.csv`; raw counter data
is in `2026-10-22-float-decimal-trailing-zero-profile.csv`.

The pending-chunk trailing-zero count updates once per digit. A universal
replacement that counts trailing zeroes from the nine-digit chunk at flush
regressed Zig cc/musl by 3–7% on the long standard inputs, so that form was
rejected for Zig, GCC, and musl. TinyCC improved across the 18 parser/workload
medians in 24 alternating pairs, ranging from 0.45% to 7.93% on standard
inputs and 0.51% to 2.12% on fallback inputs. TinyCC now derives the count only
at chunk flush; other compilers retain per-digit tracking. The 1,251-case float
differential matrix passed for all four profiles. Paired data, including the
rejected universal version, is in
`2026-10-22-float-decimal-pending-zero-ab.csv`; the final long-decimal matrix is
`2026-10-22-float-decimal-pending-zero-final-shootout.csv`.

The decimal chunk accumulator was profiled at `-O2` under TinyCC and Zig cc.
The 600-digit fraction/integer inputs execute 581–582/600–601
`pending_decimal_chunk = chunk * 10 + digit` updates per parse, grouped into 65/67
nine-digit chunks. Calibrated `rdtsc` intervals put the full nine-digit scan and
accumulation region at 98.5–128.3 cycles/group under TinyCC and 86.3–94.5 under
Zig cc; the BigUInt append plus carry interval was 129.1–154.2 and 41.6–44.5
cycles/group, respectively. These intervals include parser-loop or append work
and subtract measured timer overhead (28.51 cycles for TinyCC, 23.95 for Zig).
They are not timings of the multiply-add instruction alone.

A separate arithmetic microbenchmark measured the multiply-add kernel at
0.6204 ns/digit under TinyCC and 0.2009 under Zig cc; its generated code uses
`imul` plus repeated stack accesses under TinyCC, while Zig cc uses `lea`
sequences. Rewriting multiply-by-ten as shifts and adds regressed TinyCC by
14.72% in 24 alternating pairs and was flat under Zig cc, so that rewrite was
rejected. No production code changed. Stage timings are in
`2026-10-22-float-decimal-chunk-stage-profile.csv`; isolated kernel timings are
in `2026-10-22-float-decimal-chunk-microbench.csv`, with paired shift results in
`2026-10-22-float-decimal-chunk-microbench-shift-ab.csv`.

A TCC-only inline-assembly variant replaced `chunk * 10 + digit` with two
`lea` instructions and an add. In 24 alternating TinyCC pairs, it improved 13
of 18 parser/input medians by 0.9–4.3%, but regressed `strtof` integer input
by 2.8% and `strtold` short decimals by 2.6%; several other medians were near
noise. It was rejected because the gains were inconsistent across parser
workloads. The candidate still generated surrounding stack loads/stores, so it
did not remove the accumulator's stack traffic. Production source is unchanged.
Paired results are in `2026-10-22-float-decimal-chunk-lea-tcc-rejected.csv`.

A constant-threshold test replaced the per-digit 64-bit division in the
significand overflow check: `significand < UINT64_MAX/10`, or equality with a
digit no greater than `UINT64_MAX%10`. In 24 alternating TinyCC pairs, the
candidate regressed 17 of 18 parser/input medians, generally by 1–6%, and was
rejected. Both baseline and candidate were rebuilt from Nelua source with the
same TinyCC flags. Production source is unchanged. Paired data is in
`2026-10-22-float-decimal-significand-threshold-tcc-rejected.csv`.
