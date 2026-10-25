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

.section .note.GNU-stack,"",@progbits
