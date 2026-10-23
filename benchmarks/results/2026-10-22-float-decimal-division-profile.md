# BigUInt division profile: long decimal conversion

Temporary instrumentation on the two 600-digit decimal cases counted Knuth division outcomes and inner quotient-digit work. Each parser type ran 800 calls across the benchmark's five samples.

| Parser | Knuth calls | Fallbacks | Quotient digits | Limb visits in multiply/subtract | Estimate corrections | Addbacks |
|---|---:|---:|---:|---:|---:|---:|
| `strtof` | 400 | 0 | 800 | 51,200 | 0 | 0 |
| `strtod` | 400 | 0 | 1,200 | 75,600 | 400 | 0 |
| `strtold` | 800 | 0 | 2,400 | 180,000 | 400 | 0 |

No call used the bit-by-bit shifted-subtraction fallback. The quotient-digit loop processed 64, 63, and 75 limbs per digit on average for `strtof`, `strtod`, and `strtold`, respectively. Estimator correction occurred once per affected division; addback never occurred. This confirms the Knuth multiply/subtract loop dominates the measured division work. No production change was made from this profile.

Raw counters are in `2026-10-22-float-decimal-division-counters.txt`.

## Quotient-estimate correction profile

A follow-up counted correction-loop iterations per quotient digit. Across 800 parses per type, `strtof` had 800 quotient digits with no corrections; `strtod` had 400 of 1,200 digits corrected once; `strtold` had 400 of 2,400 corrected once. No digit needed two corrections, and the `quotient_digit >= base` clamp did not activate. This means the correction loop itself is not a large work source. The 64-bit quotient estimate runs once for each digit (800/1,200/2,400 times), while the long multiply/subtract loop visits 51,200/75,600/180,000 limbs. No production change was made.

Correction histogram: `2026-10-22-float-decimal-qdigit-profile.csv`; raw counters: `2026-10-22-float-decimal-qdigit-counters.txt`.
