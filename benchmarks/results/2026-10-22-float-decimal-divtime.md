# Knuth division share of long-decimal parse time

Timed `big_divide_knuth_u64` directly inside the existing two-600-digit-decimal benchmark. The timed interval excludes the clock helper calls; total parse medians include their small callback overhead. Each parser function made 800 measured parses. Percentages compare mean Knuth time per parse with the instrumented benchmark median per parse.

| Toolchain/libc | Parser | Knuth calls | Knuth ns/parse | Full parse ns/call | Division share |
|---|---|---:|---:|---:|---:|
| TinyCC/glibc | `strtof` | 400 | 603.38 | 37,713.12 | 1.60% |
| TinyCC/glibc | `strtod` | 400 | 741.73 | 34,931.91 | 2.12% |
| TinyCC/glibc | `strtold` | 800 | 1,094.98 | 38,419.04 | 2.85% |
| Zig cc/glibc | `strtof` | 400 | 104.50 | 16,557.54 | 0.63% |
| Zig cc/glibc | `strtod` | 400 | 132.44 | 16,162.70 | 0.82% |
| Zig cc/glibc | `strtold` | 800 | 264.72 | 16,242.06 | 1.63% |
| GCC/glibc | `strtof` | 400 | 151.86 | 13,444.77 | 1.13% |
| GCC/glibc | `strtod` | 400 | 158.50 | 13,486.02 | 1.18% |
| GCC/glibc | `strtold` | 800 | 274.85 | 13,612.51 | 2.02% |
| Zig cc/musl | `strtof` | 800 | 210.71 | 16,312.16 | 1.29% |
| Zig cc/musl | `strtod` | 800 | 310.75 | 16,222.03 | 1.92% |
| Zig cc/musl | `strtold` | 1,600 | 546.28 | 16,052.53 | 3.40% |

Division accounted for about 0.6–3.4% of parse time in these cases. The TinyCC, GCC, and Zig/glibc results show about 0.6–2.9%; Zig/musl ranges from 1.3% to 3.4% and makes more Knuth calls than the corresponding glibc parser. The quotient-digit multiply/subtract kernel is therefore not the primary long-decimal cost; the profiled `big_mul_power5` work is substantially larger. No production change was made.

The CSV contains underlying timing totals. Per-path raw counters are in the adjacent `2026-10-22-float-decimal-divtime-*.txt` files.
