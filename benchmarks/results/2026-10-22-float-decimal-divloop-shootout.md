# Knuth multiply/subtract loop shootout

I compared the current TCC-specific pointer walk plus indexed non-TCC loop against two temporary alternatives: pointer walking on every compiler, and `cuint` carry/borrow state in the indexed/pointer loops. Each result is the median of 12 alternating baseline/candidate pairs using the two long decimal inputs. Short-corpus results are retained in the adjacent CSVs.

## Universal pointer walk

| Toolchain | Parser | Baseline ns/call | Pointer ns/call | Change |
|---|---|---:|---:|---:|
| TinyCC/glibc | `strtof` | 37,346.09 | 37,367.04 | +0.1% |
| TinyCC/glibc | `strtod` | 37,201.12 | 37,022.02 | -0.5% |
| TinyCC/glibc | `strtold` | 37,660.29 | 37,749.17 | +0.2% |
| Zig cc/glibc | `strtof` | 15,923.03 | 15,983.96 | +0.4% |
| Zig cc/glibc | `strtod` | 15,695.66 | 15,795.28 | +0.6% |
| Zig cc/glibc | `strtold` | 15,828.54 | 15,968.62 | +0.9% |
| GCC/glibc | `strtof` | 13,549.87 | 13,757.86 | +1.5% |
| GCC/glibc | `strtod` | 13,639.32 | 13,542.81 | -0.7% |
| GCC/glibc | `strtold` | 13,788.99 | 13,663.68 | -0.9% |

## Narrow carry/borrow types

| Toolchain | Parser | Baseline ns/call | Narrow state ns/call | Change |
|---|---|---:|---:|---:|
| TinyCC/glibc | `strtof` | 37,049.01 | 36,947.19 | -0.3% |
| TinyCC/glibc | `strtod` | 37,633.02 | 36,014.89 | -4.3% |
| TinyCC/glibc | `strtold` | 37,867.33 | 37,733.74 | -0.4% |
| Zig cc/glibc | `strtof` | 15,885.62 | 16,042.86 | +1.0% |
| Zig cc/glibc | `strtod` | 15,772.03 | 15,855.51 | +0.5% |
| Zig cc/glibc | `strtold` | 16,032.30 | 15,760.62 | -1.7% |
| GCC/glibc | `strtof` | 13,522.62 | 13,679.89 | +1.2% |
| GCC/glibc | `strtod` | 13,446.01 | 13,412.62 | -0.2% |
| GCC/glibc | `strtold` | 13,637.38 | 13,694.24 | +0.4% |

The pointer variant regressed or was noisy on Zig cc/GCC and slightly worsened most short cases; TinyCC already uses that pointer path. Narrow carry/borrow types helped some parser/compiler combinations and hurt others, including a short GCC regression. Neither alternative had a consistent cross-compiler benefit, so production code remains unchanged. The existing TCC-only pointer specialization remains in place.

All 12-pair long/short CSVs are adjacent: `2026-10-22-float-decimal-divloop-pointer-*-ab.csv` and `2026-10-22-float-decimal-divcarry-*-ab.csv`.
