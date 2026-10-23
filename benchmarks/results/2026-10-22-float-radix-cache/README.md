# Cache the locale radix once per parse — 2026-10-22

`src/stdlib_float.nelua` now calls `nelibc_radix_character()` once before
scanning digits. The hybrid callback uses `localeconv()`, so calling it in the
digit loop repeated a locale lookup for every input byte. `tools/float_bench.c`
now models that callback with the initial C locale instead of a constant stub.

The eight-run paired comparisons used the same compiler and benchmark binary
setup for the pre-change loop and cached candidate. Median Nelibc nanoseconds
per call:

| Profile | Function | Before | Cached | Change |
| --- | --- | ---: | ---: | ---: |
| Zig/glibc | `strtof` | 505.25 | 308.48 | -39% |
| Zig/glibc | `strtod` | 411.82 | 206.14 | -50% |
| Zig/glibc | `strtold` | 428.01 | 225.63 | -47% |
| TinyCC/glibc | `strtof` | 868.61 | 621.12 | -28% |
| TinyCC/glibc | `strtod` | 834.54 | 613.21 | -27% |
| TinyCC/glibc | `strtold` | 958.54 | 737.23 | -23% |

`four-profile-matrix.csv` records the cached implementation under Zig/glibc,
GCC/glibc, Zig/musl, and TinyCC/glibc. The 1,250-case float differential
matrix passed on all four profiles after the parser change.

`zig-glibc-gprof-profile.txt` is a diagnostic `-pg -fno-inline` run with
100,000 benchmark iterations. It attributes about 20% of sampled parser time
to `big_divide_knuth_u64` and most of the remaining parser time to
`strtod_impl`; the short gprof sampling window makes these shares approximate.
