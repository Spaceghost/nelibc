# GCC quotient-product and borrow combination

The temporary candidate folds each limb subtraction's borrow into the next `product_carry`. It computes `difference = minuend - product_limb`, records `minuend < product_limb`, and adds that bit to the product high word. This replaces two 64-bit subtracts plus a top-bit borrow extraction with one 32-bit subtract, a borrow condition, and a carry addition.

GCC assembly confirms the new loop emits a 32-bit `subl`, `setb`, one 64-bit shift of the product high word, and one carry add. The current loop emits two 64-bit subtracts and extracts borrow with `shrq $63`.

Twenty-four alternating GCC/glibc pairs showed mixed long-input results: `strtof` improved 0.4%, `strtod` regressed 1.2%, and `strtold` regressed 0.8%. Short inputs improved 1.1–2.6%. The candidate passed the 1,250-case GCC float differential corpus, but long-input results do not justify retaining a GCC-only special case. No production code changed.

Paired data: `2026-10-22-float-divcarry-combine-gcc-long-ab.csv` and `2026-10-22-float-divcarry-combine-gcc-short-ab.csv`.
