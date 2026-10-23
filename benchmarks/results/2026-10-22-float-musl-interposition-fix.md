# Isolated musl float shootout

The original combined benchmark executable linked Nelibc's public `strtof`, `strtod`, and `strtold` symbols into a static musl program. Musl's `*_l` host functions resolve through those public symbols, so the purported host baseline called Nelibc too. The profiler's apparent 2x BigUInt counts were this host pass plus the Nelibc pass, not a second conversion in Nelibc.

Layer counters on one 600-digit input confirmed the cause: each host-libc phase recorded 800 Nelibc public-parser entries and 800 exact-decimal conversions; the following Nelibc phase recorded another 800 of each. The isolated BigUInt profile now records 800 `power5` calls for each 800-parse Nelibc phase, matching glibc, GCC, and TinyCC.

`tools/float_bench.c` now supports `BENCH_HOST_ONLY` and `BENCH_NELIBC_ONLY`. The musl Makefile targets build separate executables and merge their CSV output. This keeps host libc out of the Nelibc executable and Nelibc out of the host executable.

Corrected outputs:

- `2026-10-22-float-shootout-zig-musl-isolated.csv`
- `2026-10-22-float-long-shootout-zig-musl-isolated.csv`
- `2026-10-22-float-shootout-matrix-corrected.csv`
- `2026-10-22-float-long-shootout-matrix-corrected.csv`
- Corrected BigUInt counters: `2026-10-22-float-decimal-bignumtime-zig-musl.txt`

The Nelibc timings from prior combined runs remain usable because the host pass was timed separately; prior musl host-libc timings are invalid. Earlier experimental CSVs retain their Nelibc candidate/baseline comparisons, but their musl host-libc rows should not be treated as independent host results.

The corrected regular and long-decimal matrices were rerun across Zig/glibc, GCC/glibc, Zig/musl, and TinyCC/glibc with `make NELUA=... TCC=... float-shootout-matrix float-long-shootout-matrix`. Both matrix targets completed.

The same symbol interposition also made `float-diff-zig-musl` compare Nelibc with itself: musl's locale wrappers call the public conversion functions. The musl differential target now renames Nelibc's three exported symbols in a copy of its object before linking, so the host reference remains musl. That exposed errno differences on range results (zero, subnormal, infinity). Musl's `math_errhandling` reports `MATH_ERREXCEPT` without `MATH_ERRNO`; C makes `ERANGE` optional when underflow occurs and requires errno reporting only when the implementation advertises it. The test therefore permits `0` versus `ERANGE` only on range results when `MATH_ERRNO` is clear, while requiring matching values and end pointers. It counted 12 such variances in this 1,250-case run. Nelibc keeps its consistent `ERANGE` behavior; no target-specific parser code was needed. The full float differential matrix passes 1,250 cases on Zig/glibc, GCC/glibc, Zig/musl, and TinyCC/glibc.
