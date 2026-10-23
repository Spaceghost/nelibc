# Knuth multiply/subtract loop: GCC and Zig cc assembly

Generated C with Nelua `--generator c`, then compiled that C with GCC and Zig cc at `-O2 -fno-builtin`. To avoid C prototype conflicts with this libc's exported `strtof`/`strtod` signatures during standalone assembly emission, the temporary C copy replaced its `<stdlib.h>` include with `void abort(void);`; the division function body is unchanged.

## GCC

GCC keeps one quotient-limb iteration per loop. The hot sequence loads one numerator and denominator limb, multiplies, adds carry, subtracts product and borrow, stores the limb, extracts borrow, increments the index, and branches. The generated loop has no bounds check in its inner body.

```asm
movl    (%rdi,%rdx,4), %eax
movl    (%r9,%rdx,4), %ecx
imulq   %rbx, %rax
addq    %r11, %rax
movl    %eax, %eax
subq    %rax, %rcx
shrq    $32, %r11
subq    %r10, %rax
movl    %eax, (%r9,%rdx,4)
shrq    $63, %rax
addq    $1, %rdx
cmpq    %rdx, %rsi
jne     .L72
```

## Zig cc

Clang/LLVM unrolls the indexed loop by two limbs: it issues two multiply/carry/subtract sequences before incrementing the limb index by two and checking the loop bound. Its quotient estimate uses `divl` when the high 32 bits of the dividend are zero, otherwise `divq`. The inner limb loop also has no bounds check.

The earlier universal pointer-walk shootout regressed Zig cc by 0.4–0.9% on long decimals and regressed most short cases. The indexed form is what enables LLVM's two-limb unroll. GCC's scalar loop is the focused opportunity: test a GCC-only unroll or source-level two-limb unroll while keeping the indexed Zig path.

No production code changed from assembly inspection.
