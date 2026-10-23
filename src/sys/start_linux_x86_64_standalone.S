.text
.globl _start
.type _start, @function
_start:
  xorq %rbp, %rbp

  # Find AT_PHDR and AT_PHNUM in the initial process stack's auxv.
  movq %rsp, %r12
  addq $8, %r12                 # argc
.Lskip_argv:
  cmpq $0, (%r12)
  leaq 8(%r12), %r12
  jne .Lskip_argv
  movq %r12, environ(%rip)     # environ points just past argv's NULL entry
.Lskip_envp:
  cmpq $0, (%r12)
  leaq 8(%r12), %r12
  jne .Lskip_envp
  xorq %r13, %r13              # program header address
  xorq %r14, %r14              # program header count
.Lauxv:
  movq (%r12), %rax
  testq %rax, %rax              # AT_NULL
  jz .Lauxv_done
  cmpq $3, %rax                 # AT_PHDR
  jne .Lauxv_phnum
  movq 8(%r12), %r13
  jmp .Lauxv_next
.Lauxv_phnum:
  cmpq $5, %rax                 # AT_PHNUM
  jne .Lauxv_next
  movq 8(%r12), %r14
.Lauxv_next:
  addq $16, %r12
  jmp .Lauxv

.Lauxv_done:
  testq %r13, %r13
  jz .Lstartup_failed
  testq %r14, %r14
  jz .Lstartup_failed

  # Locate PT_TLS (Elf64_Phdr: type=0, vaddr=16, filesz=32,
  # memsz=40, align=48). No PT_TLS is a valid empty TLS image.
  xorq %r15, %r15              # PT_TLS p_vaddr (initial bytes)
  xorq %rbp, %rbp              # PT_TLS p_memsz
  xorq %rbx, %rbx              # PT_TLS p_filesz
  xorq %r9, %r9                # whether PT_TLS was already seen
  movq $1, %r10                # PT_TLS p_align, at least 1
.Lphdr_loop:
  testq %r14, %r14
  jz .Lphdr_done
  cmpl $7, (%r13)              # PT_TLS is a 32-bit p_type
  jne .Lphdr_next
  testq %r9, %r9                # reject duplicate PT_TLS
  jnz .Lstartup_failed
  movq $1, %r9
  movq 16(%r13), %r15
  movq 32(%r13), %rbx
  movq 40(%r13), %rbp
  movq 48(%r13), %r10
  movq %r15, nelibc_tls_template_start(%rip)
  movq %rbx, nelibc_tls_template_filesz(%rip)
  movq %rbp, nelibc_tls_template_memsz(%rip)
  movq %r10, nelibc_tls_template_align(%rip)
.Lphdr_next:
  addq $56, %r13
  decq %r14
  jmp .Lphdr_loop
.Lphdr_done:
  cmpq %rbp, %rbx              # filesz must not exceed memsz
  ja .Lstartup_failed
  testq %r10, %r10
  jnz .Lp_align_nonzero
  movq $1, %r10                # ELF permits p_align == 0
.Lp_align_nonzero:
  cmpq $16, %r10
  jae .Lalign_ready
  movq $16, %r10               # align TP for the x86_64 TCB
.Lalign_ready:
  # ELF p_align is required to be a power of two.
  movq %r10, %rax
  decq %rax
  testq %rax, %r10
  jnz .Lstartup_failed
  movq %r10, %r14              # preserve TLS alignment across syscall

  # Round memsz to p_align. x86_64 local-exec offsets are relative to this
  # rounded TLS size, not merely PT_TLS.p_memsz.
  movq %rbp, %rsi
  movq %r14, %rax
  decq %rax
  addq %rax, %rsi
  jc .Lstartup_failed
  movq %r14, %rax
  negq %rax
  andq %rax, %rsi
  # mmap(rounded_tls_size + align + 64, PROT_READ|PROT_WRITE,
  # MAP_PRIVATE|MAP_ANONYMOUS). Keep a writable TCB after the TLS image.
  addq %r14, %rsi
  jc .Lstartup_failed
  addq $64, %rsi
  jc .Lstartup_failed
  xorq %rdi, %rdi
  movq $3, %rdx
  movq $34, %r10
  movq $-1, %r8
  xorq %r9, %r9
  movq $9, %rax
  syscall
  cmpq $-4095, %rax
  jae .Lstartup_failed
  movq %rax, %r12              # mapping base

  # TP = align_up(mapping + rounded_tls_size, p_align). TLS bytes occupy
  # [TP - rounded_tls_size, TP), matching linked local-exec offsets.
  movq %rbp, %rax
  movq %r14, %rsi
  decq %rsi
  addq %rsi, %rax
  movq %r14, %rsi
  negq %rsi
  andq %rsi, %rax              # rounded TLS size
  leaq (%r12,%rax), %r13
  movq %r14, %rax
  decq %rax
  addq %rax, %r13
  movq %r14, %rax
  negq %rax
  andq %rax, %r13
  movq %r13, %rdi
  movq %rbp, %rax
  movq %r14, %rsi
  decq %rsi
  addq %rsi, %rax
  movq %r14, %rsi
  negq %rsi
  andq %rsi, %rax
  subq %rax, %rdi              # TLS destination
  movq %r15, %rsi              # PT_TLS source in the loaded image
  movq %rbx, %rcx              # initialized byte count
  rep movsb
  movq %r13, %rdi
  subq %rax, %rdi
  addq %rbx, %rdi
  # Clear .tbss and any tail padding in the rounded TLS image.
  movq %r14, %rcx
  decq %rcx
  addq %rbp, %rcx
  movq %r14, %rax
  negq %rax
  andq %rax, %rcx
  subq %rbx, %rcx
  xorq %rax, %rax
  rep stosb

  # Minimal x86_64 TCB: its self pointer at FS:0. Standalone uses static TLS
  # and reserved TCB slots for identity, pthread context, and thread keys.
  movq %r13, (%r13)
  movq %r13, %rsi
  movl $158, %eax              # arch_prctl(ARCH_SET_FS, TP)
  movl $0x1002, %edi
  syscall
  testq %rax, %rax
  jne .Lstartup_failed

  # Nelua noentrypoint modules emit initialization into their generated
  # nelua_main function. Initialize standalone stdio before user main, then
  # pass the argc and argv from the kernel-provided initial stack.
  call nelibc_stdio_init
  movq (%rsp), %rdi
  leaq 8(%rsp), %rsi
  call main
  movl %eax, %edi
  call exit
  ud2
.Lstartup_failed:
  movl $127, %edi
.Lexit:
  movl $60, %eax
  syscall
  ud2
.size _start, .-_start

.bss
.globl environ
.type environ, @object
.size environ, 8
environ:
  .zero 8

# Private metadata consumed by the initial-thread clone helper.
.section .bss
.balign 8
.globl nelibc_tls_template_start
nelibc_tls_template_start: .zero 8
.globl nelibc_tls_template_filesz
nelibc_tls_template_filesz: .zero 8
.globl nelibc_tls_template_memsz
nelibc_tls_template_memsz: .zero 8
.globl nelibc_tls_template_align
nelibc_tls_template_align: .zero 8

.text
.globl nelibc_tls_layout
.type nelibc_tls_layout, @function
nelibc_tls_layout:
  movq nelibc_tls_template_start(%rip), %rax
  movq %rax, 0(%rdi)
  movq nelibc_tls_template_filesz(%rip), %rax
  movq %rax, 8(%rdi)
  movq nelibc_tls_template_memsz(%rip), %rax
  movq %rax, 16(%rdi)
  movq nelibc_tls_template_align(%rip), %rax
  movq %rax, 24(%rdi)
  ret
.size nelibc_tls_layout, .-nelibc_tls_layout

# long nelibc_clone_thread(start_fn, arg, stack_top, tls_tp, tid_ptr)
# Returns the child's tid in the parent and starts start_fn(arg) in the child.
.globl nelibc_clone_thread
.type nelibc_clone_thread, @function
nelibc_clone_thread:
  pushq %r12
  pushq %r13
  movq %rdi, %r12              # child entry function
  movq %rsi, %r13              # child entry argument
  movq %rdx, %rsi              # child stack
  movq %r8, %rdx               # parent_tid
  movq %r8, %r10               # child_tid
  movq %rcx, %r8               # TLS base (CLONE_SETTLS)
  testq %r8, %r8
  jnz .Lclone_with_tid
  movq $0x0d0f00, %rdi         # shared resources without child TID reporting
  jmp .Lclone_flags_ready
.Lclone_with_tid:
  movq $0x3d0f00, %rdi         # shared process resources + TID words
.Lclone_flags_ready:
  movq $56, %rax               # __NR_clone
  syscall
  testq %rax, %rax
  jz .Lclone_child
  popq %r13
  popq %r12
  ret
.Lclone_child:
  movq %r13, %rdi
  call *%r12
  movl %eax, %edi
  movq $60, %rax               # __NR_exit (this thread only)
  syscall
  ud2
.size nelibc_clone_thread, .-nelibc_clone_thread

.section .note.GNU-stack,"",@progbits
