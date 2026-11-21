# Matched libc comparison

This is a reproducible experiment, not a standards certification or a claim
that one libc is fastest on all workloads.

Build the three production archives first (`lib-gcc`, `lib-zig-musl`,
`lib-cosmo`). Follow the repository's AGENTS.md: build and run inside the
`nelibc-writ` container on Bak as `build`, with scratch/cache on approved
storage. Do not run a conformance build while collecting performance data.

```sh
python3 benchmarks/comparison/conformance.py \
  --source /path/to/libc-test \
  --out /path/to/new-conformance-output \
  --cosmocc /path/to/cosmocc/bin/x86_64-unknown-cosmo-cc \
  --ape-loader "$TMPDIR/.ape-1.10"
python3 benchmarks/comparison/run.py \
  --out /path/to/new-performance-output \
  --cosmocc /path/to/cosmocc/bin/x86_64-unknown-cosmo-cc \
  --pairs 7 --cpu 10
```

Output directories retain the binaries, build commands, checks, exported
symbol census, raw pairs, per-case summaries and source/archive hashes.
Conformance output retains the pinned source, build logs, per-test diagnostics
and deltas, without suppressing expected failures. Use a new directory for
each conformance run. The source checkout must contain libc-test commit
`63e36e934e8f2bedf645303a90b64098e2af6c8f` to reproduce the published corpus.

For a concurrent optimization comparison, copy the three production archives
to a separate directory before editing source. Pass `--previous-dir DIR` to
`run.py`; names must remain `libnelibc-gcc.a`, `libnelibc-zig-musl.a` and
`libnelibc-cosmo.a`. Seven rotating process triplets then measure the native,
previous and new implementations with identical consumers. Summary rows add
`previous_ns`, `speedup_vs_previous` and observed before/after ratio ranges.
The original two-build mode retains its alternating order.

`tools/strcmp_bench.py --old-archive PATH --pairs 7 --cpu 10 --out DIR`
adds alignment and mismatch-position measurements for GCC/glibc. Add
`--source tools/memory_bench.c` for bounded copy/fill/compare size and alignment
distributions. Both retain raw samples, commands and archive/source hashes.

For the Cosmopolitan short-search comparison, use the same compiler driver
that built both archives. The scan fixture starts on a 64-byte boundary and
tests offsets 0, 1, 7, 15 and 31. It includes every size from 8 through 16,
longer scans through 64 KiB, and absent, first, middle and last-byte matches.

```sh
python3 tools/strcmp_bench.py --profile cosmopolitan \
  --cc /path/to/cosmocc/bin/x86_64-unknown-cosmo-cc \
  --source tools/scan_bench.c --old-archive /path/to/previous/libnelibc-cosmo.a \
  --pairs 7 --cpu 10 --out /path/to/new-scan-output
```

## What the comparison isolates

Each pair uses the same compiler, consumer source, compiler options and inputs.
The glibc pair uses GCC; musl uses Zig cc's x86_64-linux-musl target;
Cosmopolitan uses the x86_64 compiler wrapper. Baselines omit the Nelibc
archive and its required wrapping options. Nelibc builds use the production
archive and, for musl, the production MUSL_STDIO_WRAP_FLAGS from the Makefile.
The archives have different replacement scopes. Symbols absent from a Nelibc
archive are labelled delegated; the record parser mixes several services.

The timed memory/string/math loops call volatile pointers to noinline
**direct-call wrappers** in every profile. This prevents compiler pure/const
attributes from hoisting calls outside the loop without taking addresses of
libc IFUNC symbols. A Cosmopolitan 4.0.2 `strstr` function-pointer probe failed
in the tested toolchain while direct calls passed. The published path uses
direct calls, and makes no claim about that function-pointer ABI path.
Loop, wrapper, pointer-call and checksum overhead is included, not subtracted.
`--check` exercises the same wrappers before any performance run.

Each case calibrates to at least 5 ms, then records a timing. Seven external
process pairs alternate which implementation runs first; profile order also
rotates. Speedup is the median of the seven baseline/Nelibc paired ratios.
Minimum and maximum are observed ranges, not confidence intervals. CPU
pinning does not reserve the CPU; load averages and the shared-host setting
are recorded. These are aligned, hot-buffer cases and a synthetic parser,
not DRAM bandwidth, cold-cache, multicore or application throughput evidence.

The independent conformance comparison uses each libc's **own headers for
both halves** and runs functional, regression and math tests. Compile-only
API tests and standalone/host profiles are outside this matched experiment.
It keeps libc-test's native supervisor, 100 KiB stack limit and five-second
timeout. Cosmopolitan .exe tests run through the native APE loader extracted
by its compiler into TMPDIR, supplied with --ape-loader; both halves use the
same loader. Build failures, link failures, runtime failures and timeouts remain
non-passes. Export counts use the fixed 1,471-name musl inventory and do not
certify semantics. Existing Nelibc-header/standalone gates remain distinct.

The GCC and Cosmopolitan links retain maps; Zig 0.17.0's linker rejects GNU
`-Map`, so that pair retains symbol tables instead. All pairs retain archive
exports and exact linker commands. Built file sizes are unnormalized: the
dynamic glibc executable excludes system libraries, while the other two
baselines link statically. Do not rank deployment footprints from those sizes.

## Refresh the site

Copy manifest.json, checks.json, coverage.json, summary.json, summary.csv and
raw.csv into a new `nelua-site/public/nelua/assets/comparison-YYYYMMDD/`.
Copy the conformance results.json as conformance.json, plus deltas.json.
Include the exact source snapshot as source.tar.gz. Then, in nelua-site:

```sh
python3 scripts/nelibc-comparison.py public/nelua/assets/comparison-YYYYMMDD
npm run build
npm run check
npm run deploy
```

The renderer updates both pages from those files and keeps historical
coverage and standalone gates labelled separately. Inspect regressions and
slower cases before publishing. Promote an optimization only after its
correctness checks and the matched comparison pass again.
