/* Linux AArch64 jmp_buf. The machine context holds x19–x30, SP and
 * the low 64 bits of v8–v15. The flag and signal set follow it. */

.equ JB_SP, 96
.equ JB_FP, 112
.equ JB_SAVEMASK, 176
.equ JB_MASK, 184

.text
.globl setjmp, _setjmp
.type setjmp, %function
.type _setjmp, %function
setjmp:
_setjmp:
.Lsave:
  stp x19, x20, [x0, #0]
  stp x21, x22, [x0, #16]
  stp x23, x24, [x0, #32]
  stp x25, x26, [x0, #48]
  stp x27, x28, [x0, #64]
  stp x29, x30, [x0, #80]
  mov x2, sp
  str x2, [x0, #JB_SP]
  stp d8, d9, [x0, #JB_FP]
  stp d10, d11, [x0, #JB_FP + 16]
  stp d12, d13, [x0, #JB_FP + 32]
  stp d14, d15, [x0, #JB_FP + 48]
  mov w0, wzr
  ret
.size setjmp, .-setjmp
.size _setjmp, .-_setjmp

.globl longjmp, _longjmp
.type longjmp, %function
.type _longjmp, %function
longjmp:
_longjmp:
.Lrestore:
  ldp x19, x20, [x0, #0]
  ldp x21, x22, [x0, #16]
  ldp x23, x24, [x0, #32]
  ldp x25, x26, [x0, #48]
  ldp x27, x28, [x0, #64]
  ldp x29, x30, [x0, #80]
  ldr x2, [x0, #JB_SP]
  ldp d8, d9, [x0, #JB_FP]
  ldp d10, d11, [x0, #JB_FP + 16]
  ldp d12, d13, [x0, #JB_FP + 32]
  ldp d14, d15, [x0, #JB_FP + 48]
  mov sp, x2
  cmp w1, #0
  csinc w0, w1, wzr, ne
  ret
.size longjmp, .-longjmp
.size _longjmp, .-_longjmp

.globl sigsetjmp
.type sigsetjmp, %function
sigsetjmp:
  uxtw x1, w1
  str x1, [x0, #JB_SAVEMASK]
  cbz w1, .Lsave
  stp x0, x30, [sp, #-16]!
  add x2, x0, #JB_MASK
  mov x1, xzr
  mov w0, wzr
  bl sigprocmask
  ldp x0, x30, [sp], #16
  b .Lsave
.size sigsetjmp, .-sigsetjmp

.globl siglongjmp
.type siglongjmp, %function
siglongjmp:
  ldr x2, [x0, #JB_SAVEMASK]
  cbz x2, .Lrestore
  stp x0, x1, [sp, #-16]!
  add x1, x0, #JB_MASK
  mov x2, xzr
  mov w0, #2                   // SIG_SETMASK
  bl sigprocmask
  ldp x0, x1, [sp], #16
  b .Lrestore
.size siglongjmp, .-siglongjmp

.section .note.GNU-stack,"",%progbits
