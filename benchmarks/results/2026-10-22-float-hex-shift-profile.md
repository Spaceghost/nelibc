# BigUInt shift profile: long hexadecimal conversion

Workload: existing two 600-digit hexadecimal strings, profiled through the TinyCC/glibc Nelibc parser. Counters include all iterations in the benchmark's five samples, not just one parse.

| Parser | `big_shift_left` calls | operand limbs | `big_shift_right_one` calls | operand limbs | `big_compare_shifted` calls | operand limbs |
|---|---:|---:|---:|---:|---:|---:|
| `strtof` | 1,200 | 62,000 | 2,400 | 182,800 | 1,200 | 123,200 |
| `strtod` | 1,200 | 61,600 | 2,400 | 182,800 | 1,200 | 122,800 |
| `strtold` | 2,400 | 120,800 | 2,400 | 182,800 | 1,600 | 181,600 |

The limb column is the sum of operand `used` lengths at each call, not exact loop iterations. In particular, shifted comparisons can exit early. `big_try_subtract_shifted` had zero calls: the TCC Knuth division path handled these inputs without its fallback. `big_shift_right_one` was active in `exact_binary_to_double`: it repeatedly removed low zero bits before exact hexadecimal rational conversion. The production code now collapses this into one combined limb and bit shift. The instrumented profile build used a temporary Nelua source copy; only the measured combined-shift change was applied to production.
