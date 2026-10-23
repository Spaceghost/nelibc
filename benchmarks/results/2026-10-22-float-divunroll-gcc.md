# GCC-only Knuth loop unroll shootout

Compared the GCC `-O2` float module against two GCC-only candidates, each in 24 alternating baseline/candidate pairs on the long-decimal and regular short corpora.

1. Module-wide `-funroll-loops`: long `strtof` regressed 1.3%, `strtod` improved 1.7%, and `strtold` improved 0.7%. Short `strtof` and `strtod` regressed 1.8% and 0.9%; `strtold` improved 0.4%.
2. Source-level two-limb unroll only when the divisor has at least 16 limbs: long `strtof` regressed 0.7%, `strtod` regressed 0.4%, and `strtold` improved 0.6%. Short results were essentially flat.

Neither candidate improved the long workload consistently, so the production loop remains scalar for GCC. The source-level candidate passed the 1,250-case GCC float differential corpus. Paired timings are in `2026-10-22-float-div-unroll-flag-gcc-{long,short}-ab.csv` and `2026-10-22-float-divunroll-source-gcc-{long,short}-ab.csv`.
