# BigUInt operation shares in long-decimal conversion

Timed `big_mul_power5` and `big_divide_knuth_u64` inside the same benchmark run using the two 600-digit decimal inputs. Each Nelibc parser ran 800 calls. Percentages compare mean time per parse for each operation with the instrumented full-parser median; the latter includes small clock-hook overhead.

**Correction for Zig cc/musl:** the first profile combined host and Nelibc counters. In the static musl executable, `strtof_l`/`strtod_l`/`strtold_l` route through the executable's interposed public symbols, so the nominal host-libc pass also called Nelibc. A layer-counter run confirmed 800 public-parser calls and 800 exact-decimal conversions during the host pass, followed by the same counts in the Nelibc pass. The musl rows below now use counters reset between those passes; parser timing is from the Nelibc pass. The musl host-libc timing is not a valid host baseline and is omitted.

| Toolchain/libc | Parser | Power5 ns/parse | Power5 share | Knuth ns/parse | Knuth share | Parse ns/call |
|---|---|---:|---:|---:|---:|---:|
| TinyCC/glibc | `strtof` | 3,933.10 | 10.21% | 582.29 | 1.51% | 38,512.72 |
| TinyCC/glibc | `strtod` | 3,771.30 | 10.19% | 697.38 | 1.88% | 36,996.26 |
| TinyCC/glibc | `strtold` | 4,125.76 | 10.80% | 1,210.52 | 3.17% | 38,213.87 |
| Zig cc/glibc | `strtof` | 1,429.74 | 8.33% | 118.75 | 0.69% | 17,157.36 |
| Zig cc/glibc | `strtod` | 1,431.70 | 8.36% | 152.11 | 0.89% | 17,126.66 |
| Zig cc/glibc | `strtold` | 1,431.93 | 8.27% | 286.91 | 1.66% | 17,319.50 |
| GCC/glibc | `strtof` | 1,224.88 | 8.67% | 148.27 | 1.05% | 14,132.66 |
| GCC/glibc | `strtod` | 1,253.49 | 8.95% | 186.87 | 1.33% | 14,003.77 |
| GCC/glibc | `strtold` | 1,234.77 | 8.75% | 279.58 | 1.98% | 14,114.48 |
| Zig cc/musl | `strtof` | 1,399.0 | 8.25% | 222.20 | 1.31% | 16,952.81 |
| Zig cc/musl | `strtod` | 1,347.9 | 8.21% | 267.29 | 1.63% | 16,410.16 |
| Zig cc/musl | `strtold` | 1,384.7 | 8.42% | 257.64 | 1.57% | 16,454.99 |

`big_mul_power5` accounts for about 7–11% of parse time in this run; Knuth division accounts for about 0.7–3.2%. Power5 time is several times larger on every valid path, so it remains the better BigUInt optimization target. The earlier 5^26 chunk change is retained. No production code changes were made from this timing pass.

Underlying corrected timing totals are in `2026-10-22-float-decimal-bignumtime.csv`; raw counters are in the adjacent text files. The musl host baseline still needs a separate-process benchmark before it can be compared.
