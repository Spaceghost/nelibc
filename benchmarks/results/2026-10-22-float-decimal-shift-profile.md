# BigUInt shift profile: long decimal conversion

Workload: existing two 600-digit decimal strings, profiled through the TinyCC/glibc Nelibc parser. Counters cover the benchmark's five samples. `operand_limb_sum` adds the `used` lengths of the input BigUInts at each call; it is not an exact loop-iteration count, especially for comparisons that can exit early.

| Parser | `big_shift_left` calls | operand limbs | absolute shift amount sum | `big_compare_shifted` calls | operand limbs | absolute shift amount sum |
|---|---:|---:|---:|---:|---:|---:|
| `strtof` | 1,200 | 69,200 | 270,800 bits | 1,200 | 155,200 | 608,800 bits |
| `strtod` | 1,200 | 68,800 | 256,800 bits | 1,200 | 154,800 | 608,800 bits |
| `strtold` | 1,600 | 94,000 | 604,000 bits | 1,600 | 225,200 | 634,400 bits |

There were no shifted-subtraction fallback calls and no trailing-zero right shifts. Each parser call performs roughly one or two left shifts and shifted comparisons, rather than the repeated full-BigUInt right shifts observed in exact hexadecimal conversion. This profile does not show a comparable repeated-shift bottleneck, so production code was unchanged. Instrumentation was compiled from a temporary source copy.

## Actual shifted-comparison loop visits

A second temporary build counted each iteration inside `big_compare_shifted`. It made only 800, 1,200, and 1,600 visits across 800 parses for `strtof`, `strtod`, and `strtold`, respectively. Compared with 1,200/1,200/1,600 helper calls from the operand profile, that is 0.67/1.00/1.00 loop iterations per comparison on average. Most calls return from the used-length check before entering the limb loop, and the remaining calls stop after about one limb. This confirms shifted comparison is not a meaningful long-decimal bottleneck. Raw totals and derived averages are in `2026-10-22-float-decimal-compare-loop-visits.txt` and `.csv`.
