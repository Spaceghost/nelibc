#include <asm/unistd.h>
#include <linux/sched.h>

.text
.globl nelibc_clone_thread
.type nelibc_clone_thread, %function
nelibc_clone_thread:
  stp x19, x20, [sp, #-16]!
  mov x19, x0                  // child start function
  mov x20, x1                  // child argument
  mov x1, x2                   // child stack
  mov x2, x4                   // parent_tid
  // x3 is already the AArch64 clone TLS argument.
  // x4 is already child_tid.
  ldr x0, =(CLONE_VM | CLONE_FS | CLONE_FILES | CLONE_SIGHAND | CLONE_THREAD | CLONE_SYSVSEM | CLONE_SETTLS | CLONE_PARENT_SETTID | CLONE_CHILD_CLEARTID)
  mov x8, #__NR_clone
  svc #0
  cbz x0, .Lchild
  ldp x19, x20, [sp], #16
  ret
.Lchild:
  mov x29, xzr
  mov x0, x20
  blr x19
  mov x8, #__NR_exit           // terminate only the calling thread
  svc #0
  brk #0
.size nelibc_clone_thread, .-nelibc_clone_thread

/* Switch stacks before releasing both the worker stack and TLS. */
.globl nelibc_thread_detached_exit
.type nelibc_thread_detached_exit, %function
nelibc_thread_detached_exit:
  add x6, x4, x5
  mov sp, x6
  mov x8, #__NR_munmap
  svc #0
  mov x0, x2
  mov x1, x3
  mov x8, #__NR_munmap
  svc #0
  mov x0, x4
  mov x1, x5
  mov x8, #__NR_munmap
  svc #0
  mov x0, xzr
  mov x8, #__NR_exit
  svc #0
  brk #0
.size nelibc_thread_detached_exit, .-nelibc_thread_detached_exit

.section .note.GNU-stack,"",%progbits
