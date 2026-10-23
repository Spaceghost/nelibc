# Hexadecimal exact conversion: combined trailing-zero shift

`exact_binary_to_double` previously removed trailing binary zeroes one bit at a time. Each one-bit shift walked the whole BigUInt. The shift profile showed 2,400 calls per parser type over the benchmark's five samples, with a summed operand length of 182,800 limbs.

The implementation in `src/stdlib_float.nelua` now counts zero whole limbs and low zero bits, advances `exponent2` by the matching bit count, and shifts the BigUInt right once. This preserves the exact integer/exponent pair while replacing repeated full-array passes with one pass.

## Long-hex shootout

Twelve alternating baseline/candidate pairs per compiler, TinyCC/glibc, Zig cc/glibc, and GCC/glibc. The values below are medians of the 12 per-binary medians for the existing two 600-digit hex inputs.

| Compiler | Parser | Baseline ns/call | Candidate ns/call | Change |
|---|---|---:|---:|---:|
| TinyCC | `strtof` | 35,984.87 | 36,557.88 | +1.6% |
| TinyCC | `strtod` | 36,377.36 | 35,770.76 | -1.7% |
| TinyCC | `strtold` | 37,074.96 | 37,157.02 | +0.2% |
| Zig cc | `strtof` | 16,854.96 | 16,129.95 | -4.3% |
| Zig cc | `strtod` | 16,556.84 | 16,096.74 | -2.8% |
| Zig cc | `strtold` | 16,292.73 | 16,189.08 | -0.6% |
| GCC | `strtof` | 14,087.36 | 13,814.96 | -1.9% |
| GCC | `strtod` | 13,981.32 | 13,806.18 | -1.3% |
| GCC | `strtold` | 14,115.74 | 14,055.65 | -0.4% |

Short-input TinyCC medians were effectively flat: `strtof` +0.4%, `strtod` +2.4%, and `strtold` -0.1%. The long-input gains under the primary Zig cc and GCC paths support retaining the single-pass implementation; TinyCC results are mixed and small.

The four-way float differential matrix passed (1,250 cases on each configured compiler/libc path), and standalone TCC smoke passed. Paired timings: `2026-10-22-float-hex-shift-long-ab.csv`, `2026-10-22-float-hex-shift-short-ab.csv`, `2026-10-22-float-hex-shift-zig-ab.csv`, and `2026-10-22-float-hex-shift-gcc-ab.csv`.
