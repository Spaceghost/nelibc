.text
.globl nelibc_raw_syscall3_real
.type nelibc_raw_syscall3_real, @function
nelibc_raw_syscall3_real:
  movq %rdi, %rax
  movq %rsi, %rdi
  movq %rdx, %rsi
  movq %rcx, %rdx
  syscall
  ret
.size nelibc_raw_syscall3_real, .-nelibc_raw_syscall3_real
/* Weak alias lets the smoke harness inject one syscall result. */
.weak nelibc_raw_syscall3
.set nelibc_raw_syscall3, nelibc_raw_syscall3_real

.globl nelibc_raw_syscall6_real
.type nelibc_raw_syscall6_real, @function
nelibc_raw_syscall6_real:
  movq %rdi, %rax
  movq %rsi, %rdi
  movq %rdx, %rsi
  movq %rcx, %rdx
  movq %r8, %r10
  movq %r9, %r8
  movq 8(%rsp), %r9
  syscall
  ret
.size nelibc_raw_syscall6_real, .-nelibc_raw_syscall6_real
/* Weak alias lets the smoke harness inject one syscall result. */
.weak nelibc_raw_syscall6
.set nelibc_raw_syscall6, nelibc_raw_syscall6_real

.globl nelibc_allocator_lock
.type nelibc_allocator_lock, @function
nelibc_allocator_lock:
  leaq nelibc_allocator_lock_word(%rip), %rdx
1:
  cmpl $0, (%rdx)
  jne 2f
  movl $1, %eax
  xchgl %eax, (%rdx)
  testl %eax, %eax
  jz 3f
2:
  pause
  jmp 1b
3:
  ret
.size nelibc_allocator_lock, .-nelibc_allocator_lock

.globl nelibc_allocator_unlock
.type nelibc_allocator_unlock, @function
nelibc_allocator_unlock:
  movl $0, nelibc_allocator_lock_word(%rip)
  ret
.size nelibc_allocator_unlock, .-nelibc_allocator_unlock

/* x86-64 aligned pointer stores/loads are atomic and TSO supplies release /
   acquire ordering. Function boundaries also prevent compiler reordering. */
.globl nelibc_allocator_slab_registry_publish
.type nelibc_allocator_slab_registry_publish, @function
nelibc_allocator_slab_registry_publish:
  movq %rdi, nelibc_allocator_slab_registry_head(%rip)
  ret
.size nelibc_allocator_slab_registry_publish, .-nelibc_allocator_slab_registry_publish

.globl nelibc_allocator_slab_registry
.type nelibc_allocator_slab_registry, @function
nelibc_allocator_slab_registry:
  movq nelibc_allocator_slab_registry_head(%rip), %rax
  ret
.size nelibc_allocator_slab_registry, .-nelibc_allocator_slab_registry

/* Direct mappings can be removed and unmapped. Readers announce their
   critical section; writers close the gate and drain readers before unlink. */
.globl nelibc_direct_registry_publish
.type nelibc_direct_registry_publish, @function
nelibc_direct_registry_publish:
  movq %rdi, nelibc_direct_registry_head_word(%rip)
  ret
.size nelibc_direct_registry_publish, .-nelibc_direct_registry_publish

.globl nelibc_direct_registry_head
.type nelibc_direct_registry_head, @function
nelibc_direct_registry_head:
  movq nelibc_direct_registry_head_word(%rip), %rax
  ret
.size nelibc_direct_registry_head, .-nelibc_direct_registry_head

.globl nelibc_direct_registry_read_enter
.type nelibc_direct_registry_read_enter, @function
nelibc_direct_registry_read_enter:
0:
  cmpl $0, nelibc_direct_registry_writer(%rip)
  jne 3f
  lock incq nelibc_direct_registry_readers(%rip)
  cmpl $0, nelibc_direct_registry_writer(%rip)
  je 2f
  lock decq nelibc_direct_registry_readers(%rip)
3:
  pause
  jmp 0b
2:
  ret
.size nelibc_direct_registry_read_enter, .-nelibc_direct_registry_read_enter

.globl nelibc_direct_registry_read_leave
.type nelibc_direct_registry_read_leave, @function
nelibc_direct_registry_read_leave:
  lock decq nelibc_direct_registry_readers(%rip)
  ret
.size nelibc_direct_registry_read_leave, .-nelibc_direct_registry_read_leave

.globl nelibc_direct_registry_write_begin
.type nelibc_direct_registry_write_begin, @function
nelibc_direct_registry_write_begin:
  movl $1, %eax
  xchgl %eax, nelibc_direct_registry_writer(%rip)
1:
  cmpq $0, nelibc_direct_registry_readers(%rip)
  je 2f
  pause
  jmp 1b
2:
  ret
.size nelibc_direct_registry_write_begin, .-nelibc_direct_registry_write_begin

.globl nelibc_direct_registry_write_end
.type nelibc_direct_registry_write_end, @function
nelibc_direct_registry_write_end:
  movl $0, nelibc_direct_registry_writer(%rip)
  ret
.size nelibc_direct_registry_write_end, .-nelibc_direct_registry_write_end

.bss
.align 4
nelibc_allocator_lock_word:
  .long 0
.align 8
nelibc_allocator_slab_registry_head:
  .quad 0
.align 8
nelibc_direct_registry_head_word:
  .quad 0
nelibc_direct_registry_readers:
  .quad 0
.align 4
nelibc_direct_registry_writer:
  .long 0

.text
.globl nelibc_rt_sigreturn
.type nelibc_rt_sigreturn, @function
nelibc_rt_sigreturn:
  movq $15, %rax
  syscall
.size nelibc_rt_sigreturn, .-nelibc_rt_sigreturn

.section .note.GNU-stack,"",@progbits
