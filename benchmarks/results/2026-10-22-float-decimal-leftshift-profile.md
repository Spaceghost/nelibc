# BigUInt left-shift loop profile: long decimal conversion

The existing `big_shift_left` does up to three limb loops: move existing limbs to make room for whole-word shifts, fill the low zero limbs, and apply the residual bit shift. A temporary TinyCC/glibc instrumented build counted these loop bodies on the two 600-digit decimal cases. Each parser type ran 800 calls across five benchmark samples.

| Parser | Calls | Limb move | Zero fill | Bit shift | Total visits | Fused-pass estimate |
|---|---:|---:|---:|---:|---:|---:|
| `strtof` | 800 | 69,200 | 8,000 | 69,200 | 146,400 | 77,200 |
| `strtod` | 800 | 68,800 | 7,600 | 68,800 | 145,200 | 76,400 |
| `strtold` | 800 | 94,000 | 17,600 | 94,000 | 205,600 | 111,600 |

The profile showed an opportunity to combine the move and residual-bit loops. I measured that candidate with an eight-limb fallback for small values, alternating baseline and candidate 24 times under TinyCC/glibc, Zig cc/glibc, and GCC/glibc. Zig cc, the primary compiler path, regressed 2.2–3.1% on long decimals. TinyCC and GCC results were mixed to modestly positive, and short inputs stayed close to baseline. I reverted the candidate because the primary Zig path regressed.

No production code changed. The loop counts are in this CSV. Paired shootout data is in the adjacent `2026-10-22-float-decimal-leftshift-{tcc,zig,gcc}-{long,short}-ab.csv` files.
