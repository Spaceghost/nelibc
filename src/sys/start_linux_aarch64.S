/* Entry point for freestanding programs with no TLS or constructors. */
#include <asm/unistd.h>

.text
.globl _start
.type _start, %function
_start:
  mov x29, xzr
  mov x30, xzr
  ldr x0, [sp]                  // argc
  add x1, sp, #8                // argv
  add x2, x1, x0, lsl #3
  add x2, x2, #8                // envp follows the argv terminator
  bl main
  mov x8, #__NR_exit_group
  svc #0
  brk #0
.size _start, .-_start

.section .note.GNU-stack,"",%progbits
