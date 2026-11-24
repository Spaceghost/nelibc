#include <asm/unistd.h>

/* AAPCS64 passes the syscall number before the arguments. Linux expects
 * the number in x8 and arguments in x0 through x5. */
.section .text.nelibc_raw_syscall,"ax",%progbits
.globl nelibc_raw_syscall3_real
.type nelibc_raw_syscall3_real, %function
nelibc_raw_syscall3_real:
  mov x8, x0
  mov x0, x1
  mov x1, x2
  mov x2, x3
  svc #0
  ret
.size nelibc_raw_syscall3_real, .-nelibc_raw_syscall3_real
.weak nelibc_raw_syscall3
.set nelibc_raw_syscall3, nelibc_raw_syscall3_real

.globl nelibc_raw_syscall6_real
.type nelibc_raw_syscall6_real, %function
nelibc_raw_syscall6_real:
  mov x8, x0
  mov x0, x1
  mov x1, x2
  mov x2, x3
  mov x3, x4
  mov x4, x5
  mov x5, x6
  svc #0
  ret
.size nelibc_raw_syscall6_real, .-nelibc_raw_syscall6_real
.weak nelibc_raw_syscall6
.set nelibc_raw_syscall6, nelibc_raw_syscall6_real

.text
.globl nelibc_rt_sigreturn
.type nelibc_rt_sigreturn, %function
nelibc_rt_sigreturn:
  mov x8, #__NR_rt_sigreturn
  svc #0
  brk #0
.size nelibc_rt_sigreturn, .-nelibc_rt_sigreturn

.section .note.GNU-stack,"",%progbits
