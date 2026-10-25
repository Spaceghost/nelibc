.text
.globl nelibc_fenv_fxsave
.type nelibc_fenv_fxsave,@function
nelibc_fenv_fxsave:
  fxsave64 (%rdi)
  ret
.size nelibc_fenv_fxsave, .-nelibc_fenv_fxsave

.globl nelibc_fenv_fxrstor
.type nelibc_fenv_fxrstor,@function
nelibc_fenv_fxrstor:
  fxrstor64 (%rdi)
  ret
.size nelibc_fenv_fxrstor, .-nelibc_fenv_fxrstor

/* Deliver requested exceptions if their SSE masks are clear. Pending x87
   flags are delivered by FWAIT; explicit SSE operations cover MXCSR-only
   unmasking. XMM registers are caller-saved under the SysV ABI. */
.globl nelibc_fenv_raise_hw
.type nelibc_fenv_raise_hw,@function
nelibc_fenv_raise_hw:
  pushq %rbx
  subq $16, %rsp
  movl %edi, %ebx
  stmxcsr 4(%rsp)
  fwait

  testl $0x01, %ebx
  jz .Lnelibc_fenv_raise_divzero
  testl $0x80, 4(%rsp)
  jnz .Lnelibc_fenv_raise_divzero
  pxor %xmm0, %xmm0
  pxor %xmm1, %xmm1
  divsd %xmm1, %xmm0

.Lnelibc_fenv_raise_divzero:
  testl $0x04, %ebx
  jz .Lnelibc_fenv_raise_overflow
  testl $0x200, 4(%rsp)
  jnz .Lnelibc_fenv_raise_overflow
  movsd .Lnelibc_fenv_one(%rip), %xmm0
  pxor %xmm1, %xmm1
  divsd %xmm1, %xmm0

.Lnelibc_fenv_raise_overflow:
  testl $0x08, %ebx
  jz .Lnelibc_fenv_raise_underflow
  testl $0x400, 4(%rsp)
  jnz .Lnelibc_fenv_raise_underflow
  movsd .Lnelibc_fenv_max(%rip), %xmm0
  movsd .Lnelibc_fenv_two(%rip), %xmm1
  mulsd %xmm1, %xmm0

.Lnelibc_fenv_raise_underflow:
  testl $0x10, %ebx
  jz .Lnelibc_fenv_raise_inexact
  testl $0x800, 4(%rsp)
  jnz .Lnelibc_fenv_raise_inexact
  movsd .Lnelibc_fenv_min(%rip), %xmm0
  movsd .Lnelibc_fenv_min(%rip), %xmm1
  mulsd %xmm1, %xmm0

.Lnelibc_fenv_raise_inexact:
  testl $0x20, %ebx
  jz .Lnelibc_fenv_raise_done
  testl $0x1000, 4(%rsp)
  jnz .Lnelibc_fenv_raise_done
  movsd .Lnelibc_fenv_one(%rip), %xmm0
  movsd .Lnelibc_fenv_three(%rip), %xmm1
  divsd %xmm1, %xmm0

.Lnelibc_fenv_raise_done:
  addq $16, %rsp
  popq %rbx
  ret
.size nelibc_fenv_raise_hw, .-nelibc_fenv_raise_hw

.section .rodata
.p2align 3
.Lnelibc_fenv_one:    .double 1.0
.Lnelibc_fenv_two:    .double 2.0
.Lnelibc_fenv_three:  .double 3.0
.Lnelibc_fenv_max:    .double 0x1.fffffffffffffp1023
.Lnelibc_fenv_min:    .double 0x1p-1022
.text

.globl nelibc_sqrt_hw
.type nelibc_sqrt_hw,@function
nelibc_sqrt_hw:
  sqrtsd %xmm0, %xmm0
  ret
.size nelibc_sqrt_hw, .-nelibc_sqrt_hw

.globl nelibc_sqrtf_hw
.type nelibc_sqrtf_hw,@function
nelibc_sqrtf_hw:
  sqrtss %xmm0, %xmm0
  ret
.size nelibc_sqrtf_hw, .-nelibc_sqrtf_hw

.globl nelibc_sqrtl_hw
.type nelibc_sqrtl_hw,@function
nelibc_sqrtl_hw:
  fldt 8(%rsp)
  fsqrt
  ret
.size nelibc_sqrtl_hw, .-nelibc_sqrtl_hw

/* SysV x86-64 long-double arguments are stack-passed: y at +8, x at +24.
   FPATAN computes atan2(y, x), including signed-zero and infinity quadrants. */
.globl nelibc_atan2l_hw
.type nelibc_atan2l_hw,@function
nelibc_atan2l_hw:
  fldt 8(%rsp)
  fldt 24(%rsp)
  fpatan
  ret
.size nelibc_atan2l_hw, .-nelibc_atan2l_hw

/* FYL2X evaluates y*log2(x) in the x87 extended-precision format. */
.globl nelibc_logl_hw
.type nelibc_logl_hw,@function
nelibc_logl_hw:
  fldln2
  fldt 8(%rsp)
  fyl2x
  ret
.size nelibc_logl_hw, .-nelibc_logl_hw

.globl nelibc_log2l_hw
.type nelibc_log2l_hw,@function
nelibc_log2l_hw:
  fld1
  fldt 8(%rsp)
  fyl2x
  ret
.size nelibc_log2l_hw, .-nelibc_log2l_hw

/* Accurate log2(1+x) for the range-reduced mantissa used by powl. */
.globl nelibc_log2p1l_small_hw
.type nelibc_log2p1l_small_hw,@function
nelibc_log2p1l_small_hw:
  fld1
  fldt 8(%rsp)
  fyl2xp1
  ret
.size nelibc_log2p1l_small_hw, .-nelibc_log2p1l_small_hw

.globl nelibc_log10l_hw
.type nelibc_log10l_hw,@function
nelibc_log10l_hw:
  fldlg2
  fldt 8(%rsp)
  fyl2x
  ret
.size nelibc_log10l_hw, .-nelibc_log10l_hw

/* Accurate y*log2(1+x) for |x| < 1; used with y=ln(2) for log1pl. */
.globl nelibc_log1pl_small_hw
.type nelibc_log1pl_small_hw,@function
nelibc_log1pl_small_hw:
  fldln2
  fldt 8(%rsp)
  fyl2xp1
  ret
.size nelibc_log1pl_small_hw, .-nelibc_log1pl_small_hw

/* Compute exp(x): split x*log2(e) into an integer scale and an F2XM1
   fraction, then apply the scale without rounding through binary64. */
.globl nelibc_expl_hw
.type nelibc_expl_hw,@function
nelibc_expl_hw:
  subq $16, %rsp
  fldt 24(%rsp)
  fldl2e
  fmulp %st, %st(1)
  fld %st(0)
  frndint
  fistpq (%rsp)
  fstp %st(0)
  fildq (%rsp)
  fldl .Lnelibc_ln2_hi(%rip)
  fmulp %st, %st(1)
  fldt 24(%rsp)
  fsub %st(1), %st(0)
  fstp %st(1)
  fildq (%rsp)
  fldl .Lnelibc_ln2_lo(%rip)
  fmulp %st, %st(1)
  fsubrp %st, %st(1)
  fldl2e
  fmulp %st, %st(1)
  f2xm1
  fld1
  faddp %st, %st(1)
  fildq (%rsp)
  fxch %st(1)
  fscale
  fstp %st(1)
  addq $16, %rsp
  ret
.size nelibc_expl_hw, .-nelibc_expl_hw

/* Compute exp(x)/2 by reducing as exp(x), then decrementing the binary scale.
   This preserves the sinh/cosh overflow boundary without an intermediate exp overflow. */
.globl nelibc_expl_half_hw
.type nelibc_expl_half_hw,@function
nelibc_expl_half_hw:
  subq $16, %rsp
  fldt 24(%rsp)
  fldl2e
  fmulp %st, %st(1)
  fld %st(0)
  frndint
  fistpq (%rsp)
  fstp %st(0)
  fildq (%rsp)
  fldl .Lnelibc_ln2_hi(%rip)
  fmulp %st, %st(1)
  fldt 24(%rsp)
  fsub %st(1), %st(0)
  fstp %st(1)
  fildq (%rsp)
  fldl .Lnelibc_ln2_lo(%rip)
  fmulp %st, %st(1)
  fsubrp %st, %st(1)
  fldl2e
  fmulp %st, %st(1)
  f2xm1
  fld1
  faddp %st, %st(1)
  subq $1, (%rsp)
  fildq (%rsp)
  fxch %st(1)
  fscale
  fstp %st(1)
  addq $16, %rsp
  ret
.size nelibc_expl_half_hw, .-nelibc_expl_half_hw

/* Compute 2^x directly: split x into an integral scale and a fraction. */
.globl nelibc_exp2l_hw
.type nelibc_exp2l_hw,@function
nelibc_exp2l_hw:
  fldt 8(%rsp)
  fld %st(0)
  frndint
  fxch %st(1)
  fsub %st(1), %st(0)
  f2xm1
  fld1
  faddp %st, %st(1)
  fscale
  fstp %st(1)
  ret
.size nelibc_exp2l_hw, .-nelibc_exp2l_hw

/* Return sinl(value) and store cosl(value) through the pointer. */
.globl nelibc_sincosl_hw
.type nelibc_sincosl_hw,@function
nelibc_sincosl_hw:
  fldt 8(%rsp)
  fsincos
  fstpt (%rdi)
  ret
.size nelibc_sincosl_hw, .-nelibc_sincosl_hw

/* Evaluate tanl on a Payne-Hanek-reduced binary80 argument. */
.globl nelibc_tanl_hw
.type nelibc_tanl_hw,@function
nelibc_tanl_hw:
  fldt 8(%rsp)
  fptan
  fstp %st(0)
  ret
.size nelibc_tanl_hw, .-nelibc_tanl_hw

.section .rodata
.p2align 4
.Lnelibc_ln2_hi:
  .double 0.693147180369123816490
.Lnelibc_ln2_lo:
  .double 1.90821492927058770002e-10
.p2align 4
.Lnelibc_pio2_hi:
  .byte 0x35, 0xc2, 0x68, 0x21, 0xa2, 0xda, 0x0f, 0xc9, 0xff, 0x3f
  .zero 6
.Lnelibc_pio2_lo:
  .byte 0xbb, 0x8c, 0x8f, 0xfc, 0xd1, 0x75, 0xe6, 0xec, 0xbd, 0xbf
  .zero 6
.text

.globl nelibc_pio2l_hi_hw
.type nelibc_pio2l_hi_hw,@function
nelibc_pio2l_hi_hw:
  fldt .Lnelibc_pio2_hi(%rip)
  ret
.size nelibc_pio2l_hi_hw, .-nelibc_pio2l_hi_hw

.globl nelibc_pio2l_lo_hw
.type nelibc_pio2l_lo_hw,@function
nelibc_pio2l_lo_hw:
  fldt .Lnelibc_pio2_lo(%rip)
  ret
.size nelibc_pio2l_lo_hw, .-nelibc_pio2l_lo_hw

/* Accurate cancellation-free expm1(x) for inputs scaled into [-1,1]. */
.globl nelibc_expm1l_small_hw
.type nelibc_expm1l_small_hw,@function
nelibc_expm1l_small_hw:
  fldl2e
  fldt 8(%rsp)
  fmulp %st, %st(1)
  f2xm1
  ret
.size nelibc_expm1l_small_hw, .-nelibc_expm1l_small_hw

.globl nelibc_x87_rounding_mode
.type nelibc_x87_rounding_mode,@function
nelibc_x87_rounding_mode:
  subq $8, %rsp
  fnstcw (%rsp)
  movzwl (%rsp), %eax
  andl $0x0c00, %eax
  addq $8, %rsp
  ret
.size nelibc_x87_rounding_mode, .-nelibc_x87_rounding_mode

.globl nelibc_x87_pi
.type nelibc_x87_pi,@function
nelibc_x87_pi:
  fldpi
  ret
.size nelibc_x87_pi, .-nelibc_x87_pi

/* Merge standard FE_* exception bits into the active x87 and SSE state. */
.globl nelibc_math_raise_except
.type nelibc_math_raise_except,@function
nelibc_math_raise_except:
  subq $40, %rsp
  fnstenv 0(%rsp)
  orw %di, 4(%rsp)
  fldenv 0(%rsp)
  stmxcsr 32(%rsp)
  movl 32(%rsp), %eax
  orl %edi, %eax
  movl %eax, 32(%rsp)
  ldmxcsr 32(%rsp)
  addq $40, %rsp
  ret
.size nelibc_math_raise_except, .-nelibc_math_raise_except

.globl nelibc_neg_one_up_hw
.type nelibc_neg_one_up_hw,@function
nelibc_neg_one_up_hw:
  fldt .Lnelibc_neg_one_up(%rip)
  ret
.size nelibc_neg_one_up_hw, .-nelibc_neg_one_up_hw

.section .rodata
.p2align 4
.Lnelibc_neg_one_up:
  .byte 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xfe, 0xbf
.zero 6
.text

/* FXTRACT returns the significand in ST(0) and the exponent in ST(1). */
.globl nelibc_frexpl_hw
.type nelibc_frexpl_hw,@function
nelibc_frexpl_hw:
  movq 8(%rsp), %rax
  movzwl 16(%rsp), %edx
  andl $0x7fff, %edx
  orq %rax, %rdx
  jne 1f
  movl $0, (%rdi)
  fldt 8(%rsp)
  ret
1:
  fldt 8(%rsp)
  fxam
  fnstsw %ax
  andw $0x4500, %ax
  cmpw $0x0500, %ax
  je 2f
  cmpw $0x0100, %ax
  je 2f
  fxtract
  fxch %st(1)
  fistpl (%rdi)
  addl $1, (%rdi)
  fldt .Ltwo
  fdivrp %st, %st(1)
  ret
2:
  movl $0, (%rdi)
  ret
.size nelibc_frexpl_hw, .-nelibc_frexpl_hw

/* Round a copy toward zero, store it, and return x - integral. */
.globl nelibc_modfl_hw
.type nelibc_modfl_hw,@function
nelibc_modfl_hw:
  subq $48, %rsp
  fnstsw 0(%rsp)
  fldt 56(%rsp)
  fxam
  fnstsw %ax
  andw $0x4500, %ax
  cmpw $0x0500, %ax
  je 2f
  fnstcw 2(%rsp)
  movzwl 2(%rsp), %eax
  andl $0xfffff3ff, %eax
  orl $0x0c00, %eax
  movw %ax, 4(%rsp)
  fldcw 4(%rsp)
  fld %st(0)
  frndint
  fstpt (%rdi)
  fldcw 2(%rsp)
  fldt (%rdi)
  fsubrp %st, %st(1)
  fnstenv 16(%rsp)
  testw $0x0020, 0(%rsp)
  jne 1f
  andw $0xffdf, 20(%rsp)
1:
  fldenv 16(%rsp)
  addq $48, %rsp
  ret
2:
  fstpt (%rdi)
  fldz
  addq $48, %rsp
  ret
.size nelibc_modfl_hw, .-nelibc_modfl_hw

.globl nelibc_ilogbl_hw
.type nelibc_ilogbl_hw,@function
nelibc_ilogbl_hw:
  movq 8(%rsp), %rax
  movzwl 16(%rsp), %edx
  andl $0x7fff, %edx
  testq %rax, %rax
  jne 1f
  testl %edx, %edx
  je 3f
1:
  cmpl $0x7fff, %edx
  jne 2f
  movabsq $0x8000000000000000, %rcx
  cmpq %rcx, %rax
  je 4f
  jmp 3f
2:
  fldt 8(%rsp)
  fxtract
  fxch %st(1)
  fistpl -4(%rsp)
  movl -4(%rsp), %eax
  fstp %st(0)
  ret
3:
  movl $0x80000000, %eax
  ret
4:
  movl $0x7fffffff, %eax
  ret
.size nelibc_ilogbl_hw, .-nelibc_ilogbl_hw

.globl nelibc_logbl_hw
.type nelibc_logbl_hw,@function
nelibc_logbl_hw:
  fldt 8(%rsp)
  fxtract
  fstp %st(0)
  ret
.size nelibc_logbl_hw, .-nelibc_logbl_hw

/* Scale x87 long double by an integer power of two and report ERANGE cases. */
.globl nelibc_ldexpl_hw
.type nelibc_ldexpl_hw,@function
nelibc_ldexpl_hw:
  subq $16, %rsp
  movq %rsi, %r8
  movl $0, (%r8)
  movl %edi, 0(%rsp)
  fildl 0(%rsp)
  fldt 24(%rsp)
  fscale
  fstp %st(1)
  fstpt 0(%rsp)
  jmp .Lscale_check
.size nelibc_ldexpl_hw, .-nelibc_ldexpl_hw

.globl nelibc_scalblnl_hw
.type nelibc_scalblnl_hw,@function
nelibc_scalblnl_hw:
  subq $16, %rsp
  movq %rsi, %r8
  movl $0, (%r8)
  movq %rdi, 0(%rsp)
  fildq 0(%rsp)
  fldt 24(%rsp)
  fscale
  fstp %st(1)
  fstpt 0(%rsp)
.Lscale_check:
  movq 24(%rsp), %rax
  movzwl 32(%rsp), %ecx
  andl $0x7fff, %ecx
  cmpl $0x7fff, %ecx
  je .Lscale_return
  testq %rax, %rax
  jne .Lscale_result
  testl %ecx, %ecx
  je .Lscale_return
.Lscale_result:
  movq 0(%rsp), %rax
  movzwl 8(%rsp), %ecx
  andl $0x7fff, %ecx
  testq %rax, %rax
  jne .Lscale_inf
  testl %ecx, %ecx
  jne .Lscale_return
  movl $1, (%r8)
  jmp .Lscale_return
.Lscale_inf:
  cmpl $0x7fff, %ecx
  jne .Lscale_return
  movabsq $0x8000000000000000, %rdx
  cmpq %rdx, %rax
  jne .Lscale_return
  movl $1, (%r8)
.Lscale_return:
  fldt 0(%rsp)
  addq $16, %rsp
  ret
.size nelibc_scalblnl_hw, .-nelibc_scalblnl_hw

/* mode: 0 trunc, 1 floor, 2 ceil, 3 rint, 4 nearbyint. */
.globl nelibc_roundl_hw
.type nelibc_roundl_hw,@function
nelibc_roundl_hw:
  subq $48, %rsp
  movl %edi, 4(%rsp)
  fnstsw %ax
  andw $0x0020, %ax
  movw %ax, 6(%rsp)
  fnstcw 0(%rsp)
  movzwl 0(%rsp), %eax
  cmpl $0, %edi
  je .Lroundl_trunc
  cmpl $1, %edi
  je .Lroundl_floor
  cmpl $2, %edi
  je .Lroundl_ceil
  jmp .Lroundl_apply
.Lroundl_trunc:
  andl $0xfffff3ff, %eax
  orl $0x0c00, %eax
  jmp .Lroundl_setcw
.Lroundl_floor:
  andl $0xfffff3ff, %eax
  orl $0x0400, %eax
  jmp .Lroundl_setcw
.Lroundl_ceil:
  andl $0xfffff3ff, %eax
  orl $0x0800, %eax
.Lroundl_setcw:
  movw %ax, 2(%rsp)
  fldcw 2(%rsp)
.Lroundl_apply:
  fldt 56(%rsp)
  frndint
  fldcw 0(%rsp)
  cmpl $4, 4(%rsp)
  jne .Lroundl_return
  fnstenv 16(%rsp)
  testw $0x0020, 6(%rsp)
  jne .Lroundl_restore_env
  andw $0xffdf, 20(%rsp)
.Lroundl_restore_env:
  fldenv 16(%rsp)
.Lroundl_return:
  addq $48, %rsp
  ret
.size nelibc_roundl_hw, .-nelibc_roundl_hw

/* roundl: nearest integral value, halfway cases away from zero. */
.globl nelibc_roundl_away_hw
.type nelibc_roundl_away_hw,@function
nelibc_roundl_away_hw:
  subq $48, %rsp
  movzwl 64(%rsp), %eax
  andl $0x8000, %eax
  movw %ax, 4(%rsp)
  fnstcw 0(%rsp)
  movzwl 0(%rsp), %eax
  andl $0xfffff0ff, %eax
  orl $0x0f00, %eax
  movw %ax, 2(%rsp)
  fldcw 2(%rsp)
  fldt 56(%rsp)
  fabs
  fldt .Lhalf80
  faddp %st, %st(1)
  frndint
  cmpw $0, 4(%rsp)
  je .Lroundl_away_done
  fchs
.Lroundl_away_done:
  fldcw 0(%rsp)
  addq $48, %rsp
  ret
.size nelibc_roundl_away_hw, .-nelibc_roundl_away_hw

/* fistpll follows the active x87 rounding direction and raises FE_INVALID
   when the result is outside the signed 64-bit range. */
.globl nelibc_lrintl_hw
.type nelibc_lrintl_hw,@function
nelibc_lrintl_hw:
  subq $16, %rsp
  fldt 24(%rsp)
  fistpll 0(%rsp)
  movq 0(%rsp), %rax
  addq $16, %rsp
  ret
.size nelibc_lrintl_hw, .-nelibc_lrintl_hw

/* lroundl/llroundl round halfway cases away from zero and suppress newly
   raised FE_INEXACT, while preserving pre-existing and invalid flags. */
.globl nelibc_lroundl_hw
.type nelibc_lroundl_hw,@function
nelibc_lroundl_hw:
  subq $64, %rsp
  fnstsw %ax
  andw $0x0020, %ax
  movw %ax, 6(%rsp)
  movzwl 80(%rsp), %eax
  andl $0x8000, %eax
  movw %ax, 4(%rsp)
  fnstcw 0(%rsp)
  movzwl 0(%rsp), %eax
  andl $0xfffff0ff, %eax
  orl $0x0f00, %eax
  movw %ax, 2(%rsp)
  fldcw 2(%rsp)
  fldt 72(%rsp)
  fabs
  fldt .Lhalf80
  faddp %st, %st(1)
  frndint
  cmpw $0, 4(%rsp)
  je .Llroundl_convert
  fchs
.Llroundl_convert:
  fistpll 8(%rsp)
  fldcw 0(%rsp)
  fnstenv 16(%rsp)
  testw $0x0020, 6(%rsp)
  jne .Llroundl_restore_env
  andw $0xffdf, 20(%rsp)
.Llroundl_restore_env:
  fldenv 16(%rsp)
  movq 8(%rsp), %rax
  addq $64, %rsp
  ret
.size nelibc_lroundl_hw, .-nelibc_lroundl_hw

.globl nelibc_fmodl_hw
.type nelibc_fmodl_hw,@function
nelibc_fmodl_hw:
  fldt 24(%rsp)
  fldt 8(%rsp)
1:
  fprem
  fnstsw %ax
  testw $0x0400, %ax
  jne 1b
  fstp %st(1)
  ret
.size nelibc_fmodl_hw, .-nelibc_fmodl_hw

.globl nelibc_remainderl_hw
.type nelibc_remainderl_hw,@function
nelibc_remainderl_hw:
  fldt 24(%rsp)
  fldt 8(%rsp)
1:
  fprem1
  fnstsw %ax
  testw $0x0400, %ax
  jne 1b
  fstp %st(1)
  ret
.size nelibc_remainderl_hw, .-nelibc_remainderl_hw

/* SysV x86-64 passes long double arguments in 16-byte stack slots. */
.globl nelibc_fabsl_hw
.type nelibc_fabsl_hw,@function
nelibc_fabsl_hw:
  fldt 8(%rsp)
  fabs
  ret
.size nelibc_fabsl_hw, .-nelibc_fabsl_hw

.globl nelibc_copysignl_hw
.type nelibc_copysignl_hw,@function
nelibc_copysignl_hw:
  subq $16, %rsp
  movq 24(%rsp), %rax
  movzwl 32(%rsp), %edx
  andl $0x7fff, %edx
  movzwl 48(%rsp), %ecx
  andl $0x8000, %ecx
  orl %ecx, %edx
  movq %rax, (%rsp)
  movw %dx, 8(%rsp)
  fldt (%rsp)
  addq $16, %rsp
  ret
.size nelibc_copysignl_hw, .-nelibc_copysignl_hw

/* CVT*2SI obeys MXCSR rounding control. The int64 intermediate is exact for
   every input below 2^52 (or 2^23 for float); larger finite values are already
   integral at their source precision. */
.globl nelibc_rint_hw
.type nelibc_rint_hw,@function
nelibc_rint_hw:
  ucomisd %xmm0, %xmm0
  jp 1f
  movq %xmm0, %r8
  movq %r8, %rax
  shlq $1, %rax
  shrq $53, %rax
  cmpl $0x433, %eax /* unbiased exponent >= 52 */
  jae 2f
  cvtsd2siq %xmm0, %rax
  cvtsi2sdq %rax, %xmm0
  testq %rax, %rax
  jne 3f
  shrq $63, %r8
  shlq $63, %r8
  movq %xmm0, %rdx
  orq %r8, %rdx
  movq %rdx, %xmm0
3:
  ret
1:
  addsd %xmm0, %xmm0
2:
  ret
.size nelibc_rint_hw, .-nelibc_rint_hw

.globl nelibc_nearbyint_hw
.type nelibc_nearbyint_hw,@function
nelibc_nearbyint_hw:
  subq $8, %rsp
  stmxcsr (%rsp)
  movl (%rsp), %eax
  orl $0x1000, %eax
  andl $0xffffffdf, %eax
  movl %eax, 4(%rsp)
  ldmxcsr 4(%rsp)
  ucomisd %xmm0, %xmm0
  jp 1f
  movq %xmm0, %r8
  movq %r8, %rax
  shlq $1, %rax
  shrq $53, %rax
  cmpl $0x433, %eax
  jae 2f
  cvtsd2siq %xmm0, %rax
  cvtsi2sdq %rax, %xmm0
  testq %rax, %rax
  jne 3f
  shrq $63, %r8
  shlq $63, %r8
  movq %xmm0, %rdx
  orq %r8, %rdx
  movq %rdx, %xmm0
3:
  stmxcsr 4(%rsp)
  movl 4(%rsp), %edx
  andl $0x1f, %edx
  movl (%rsp), %eax
  andl $0x0000ffff, %eax
  orl %eax, %edx
  movl %edx, 4(%rsp)
  ldmxcsr 4(%rsp)
  addq $8, %rsp
  ret
1:
  addsd %xmm0, %xmm0
2:
  stmxcsr 4(%rsp)
  movl 4(%rsp), %edx
  andl $0x1f, %edx
  movl (%rsp), %eax
  andl $0x0000ffff, %eax
  orl %eax, %edx
  movl %edx, 4(%rsp)
  ldmxcsr 4(%rsp)
  addq $8, %rsp
  ret
.size nelibc_nearbyint_hw, .-nelibc_nearbyint_hw

.globl nelibc_rintf_hw
.type nelibc_rintf_hw,@function
nelibc_rintf_hw:
  ucomiss %xmm0, %xmm0
  jp 1f
  movd %xmm0, %r8d
  movl %r8d, %eax
  shll $1, %eax
  shrl $24, %eax
  cmpl $0x96, %eax /* unbiased exponent >= 23 */
  jae 2f
  cvtss2siq %xmm0, %rax
  cvtsi2ssq %rax, %xmm0
  testq %rax, %rax
  jne 3f
  shrl $31, %r8d
  shll $31, %r8d
  movd %xmm0, %edx
  orl %r8d, %edx
  movd %edx, %xmm0
3:
  ret
1:
  addss %xmm0, %xmm0
2:
  ret
.size nelibc_rintf_hw, .-nelibc_rintf_hw

/* 64-bit CVT*2SI forms honor MXCSR and raise FE_INEXACT / FE_INVALID. */
.globl nelibc_lrint_hw
.type nelibc_lrint_hw,@function
nelibc_lrint_hw:
  cvtsd2siq %xmm0, %rax
  ret
.size nelibc_lrint_hw, .-nelibc_lrint_hw

.globl nelibc_lrintf_hw
.type nelibc_lrintf_hw,@function
nelibc_lrintf_hw:
  cvtss2siq %xmm0, %rax
  ret
.size nelibc_lrintf_hw, .-nelibc_lrintf_hw

/* The caller applies round-away-from-zero first; CVTT then converts exactly. */
.globl nelibc_lround_hw
.type nelibc_lround_hw,@function
nelibc_lround_hw:
  cvttsd2siq %xmm0, %rax
  ret
.size nelibc_lround_hw, .-nelibc_lround_hw

.globl nelibc_lroundf_hw
.type nelibc_lroundf_hw,@function
nelibc_lroundf_hw:
  cvttss2siq %xmm0, %rax
  ret
.size nelibc_lroundf_hw, .-nelibc_lroundf_hw

.globl nelibc_nearbyintf_hw
.type nelibc_nearbyintf_hw,@function
nelibc_nearbyintf_hw:
  subq $8, %rsp
  stmxcsr (%rsp)
  movl (%rsp), %eax
  orl $0x1000, %eax
  andl $0xffffffdf, %eax
  movl %eax, 4(%rsp)
  ldmxcsr 4(%rsp)
  ucomiss %xmm0, %xmm0
  jp 1f
  movd %xmm0, %r8d
  movl %r8d, %eax
  shll $1, %eax
  shrl $24, %eax
  cmpl $0x96, %eax
  jae 2f
  cvtss2siq %xmm0, %rax
  cvtsi2ssq %rax, %xmm0
  testq %rax, %rax
  jne 3f
  shrl $31, %r8d
  shll $31, %r8d
  movd %xmm0, %edx
  orl %r8d, %edx
  movd %edx, %xmm0
3:
  stmxcsr 4(%rsp)
  movl 4(%rsp), %edx
  andl $0x1f, %edx
  movl (%rsp), %eax
  andl $0x0000ffff, %eax
  orl %eax, %edx
  movl %edx, 4(%rsp)
  ldmxcsr 4(%rsp)
  addq $8, %rsp
  ret
1:
  addss %xmm0, %xmm0
2:
  stmxcsr 4(%rsp)
  movl 4(%rsp), %edx
  andl $0x1f, %edx
  movl (%rsp), %eax
  andl $0x0000ffff, %eax
  orl %eax, %edx
  movl %edx, 4(%rsp)
  ldmxcsr 4(%rsp)
  addq $8, %rsp
  ret
.size nelibc_nearbyintf_hw, .-nelibc_nearbyintf_hw

.globl nelibc_exp_overflow_hw
.type nelibc_exp_overflow_hw,@function
nelibc_exp_overflow_hw:
  movabsq $0x7fefffffffffffff, %rax
  movq %rax, %xmm0
  addsd %xmm0, %xmm0
  ret
.size nelibc_exp_overflow_hw, .-nelibc_exp_overflow_hw

.globl nelibc_exp_underflow_hw
.type nelibc_exp_underflow_hw,@function
nelibc_exp_underflow_hw:
  movq $1, %rax
  movq %rax, %xmm0
  mulsd .Lhalf(%rip), %xmm0
  ret
.size nelibc_exp_underflow_hw, .-nelibc_exp_underflow_hw

.globl nelibc_log_domain_error_hw
.type nelibc_log_domain_error_hw,@function
nelibc_log_domain_error_hw:
  pxor %xmm0, %xmm0
  divsd %xmm0, %xmm0
  ret
.size nelibc_log_domain_error_hw, .-nelibc_log_domain_error_hw

.globl nelibc_log_pole_hw
.type nelibc_log_pole_hw,@function
nelibc_log_pole_hw:
  movabsq $0xbff0000000000000, %rax
  movq %rax, %xmm0
  pxor %xmm1, %xmm1
  divsd %xmm1, %xmm0
  ret
.size nelibc_log_pole_hw, .-nelibc_log_pole_hw

.section .rodata
.p2align 3
.Lhalf:
  .double 0.5
.p2align 4
.Lhalf80:
  .quad 0x8000000000000000
  .word 0x3ffe
.p2align 4
.Ltwo:
  .quad 0x8000000000000000
  .word 0x4000

.text

/* FSCALE applies an integral power of two without converting through C math. */
.globl nelibc_ldexp_hw
.type nelibc_ldexp_hw,@function
nelibc_ldexp_hw:
  subq $24, %rsp
  movsd %xmm0, 0(%rsp)
  movl %edi, 8(%rsp)
  fildl 8(%rsp)
  fldl 0(%rsp)
  fscale
  fstp %st(1)
  fstpl 16(%rsp)
  movsd 16(%rsp), %xmm0
  addq $24, %rsp
  ret
.size nelibc_ldexp_hw, .-nelibc_ldexp_hw

.globl nelibc_ldexpf_hw
.type nelibc_ldexpf_hw,@function
nelibc_ldexpf_hw:
  subq $24, %rsp
  movss %xmm0, 0(%rsp)
  movl %edi, 8(%rsp)
  fildl 8(%rsp)
  flds 0(%rsp)
  fscale
  fstp %st(1)
  fstps 16(%rsp)
  movss 16(%rsp), %xmm0
  addq $24, %rsp
  ret
.size nelibc_ldexpf_hw, .-nelibc_ldexpf_hw

/* x87 FPREM computes truncating remainder; FPREM1 uses nearest-even. */
.globl nelibc_fmod_hw
.type nelibc_fmod_hw,@function
nelibc_fmod_hw:
  subq $24, %rsp
  movsd %xmm0, 0(%rsp)
  movsd %xmm1, 8(%rsp)
  fldl 8(%rsp)
  fldl 0(%rsp)
1:
  fprem
  fnstsw %ax
  testw $0x0400, %ax
  jne 1b
  fstp %st(1)
  fstpl 16(%rsp)
  movsd 16(%rsp), %xmm0
  addq $24, %rsp
  ret
.size nelibc_fmod_hw, .-nelibc_fmod_hw

.globl nelibc_remainder_hw
.type nelibc_remainder_hw,@function
nelibc_remainder_hw:
  subq $24, %rsp
  movsd %xmm0, 0(%rsp)
  movsd %xmm1, 8(%rsp)
  fldl 8(%rsp)
  fldl 0(%rsp)
2:
  fprem1
  fnstsw %ax
  testw $0x0400, %ax
  jne 2b
  fstp %st(1)
  fstpl 16(%rsp)
  movsd 16(%rsp), %xmm0
  addq $24, %rsp
  ret
.size nelibc_remainder_hw, .-nelibc_remainder_hw

/* FPREM1 returns the low quotient bits in C0/C3/C1 (Q2/Q1/Q0). */
.globl nelibc_remquo_hw
.type nelibc_remquo_hw,@function
nelibc_remquo_hw:
  subq $24, %rsp
  movsd %xmm0, 0(%rsp)
  movsd %xmm1, 8(%rsp)
  fldl 8(%rsp)
  fldl 0(%rsp)
1:
  fprem1
  fnstsw %ax
  testw $0x0400, %ax
  jne 1b
  fstp %st(1)
  fstpl 16(%rsp)
  movsd 16(%rsp), %xmm0
  movzwl %ax, %eax
  movl %eax, %ecx
  shrl $6, %ecx
  andl $4, %ecx
  movl %eax, %edx
  shrl $13, %edx
  andl $2, %edx
  orl %edx, %ecx
  movl %eax, %edx
  shrl $9, %edx
  andl $1, %edx
  orl %edx, %ecx
  movq 0(%rsp), %r8
  xorq 8(%rsp), %r8
  testq %r8, %r8
  jns 2f
  negl %ecx
2:
  movl %ecx, (%rdi)
  addq $24, %rsp
  ret
.size nelibc_remquo_hw, .-nelibc_remquo_hw

/* FPREM1 returns the low quotient bits in C0/C3/C1 (Q2/Q1/Q0). */
.globl nelibc_remquol_hw
.type nelibc_remquol_hw,@function
nelibc_remquol_hw:
  fldt 24(%rsp)
  fldt 8(%rsp)
3:
  fprem1
  fnstsw %ax
  testw $0x0400, %ax
  jne 3b
  movzwl %ax, %eax
  movl %eax, %ecx
  shrl $6, %ecx
  andl $4, %ecx
  movl %eax, %edx
  shrl $13, %edx
  andl $2, %edx
  orl %edx, %ecx
  movl %eax, %edx
  shrl $9, %edx
  andl $1, %edx
  orl %edx, %ecx
  movzwl 16(%rsp), %r8d
  xorw 32(%rsp), %r8w
  testw $0x8000, %r8w
  jz 4f
  negl %ecx
4:
  movl %ecx, (%rdi)
  fstp %st(1)
  ret
.size nelibc_remquol_hw, .-nelibc_remquol_hw

.section .note.GNU-stack,"",@progbits
