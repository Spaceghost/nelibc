# Long-decimal `big_mul_power5` profile and wide-chunk shootout

The existing 600-digit decimal workload exercises `big_mul_power5` heavily. Across 800 parses per parser, the baseline made 55,600 `5^13`/tail multiplications and visited 2,031,600 input limbs per conversion type. The candidate uses a fused two-word multiply for `5^26` when the requested power is at least 256; shorter powers retain the existing `5^13` path. The candidate made 27,600 `5^26` calls plus 400 tail calls and visited 1,020,400 input limbs, a 49.8% reduction in BigUInt loop visits. Each wide-chunk visit computes two 32-bit partial products, so the main gain is fewer passes over the limb array.

## Shootout

Twenty-four alternating baseline/candidate pairs per toolchain. Long cases use the two 600-digit decimal strings; short cases use the regular float corpus. Values are medians of the 24 per-binary medians.

| Toolchain/libc | Parser | Baseline ns/call | Candidate ns/call | Change |
|---|---|---:|---:|---:|
| TinyCC/glibc | `strtof` | 37,548.47 | 36,862.83 | -1.8% |
| TinyCC/glibc | `strtod` | 37,581.91 | 37,017.32 | -1.5% |
| TinyCC/glibc | `strtold` | 38,338.67 | 37,933.82 | -1.1% |
| Zig cc/glibc | `strtof` | 16,131.38 | 15,862.22 | -1.7% |
| Zig cc/glibc | `strtod` | 15,840.01 | 15,836.21 | -0.0% |
| Zig cc/glibc | `strtold` | 16,069.27 | 16,034.92 | -0.2% |
| GCC/glibc | `strtof` | 13,887.99 | 13,521.55 | -2.6% |
| GCC/glibc | `strtod` | 13,562.53 | 13,380.58 | -1.3% |
| GCC/glibc | `strtold` | 13,816.18 | 13,513.64 | -2.2% |
| Zig cc/musl | `strtof` | 16,276.69 | 15,984.99 | -1.8% |
| Zig cc/musl | `strtod` | 16,192.60 | 15,782.46 | -2.5% |
| Zig cc/musl | `strtold` | 16,436.81 | 15,983.06 | -2.8% |

Short-corpus medians stayed within about 1% of baseline. The four-way float differential matrix passed 1,250 cases per compiler/libc path, including the long decimal cases; standalone TCC smoke also passed. This candidate is retained.

Profile counts: `2026-10-22-float-decimal-power5-profile.csv`; raw baseline/candidate counters are in the adjacent text files. Paired data is in the adjacent `2026-10-22-float-decimal-power5-*-ab.csv` files.
