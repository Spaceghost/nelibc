/* Shared x86-64 SSE2 helpers for TinyCC, GCC, and Zig cc memory paths. */
.text

#ifndef NELIBC_MEMSCAN_AVX2_THRESHOLD
#define NELIBC_MEMSCAN_AVX2_THRESHOLD 2048
#endif
#ifndef NELIBC_MEMSCAN_AVX512_THRESHOLD
#define NELIBC_MEMSCAN_AVX512_THRESHOLD 16384
#endif
#ifndef NELIBC_MEMSCAN_AVX2_BLOCK_SIZE
#define NELIBC_MEMSCAN_AVX2_BLOCK_SIZE 256
#endif
#ifndef NELIBC_MEMMOVE_AVX2_THRESHOLD
#define NELIBC_MEMMOVE_AVX2_THRESHOLD 2048
#endif

.globl nelibc_x86_64_rep_memcpy
.type nelibc_x86_64_rep_memcpy,@function
nelibc_x86_64_rep_memcpy:
#ifdef NELIBC_TCC_PUBLIC_MEMCPY
.globl memcpy
.type memcpy,@function
memcpy:
#endif
#if defined(NELIBC_TCC_PUBLIC_MEMCPY) && !defined(NELIBC_TCC_MEMCPY_REP_ONLY)
  /* Avoid ERMS setup cost for the fixed 64-byte TinyCC copy workload. */
  cmpq $64, %rdx
  jne .Lmemcpy_rep
  movdqu 0(%rsi), %xmm0
  movdqu 16(%rsi), %xmm1
  movdqu 32(%rsi), %xmm2
  movdqu 48(%rsi), %xmm3
  movdqu %xmm0, 0(%rdi)
  movdqu %xmm1, 16(%rdi)
  movdqu %xmm2, 32(%rdi)
  movdqu %xmm3, 48(%rdi)
  movq %rdi, %rax
  ret
.Lmemcpy_rep:
#endif
  movq %rdi, %rax
  movq %rdx, %rcx
  rep movsb
  ret
.size nelibc_x86_64_rep_memcpy, .-nelibc_x86_64_rep_memcpy
#ifdef NELIBC_TCC_PUBLIC_MEMCPY
.size memcpy, .-memcpy
#endif

.globl nelibc_x86_64_sse_strlen
.type nelibc_x86_64_sse_strlen,@function
nelibc_x86_64_sse_strlen:
  movq %rdi, %rsi
  pxor %xmm0, %xmm0
.Lstrlen_align:
  testq $15, %rdi
  jz .Lstrlen_aligned_entry
  # Align down only within the current page; discard bytes preceding s.
  movq %rdi, %rcx
  andq $15, %rcx
  andq $-16, %rdi
  movdqa (%rdi), %xmm1
  pcmpeqb %xmm0, %xmm1
  pmovmskb %xmm1, %eax
  shrl %cl, %eax
  testl %eax, %eax
  jnz .Lstrlen_initial_found
  addq $16, %rdi
  jmp .Lstrlen_aligned_entry
.Lstrlen_initial_found:
  movq %rsi, %rdi
  jmp .Lstrlen_found_mask
.Lstrlen_aligned_entry:
  testq $63, %rdi
  jz .Lstrlen_vector64
  jmp .Lstrlen_vector
.Lstrlen_vector:
  movdqa (%rdi), %xmm1
  pcmpeqb %xmm0, %xmm1
  pmovmskb %xmm1, %eax
  testl %eax, %eax
  jnz .Lstrlen_found_mask
  addq $16, %rdi
  testq $63, %rdi
  jnz .Lstrlen_vector
.Lstrlen_vector64:
  movdqa 0(%rdi), %xmm1
  pcmpeqb %xmm0, %xmm1
  movdqa 16(%rdi), %xmm2
  pcmpeqb %xmm0, %xmm2
  movdqa 32(%rdi), %xmm3
  pcmpeqb %xmm0, %xmm3
  movdqa 48(%rdi), %xmm4
  pcmpeqb %xmm0, %xmm4
  movdqa %xmm1, %xmm5
  por %xmm2, %xmm5
  por %xmm3, %xmm5
  por %xmm4, %xmm5
  pmovmskb %xmm5, %eax
  testl %eax, %eax
  jnz .Lstrlen_found64
  addq $64, %rdi
  jmp .Lstrlen_after64
.Lstrlen_after64:
  movdqa (%rdi), %xmm1
  pcmpeqb %xmm0, %xmm1
  pmovmskb %xmm1, %eax
  testl %eax, %eax
  jnz .Lstrlen_found_mask
  addq $16, %rdi
  testq $127, %rdi
  jnz .Lstrlen_after64
.Lstrlen_vector128:
  movdqa 0(%rdi), %xmm1
  pcmpeqb %xmm0, %xmm1
  movdqa 16(%rdi), %xmm2
  pcmpeqb %xmm0, %xmm2
  movdqa 32(%rdi), %xmm3
  pcmpeqb %xmm0, %xmm3
  movdqa 48(%rdi), %xmm4
  pcmpeqb %xmm0, %xmm4
  movdqa %xmm1, %xmm5
  por %xmm2, %xmm5
  por %xmm3, %xmm5
  por %xmm4, %xmm5
  movdqa 64(%rdi), %xmm1
  pcmpeqb %xmm0, %xmm1
  por %xmm1, %xmm5
  movdqa 80(%rdi), %xmm2
  pcmpeqb %xmm0, %xmm2
  por %xmm2, %xmm5
  movdqa 96(%rdi), %xmm3
  pcmpeqb %xmm0, %xmm3
  por %xmm3, %xmm5
  movdqa 112(%rdi), %xmm4
  pcmpeqb %xmm0, %xmm4
  por %xmm4, %xmm5
  pmovmskb %xmm5, %eax
  testl %eax, %eax
  jnz .Lstrlen_found128
  addq $128, %rdi
  testq $255, %rdi
  jz .Lstrlen_vector256
  jmp .Lstrlen_after128
.Lstrlen_after128:
  movdqa (%rdi), %xmm1
  pcmpeqb %xmm0, %xmm1
  pmovmskb %xmm1, %eax
  testl %eax, %eax
  jnz .Lstrlen_found_mask
  addq $16, %rdi
  testq $255, %rdi
  jz .Lstrlen_vector256
  jmp .Lstrlen_after128
.Lstrlen_found128:
  movdqa 0(%rdi), %xmm1
  pcmpeqb %xmm0, %xmm1
  pmovmskb %xmm1, %eax
  testl %eax, %eax
  jnz .Lstrlen_found_mask
  addq $16, %rdi
  movdqa 0(%rdi), %xmm1
  pcmpeqb %xmm0, %xmm1
  pmovmskb %xmm1, %eax
  testl %eax, %eax
  jnz .Lstrlen_found_mask
  addq $16, %rdi
  movdqa 0(%rdi), %xmm1
  pcmpeqb %xmm0, %xmm1
  pmovmskb %xmm1, %eax
  testl %eax, %eax
  jnz .Lstrlen_found_mask
  addq $16, %rdi
  movdqa 0(%rdi), %xmm1
  pcmpeqb %xmm0, %xmm1
  pmovmskb %xmm1, %eax
  testl %eax, %eax
  jnz .Lstrlen_found_mask
  addq $16, %rdi
  movdqa 0(%rdi), %xmm1
  pcmpeqb %xmm0, %xmm1
  pmovmskb %xmm1, %eax
  testl %eax, %eax
  jnz .Lstrlen_found_mask
  addq $16, %rdi
  movdqa 0(%rdi), %xmm1
  pcmpeqb %xmm0, %xmm1
  pmovmskb %xmm1, %eax
  testl %eax, %eax
  jnz .Lstrlen_found_mask
  addq $16, %rdi
  movdqa 0(%rdi), %xmm1
  pcmpeqb %xmm0, %xmm1
  pmovmskb %xmm1, %eax
  testl %eax, %eax
  jnz .Lstrlen_found_mask
  addq $16, %rdi
  movdqa 0(%rdi), %xmm1
  pcmpeqb %xmm0, %xmm1
  pmovmskb %xmm1, %eax
  jmp .Lstrlen_found_mask
.Lstrlen_vector256:
  movdqa 0(%rdi), %xmm1
  pcmpeqb %xmm0, %xmm1
  movdqa 16(%rdi), %xmm2
  pcmpeqb %xmm0, %xmm2
  movdqa 32(%rdi), %xmm3
  pcmpeqb %xmm0, %xmm3
  movdqa 48(%rdi), %xmm4
  pcmpeqb %xmm0, %xmm4
  movdqa 64(%rdi), %xmm5
  pcmpeqb %xmm0, %xmm5
  movdqa 80(%rdi), %xmm6
  pcmpeqb %xmm0, %xmm6
  movdqa 96(%rdi), %xmm7
  pcmpeqb %xmm0, %xmm7
  movdqa 112(%rdi), %xmm8
  pcmpeqb %xmm0, %xmm8
  movdqa %xmm1, %xmm9
  por %xmm2, %xmm9
  movdqa %xmm3, %xmm10
  por %xmm4, %xmm10
  por %xmm10, %xmm9
  movdqa %xmm5, %xmm10
  por %xmm6, %xmm10
  movdqa %xmm7, %xmm11
  por %xmm8, %xmm11
  por %xmm11, %xmm10
  por %xmm10, %xmm9
  movdqa 128(%rdi), %xmm1
  pcmpeqb %xmm0, %xmm1
  movdqa 144(%rdi), %xmm2
  pcmpeqb %xmm0, %xmm2
  movdqa 160(%rdi), %xmm3
  pcmpeqb %xmm0, %xmm3
  movdqa 176(%rdi), %xmm4
  pcmpeqb %xmm0, %xmm4
  movdqa 192(%rdi), %xmm5
  pcmpeqb %xmm0, %xmm5
  movdqa 208(%rdi), %xmm6
  pcmpeqb %xmm0, %xmm6
  movdqa 224(%rdi), %xmm7
  pcmpeqb %xmm0, %xmm7
  movdqa 240(%rdi), %xmm8
  pcmpeqb %xmm0, %xmm8
  movdqa %xmm1, %xmm10
  por %xmm2, %xmm10
  movdqa %xmm3, %xmm11
  por %xmm4, %xmm11
  por %xmm11, %xmm10
  movdqa %xmm5, %xmm11
  por %xmm6, %xmm11
  movdqa %xmm7, %xmm12
  por %xmm8, %xmm12
  por %xmm12, %xmm11
  por %xmm11, %xmm10
  por %xmm10, %xmm9
  pmovmskb %xmm9, %eax
  testl %eax, %eax
  jnz .Lstrlen_found256
  addq $256, %rdi
  jmp .Lstrlen_vector256
.Lstrlen_found256:
  movl $16, %ecx
.Lstrlen_found256_scan:
  movdqa (%rdi), %xmm1
  pcmpeqb %xmm0, %xmm1
  pmovmskb %xmm1, %eax
  testl %eax, %eax
  jnz .Lstrlen_found_mask
  addq $16, %rdi
  decl %ecx
  jnz .Lstrlen_found256_scan
  ud2
.Lstrlen_found64:
  pmovmskb %xmm1, %eax
  testl %eax, %eax
  jnz .Lstrlen_found_mask
  addq $16, %rdi
  pmovmskb %xmm2, %eax
  testl %eax, %eax
  jnz .Lstrlen_found_mask
  addq $16, %rdi
  pmovmskb %xmm3, %eax
  testl %eax, %eax
  jnz .Lstrlen_found_mask
  addq $16, %rdi
  pmovmskb %xmm4, %eax
  jmp .Lstrlen_found_mask
.Lstrlen_found_mask:
  bsfl %eax, %eax
  addq %rax, %rdi
.Lstrlen_found:
  movq %rdi, %rax
  subq %rsi, %rax
  ret
.size nelibc_x86_64_sse_strlen, .-nelibc_x86_64_sse_strlen

#ifndef NELIBC_DISABLE_AVX2
.section .bss,"aw",@nobits
.p2align 2
.Lmemscan_avx2_state:
  .byte 0
.text
.type nelibc_x86_64_memscan_has_avx2,@function
nelibc_x86_64_memscan_has_avx2:
  cmpb $0, .Lmemscan_avx2_state(%rip)
  je .Lmemscan_detect_avx2
  movzbl .Lmemscan_avx2_state(%rip), %eax
  decl %eax
  ret
.Lmemscan_detect_avx2:
  pushq %rbx
  xorl %eax, %eax
  cpuid
  cmpl $7, %eax
  jb .Lmemscan_avx2_unavailable
  movl $1, %eax
  xorl %ecx, %ecx
  cpuid
  btl $27, %ecx
  jnc .Lmemscan_avx2_unavailable
  btl $28, %ecx
  jnc .Lmemscan_avx2_unavailable
  xorl %ecx, %ecx
  xgetbv
  movl %eax, %r8d
  andl $6, %eax
  cmpl $6, %eax
  jne .Lmemscan_avx2_unavailable
  movl $7, %eax
  xorl %ecx, %ecx
  cpuid
  btl $5, %ebx
  jnc .Lmemscan_avx2_unavailable
  btl $16, %ebx
  jnc .Lmemscan_avx2_available
  btl $30, %ebx
  jnc .Lmemscan_avx2_available
  andl $0xe6, %r8d
  cmpl $0xe6, %r8d
  jne .Lmemscan_avx2_available
  movb $3, .Lmemscan_avx2_state(%rip)
  movl $2, %eax
  popq %rbx
  ret
.Lmemscan_avx2_available:
  movb $2, .Lmemscan_avx2_state(%rip)
  movl $1, %eax
  popq %rbx
  ret
.Lmemscan_avx2_unavailable:
  movb $1, .Lmemscan_avx2_state(%rip)
  xorl %eax, %eax
  popq %rbx
  ret
.size nelibc_x86_64_memscan_has_avx2, .-nelibc_x86_64_memscan_has_avx2
#endif

.globl nelibc_tcc_sse_memchr
.type nelibc_tcc_sse_memchr,@function
.globl nelibc_x86_64_sse_memchr
.type nelibc_x86_64_sse_memchr,@function
nelibc_tcc_sse_memchr:
nelibc_x86_64_sse_memchr:
#ifndef NELIBC_DISABLE_AVX2
  cmpq $NELIBC_MEMSCAN_AVX2_THRESHOLD, %rdx
  jb .Lmemchr_sse_entry
  cmpb $3, .Lmemscan_avx2_state(%rip)
  je .Lmemchr_avx512_check
  cmpb $2, .Lmemscan_avx2_state(%rip)
  je .Lmemchr_avx2_entry
  cmpb $1, .Lmemscan_avx2_state(%rip)
  je .Lmemchr_sse_entry
  pushq %rdx
  call nelibc_x86_64_memscan_has_avx2
  popq %rdx
  cmpl $2, %eax
  je .Lmemchr_avx512_check
  testl %eax, %eax
  jnz .Lmemchr_avx2_entry
.Lmemchr_sse_entry:
#endif
  movzbl %sil, %eax
  movd %eax, %xmm0
  punpcklbw %xmm0, %xmm0
  pshuflw $0, %xmm0, %xmm0
  pshufd $0, %xmm0, %xmm0
.Lmemchr_vector:
.Lmemchr_vector64:
  cmpq $64, %rdx
  jb .Lmemchr_vector16
  jne .Lmemchr_vector64_parallel
  # 64-byte calls are common. Check each SSE2 vector in order so early and
  # absent matches avoid the parallel four-vector combine and decode path.
  movdqu 0(%rdi), %xmm1
  pcmpeqb %xmm0, %xmm1
  pmovmskb %xmm1, %eax
  testl %eax, %eax
  jnz .Lmemchr_found
  movdqu 16(%rdi), %xmm1
  pcmpeqb %xmm0, %xmm1
  pmovmskb %xmm1, %eax
  testl %eax, %eax
  jnz .Lmemchr_found16short
  movdqu 32(%rdi), %xmm1
  pcmpeqb %xmm0, %xmm1
  pmovmskb %xmm1, %eax
  testl %eax, %eax
  jnz .Lmemchr_found32short
  movdqu 48(%rdi), %xmm1
  pcmpeqb %xmm0, %xmm1
  pmovmskb %xmm1, %eax
  testl %eax, %eax
  jnz .Lmemchr_found48short
  xorl %eax, %eax
  ret
.Lmemchr_found16short:
  addq $16, %rdi
  jmp .Lmemchr_found
.Lmemchr_found32short:
  addq $32, %rdi
  jmp .Lmemchr_found
.Lmemchr_found48short:
  addq $48, %rdi
  jmp .Lmemchr_found
.Lmemchr_vector64_parallel:
  movdqu 0(%rdi), %xmm1
  pcmpeqb %xmm0, %xmm1
  movdqu 16(%rdi), %xmm2
  pcmpeqb %xmm0, %xmm2
  movdqu 32(%rdi), %xmm3
  pcmpeqb %xmm0, %xmm3
  movdqu 48(%rdi), %xmm4
  pcmpeqb %xmm0, %xmm4
  movdqa %xmm1, %xmm5
  por %xmm2, %xmm5
  por %xmm3, %xmm5
  por %xmm4, %xmm5
  pmovmskb %xmm5, %eax
  testl %eax, %eax
  jnz .Lmemchr_candidates
  addq $64, %rdi
  subq $64, %rdx
  jmp .Lmemchr_vector
.Lmemchr_candidates:
  pmovmskb %xmm1, %eax
  testl %eax, %eax
  jnz .Lmemchr_found
  addq $16, %rdi
  pmovmskb %xmm2, %eax
  testl %eax, %eax
  jnz .Lmemchr_found
  addq $16, %rdi
  pmovmskb %xmm3, %eax
  testl %eax, %eax
  jnz .Lmemchr_found
  addq $16, %rdi
  pmovmskb %xmm4, %eax
  jmp .Lmemchr_found
.Lmemchr_vector16:
  cmpq $16, %rdx
  jb .Lmemchr_tail
  movdqu (%rdi), %xmm1
  pcmpeqb %xmm0, %xmm1
  pmovmskb %xmm1, %eax
  testl %eax, %eax
  jnz .Lmemchr_found
  addq $16, %rdi
  subq $16, %rdx
  jmp .Lmemchr_vector
.Lmemchr_tail:
  testq %rdx, %rdx
  jz .Lmemchr_not_found
.Lmemchr_tail_loop:
  cmpb %sil, (%rdi)
  je .Lmemchr_found_scalar
  incq %rdi
  decq %rdx
  jnz .Lmemchr_tail_loop
.Lmemchr_not_found:
  xorl %eax, %eax
  ret
#ifndef NELIBC_DISABLE_AVX2
.Lmemchr_avx512_check:
  cmpq $NELIBC_MEMSCAN_AVX512_THRESHOLD, %rdx
  jae .Lmemchr_avx512_entry
  jmp .Lmemchr_avx2_entry
.Lmemchr_avx2_entry:
  vmovd %esi, %xmm0
  vpbroadcastb %xmm0, %ymm0
  xorl %r8d, %r8d
#if NELIBC_MEMSCAN_AVX2_BLOCK_SIZE == 256
.Lmemchr_avx2_loop256:
  cmpq $256, %rdx
  jb .Lmemchr_avx2_loop128
  vmovdqu 0(%rdi,%r8), %ymm1
  vpcmpeqb %ymm0, %ymm1, %ymm1
  vmovdqu 32(%rdi,%r8), %ymm2
  vpcmpeqb %ymm0, %ymm2, %ymm2
  vmovdqu 64(%rdi,%r8), %ymm3
  vpcmpeqb %ymm0, %ymm3, %ymm3
  vmovdqu 96(%rdi,%r8), %ymm4
  vpcmpeqb %ymm0, %ymm4, %ymm4
  vmovdqu 128(%rdi,%r8), %ymm5
  vpcmpeqb %ymm0, %ymm5, %ymm5
  vmovdqu 160(%rdi,%r8), %ymm6
  vpcmpeqb %ymm0, %ymm6, %ymm6
  vmovdqu 192(%rdi,%r8), %ymm7
  vpcmpeqb %ymm0, %ymm7, %ymm7
  vmovdqu 224(%rdi,%r8), %ymm8
  vpcmpeqb %ymm0, %ymm8, %ymm8
  vpor %ymm2, %ymm1, %ymm9
  vpor %ymm4, %ymm3, %ymm10
  vpor %ymm6, %ymm5, %ymm11
  vpor %ymm8, %ymm7, %ymm12
  vpor %ymm10, %ymm9, %ymm9
  vpor %ymm12, %ymm11, %ymm11
  vpor %ymm11, %ymm9, %ymm9
  vpmovmskb %ymm9, %eax
  testl %eax, %eax
  jnz .Lmemchr_avx2_found256
  addq $256, %r8
  subq $256, %rdx
  jmp .Lmemchr_avx2_loop256
.Lmemchr_avx2_found256:
  vpmovmskb %ymm1, %eax
  testl %eax, %eax
  jnz .Lmemchr_avx2_index256
  addq $32, %r8
  vpmovmskb %ymm2, %eax
  testl %eax, %eax
  jnz .Lmemchr_avx2_index256
  addq $32, %r8
  vpmovmskb %ymm3, %eax
  testl %eax, %eax
  jnz .Lmemchr_avx2_index256
  addq $32, %r8
  vpmovmskb %ymm4, %eax
  testl %eax, %eax
  jnz .Lmemchr_avx2_index256
  addq $32, %r8
  vpmovmskb %ymm5, %eax
  testl %eax, %eax
  jnz .Lmemchr_avx2_index256
  addq $32, %r8
  vpmovmskb %ymm6, %eax
  testl %eax, %eax
  jnz .Lmemchr_avx2_index256
  addq $32, %r8
  vpmovmskb %ymm7, %eax
  testl %eax, %eax
  jnz .Lmemchr_avx2_index256
  addq $32, %r8
  vpmovmskb %ymm8, %eax
.Lmemchr_avx2_index256:
  bsfl %eax, %eax
  addq %r8, %rax
  vzeroupper
  addq %rdi, %rax
  ret
.Lmemchr_avx2_loop128:
#endif
.Lmemchr_avx2_loop:
  cmpq $128, %rdx
  jb .Lmemchr_avx2_tail
  vmovdqu 0(%rdi,%r8), %ymm1
  vpcmpeqb %ymm0, %ymm1, %ymm1
  vmovdqu 32(%rdi,%r8), %ymm2
  vpcmpeqb %ymm0, %ymm2, %ymm2
  vmovdqu 64(%rdi,%r8), %ymm3
  vpcmpeqb %ymm0, %ymm3, %ymm3
  vmovdqu 96(%rdi,%r8), %ymm4
  vpcmpeqb %ymm0, %ymm4, %ymm4
  vpor %ymm2, %ymm1, %ymm5
  vpor %ymm4, %ymm3, %ymm6
  vpor %ymm6, %ymm5, %ymm5
  vpmovmskb %ymm5, %eax
  testl %eax, %eax
  jnz .Lmemchr_avx2_found
  addq $128, %r8
  subq $128, %rdx
  jmp .Lmemchr_avx2_loop
.Lmemchr_avx2_found:
  vpmovmskb %ymm1, %eax
  testl %eax, %eax
  jnz .Lmemchr_avx2_index
  addq $32, %r8
  vpmovmskb %ymm2, %eax
  testl %eax, %eax
  jnz .Lmemchr_avx2_index
  addq $32, %r8
  vpmovmskb %ymm3, %eax
  testl %eax, %eax
  jnz .Lmemchr_avx2_index
  addq $32, %r8
  vpmovmskb %ymm4, %eax
.Lmemchr_avx2_index:
  bsfl %eax, %eax
  addq %r8, %rax
  vzeroupper
  addq %rdi, %rax
  ret
.Lmemchr_avx2_tail:
  vzeroupper
  addq %r8, %rdi
  jmp .Lmemchr_sse_entry
.Lmemchr_avx512_entry:
  movq %rdi, %r8
  negq %r8
  andq $63, %r8
  jnz .Lmemchr_avx512_align_loop
  cmpb %sil, (%rdi)
  je .Lmemchr_found_scalar
.Lmemchr_avx512_align_loop:
  testq %r8, %r8
  jz .Lmemchr_avx512_aligned
.Lmemchr_avx512_align_scan:
  cmpb %sil, (%rdi)
  je .Lmemchr_found_scalar
  incq %rdi
  decq %rdx
  decq %r8
  jnz .Lmemchr_avx512_align_scan
.Lmemchr_avx512_aligned:
  vmovd %esi, %xmm0
  vpbroadcastb %xmm0, %zmm0
  vpcmpeqb 0(%rdi), %zmm0, %k1
  kortestq %k1, %k1
  jnz .Lmemchr_avx512_found_edge
  addq $64, %rdi
  subq $64, %rdx
  cmpq $256, %rdx
  jb .Lmemchr_avx512_tail
.Lmemchr_avx512_loop:
  vpcmpeqb 0(%rdi), %zmm0, %k1
  vpcmpeqb 64(%rdi), %zmm0, %k2
  vpcmpeqb 128(%rdi), %zmm0, %k3
  vpcmpeqb 192(%rdi), %zmm0, %k4
  korq %k2, %k1, %k5
  korq %k4, %k3, %k6
  korq %k6, %k5, %k5
  kortestq %k5, %k5
  jnz .Lmemchr_avx512_found
  addq $256, %rdi
  subq $256, %rdx
  cmpq $256, %rdx
  jae .Lmemchr_avx512_loop
.Lmemchr_avx512_tail:
  xorl %r8d, %r8d
  jmp .Lmemchr_avx2_loop
.Lmemchr_avx512_found:
  kortestq %k1, %k1
  jnz .Lmemchr_avx512_found0
  kortestq %k2, %k2
  jnz .Lmemchr_avx512_found1
  kortestq %k3, %k3
  jnz .Lmemchr_avx512_found2
  kmovq %k4, %rax
  bsfq %rax, %rax
  addq $192, %rdi
  addq %rdi, %rax
  vzeroupper
  ret
.Lmemchr_avx512_found_edge:
  kmovq %k1, %rax
  bsfq %rax, %rax
  addq %rdi, %rax
  vzeroupper
  ret
.Lmemchr_avx512_found0:
  kmovq %k1, %rax
  jmp .Lmemchr_avx512_found_index
.Lmemchr_avx512_found1:
  kmovq %k2, %rax
  addq $64, %rdi
  jmp .Lmemchr_avx512_found_index
.Lmemchr_avx512_found2:
  kmovq %k3, %rax
  addq $128, %rdi
.Lmemchr_avx512_found_index:
  bsfq %rax, %rax
  addq %rdi, %rax
  vzeroupper
  ret
#endif
.Lmemchr_found:
  bsfl %eax, %eax
  addq %rax, %rdi
.Lmemchr_found_scalar:
  movq %rdi, %rax
  ret
.size nelibc_tcc_sse_memchr, .-nelibc_tcc_sse_memchr
.size nelibc_x86_64_sse_memchr, .-nelibc_x86_64_sse_memchr

.globl nelibc_tcc_sse_memrchr
.type nelibc_tcc_sse_memrchr,@function
.globl nelibc_x86_64_sse_memrchr
.type nelibc_x86_64_sse_memrchr,@function
.macro NELIBC_MEMRCHR_LOAD offset, reg
  movdqu \offset(%rdi), \reg
  pcmpeqb %xmm0, \reg
.endm
nelibc_tcc_sse_memrchr:
nelibc_x86_64_sse_memrchr:
#ifndef NELIBC_DISABLE_AVX2
  cmpq $NELIBC_MEMSCAN_AVX2_THRESHOLD, %rdx
  jb .Lmemrchr_sse_entry
  cmpb $3, .Lmemscan_avx2_state(%rip)
  je .Lmemrchr_avx512_check
  cmpb $2, .Lmemscan_avx2_state(%rip)
  je .Lmemrchr_avx2_entry
  cmpb $1, .Lmemscan_avx2_state(%rip)
  je .Lmemrchr_sse_entry
  pushq %rdx
  call nelibc_x86_64_memscan_has_avx2
  popq %rdx
  cmpl $2, %eax
  je .Lmemrchr_avx512_check
  testl %eax, %eax
  jnz .Lmemrchr_avx2_entry
.Lmemrchr_sse_entry:
#endif
  movzbl %sil, %eax
  movd %eax, %xmm0
  punpcklbw %xmm0, %xmm0
  pshuflw $0, %xmm0, %xmm0
  pshufd $0, %xmm0, %xmm0
  leaq (%rdi,%rdx), %rdi
.Lmemrchr_vector:
  cmpq $64, %rdx
  jb .Lmemrchr_vector16
  subq $64, %rdi
  subq $64, %rdx
  NELIBC_MEMRCHR_LOAD 48, %xmm1
  NELIBC_MEMRCHR_LOAD 32, %xmm2
  NELIBC_MEMRCHR_LOAD 16, %xmm3
  NELIBC_MEMRCHR_LOAD 0, %xmm4
  movdqa %xmm1, %xmm5
  por %xmm2, %xmm5
  por %xmm3, %xmm5
  por %xmm4, %xmm5
  pmovmskb %xmm5, %eax
  testl %eax, %eax
  jnz .Lmemrchr_candidates
  jmp .Lmemrchr_vector
.Lmemrchr_candidates:
  pmovmskb %xmm1, %eax
  testl %eax, %eax
  jnz .Lmemrchr_found48
  pmovmskb %xmm2, %eax
  testl %eax, %eax
  jnz .Lmemrchr_found32
  pmovmskb %xmm3, %eax
  testl %eax, %eax
  jnz .Lmemrchr_found16
  pmovmskb %xmm4, %eax
  jmp .Lmemrchr_found
.Lmemrchr_found48:
  addq $48, %rdi
  jmp .Lmemrchr_found
.Lmemrchr_found32:
  addq $32, %rdi
  jmp .Lmemrchr_found
.Lmemrchr_found16:
  addq $16, %rdi
  jmp .Lmemrchr_found
.Lmemrchr_vector16:
  cmpq $16, %rdx
  jb .Lmemrchr_tail
  subq $16, %rdi
  subq $16, %rdx
  movdqu (%rdi), %xmm1
  pcmpeqb %xmm0, %xmm1
  pmovmskb %xmm1, %eax
  testl %eax, %eax
  jnz .Lmemrchr_found
  jmp .Lmemrchr_vector
.Lmemrchr_tail:
  testq %rdx, %rdx
  jz .Lmemrchr_not_found
.Lmemrchr_tail_loop:
  decq %rdi
  cmpb %sil, (%rdi)
  je .Lmemrchr_found_scalar
  decq %rdx
  jnz .Lmemrchr_tail_loop
.Lmemrchr_not_found:
  xorl %eax, %eax
  ret
#ifndef NELIBC_DISABLE_AVX2
.Lmemrchr_avx512_check:
  cmpq $NELIBC_MEMSCAN_AVX512_THRESHOLD, %rdx
  jae .Lmemrchr_avx512_entry
  jmp .Lmemrchr_avx2_entry
.Lmemrchr_avx2_entry:
  vmovd %esi, %xmm0
  vpbroadcastb %xmm0, %ymm0
  leaq (%rdi,%rdx), %rdi
#if NELIBC_MEMSCAN_AVX2_BLOCK_SIZE == 256
.Lmemrchr_avx2_loop256:
  cmpq $256, %rdx
  jb .Lmemrchr_avx2_loop128
  subq $256, %rdi
  subq $256, %rdx
  vmovdqu 224(%rdi), %ymm1
  vpcmpeqb %ymm0, %ymm1, %ymm1
  vmovdqu 192(%rdi), %ymm2
  vpcmpeqb %ymm0, %ymm2, %ymm2
  vmovdqu 160(%rdi), %ymm3
  vpcmpeqb %ymm0, %ymm3, %ymm3
  vmovdqu 128(%rdi), %ymm4
  vpcmpeqb %ymm0, %ymm4, %ymm4
  vmovdqu 96(%rdi), %ymm5
  vpcmpeqb %ymm0, %ymm5, %ymm5
  vmovdqu 64(%rdi), %ymm6
  vpcmpeqb %ymm0, %ymm6, %ymm6
  vmovdqu 32(%rdi), %ymm7
  vpcmpeqb %ymm0, %ymm7, %ymm7
  vmovdqu 0(%rdi), %ymm8
  vpcmpeqb %ymm0, %ymm8, %ymm8
  vpor %ymm2, %ymm1, %ymm9
  vpor %ymm4, %ymm3, %ymm10
  vpor %ymm6, %ymm5, %ymm11
  vpor %ymm8, %ymm7, %ymm12
  vpor %ymm10, %ymm9, %ymm9
  vpor %ymm12, %ymm11, %ymm11
  vpor %ymm11, %ymm9, %ymm9
  vpmovmskb %ymm9, %eax
  testl %eax, %eax
  jnz .Lmemrchr_avx2_found256
  jmp .Lmemrchr_avx2_loop256
.Lmemrchr_avx2_found256:
  vpmovmskb %ymm1, %eax
  testl %eax, %eax
  jnz .Lmemrchr_avx2_256_224
  vpmovmskb %ymm2, %eax
  testl %eax, %eax
  jnz .Lmemrchr_avx2_256_192
  vpmovmskb %ymm3, %eax
  testl %eax, %eax
  jnz .Lmemrchr_avx2_256_160
  vpmovmskb %ymm4, %eax
  testl %eax, %eax
  jnz .Lmemrchr_avx2_256_128
  vpmovmskb %ymm5, %eax
  testl %eax, %eax
  jnz .Lmemrchr_avx2_256_96
  vpmovmskb %ymm6, %eax
  testl %eax, %eax
  jnz .Lmemrchr_avx2_256_64
  vpmovmskb %ymm7, %eax
  testl %eax, %eax
  jnz .Lmemrchr_avx2_256_32
  vpmovmskb %ymm8, %eax
  leaq 0(%rdi), %r8
  jmp .Lmemrchr_avx2_index256
.Lmemrchr_avx2_256_224:
  leaq 224(%rdi), %r8
  jmp .Lmemrchr_avx2_index256
.Lmemrchr_avx2_256_192:
  leaq 192(%rdi), %r8
  jmp .Lmemrchr_avx2_index256
.Lmemrchr_avx2_256_160:
  leaq 160(%rdi), %r8
  jmp .Lmemrchr_avx2_index256
.Lmemrchr_avx2_256_128:
  leaq 128(%rdi), %r8
  jmp .Lmemrchr_avx2_index256
.Lmemrchr_avx2_256_96:
  leaq 96(%rdi), %r8
  jmp .Lmemrchr_avx2_index256
.Lmemrchr_avx2_256_64:
  leaq 64(%rdi), %r8
  jmp .Lmemrchr_avx2_index256
.Lmemrchr_avx2_256_32:
  leaq 32(%rdi), %r8
.Lmemrchr_avx2_index256:
  bsrl %eax, %eax
  addq %r8, %rax
  vzeroupper
  ret
.Lmemrchr_avx2_loop128:
#endif
.Lmemrchr_avx2_loop:
  cmpq $128, %rdx
  jb .Lmemrchr_avx2_tail
  subq $128, %rdi
  subq $128, %rdx
  vmovdqu 96(%rdi), %ymm1
  vpcmpeqb %ymm0, %ymm1, %ymm1
  vmovdqu 64(%rdi), %ymm2
  vpcmpeqb %ymm0, %ymm2, %ymm2
  vmovdqu 32(%rdi), %ymm3
  vpcmpeqb %ymm0, %ymm3, %ymm3
  vmovdqu 0(%rdi), %ymm4
  vpcmpeqb %ymm0, %ymm4, %ymm4
  vpor %ymm2, %ymm1, %ymm5
  vpor %ymm4, %ymm3, %ymm6
  vpor %ymm6, %ymm5, %ymm5
  vpmovmskb %ymm5, %eax
  testl %eax, %eax
  jnz .Lmemrchr_avx2_found
  jmp .Lmemrchr_avx2_loop
.Lmemrchr_avx2_found:
  vpmovmskb %ymm1, %eax
  testl %eax, %eax
  jnz .Lmemrchr_avx2_96
  vpmovmskb %ymm2, %eax
  testl %eax, %eax
  jnz .Lmemrchr_avx2_64
  vpmovmskb %ymm3, %eax
  testl %eax, %eax
  jnz .Lmemrchr_avx2_32
  vpmovmskb %ymm4, %eax
  leaq 0(%rdi), %r8
  jmp .Lmemrchr_avx2_index
.Lmemrchr_avx2_96:
  leaq 96(%rdi), %r8
  jmp .Lmemrchr_avx2_index
.Lmemrchr_avx2_64:
  leaq 64(%rdi), %r8
  jmp .Lmemrchr_avx2_index
.Lmemrchr_avx2_32:
  leaq 32(%rdi), %r8
.Lmemrchr_avx2_index:
  bsrl %eax, %eax
  addq %r8, %rax
  vzeroupper
  ret
.Lmemrchr_avx2_tail:
  vzeroupper
  subq %rdx, %rdi
  movzbl %sil, %eax
  movd %eax, %xmm0
  punpcklbw %xmm0, %xmm0
  pshuflw $0, %xmm0, %xmm0
  pshufd $0, %xmm0, %xmm0
  leaq (%rdi,%rdx), %rdi
  jmp .Lmemrchr_vector
.Lmemrchr_avx512_entry:
  leaq (%rdi,%rdx), %rdi
  movq %rdi, %r8
  andq $63, %r8
  jnz .Lmemrchr_avx512_align_loop
  cmpb %sil, -1(%rdi)
  je .Lmemrchr_avx512_found_last
.Lmemrchr_avx512_align_loop:
  testq %r8, %r8
  jz .Lmemrchr_avx512_aligned
.Lmemrchr_avx512_align_scan:
  decq %rdi
  decq %rdx
  cmpb %sil, (%rdi)
  je .Lmemrchr_found_scalar
  decq %r8
  jnz .Lmemrchr_avx512_align_scan
.Lmemrchr_avx512_aligned:
  vmovd %esi, %xmm0
  vpbroadcastb %xmm0, %zmm0
  subq $64, %rdi
  subq $64, %rdx
  vpcmpeqb 0(%rdi), %zmm0, %k1
  kortestq %k1, %k1
  jnz .Lmemrchr_avx512_found_edge
  cmpq $256, %rdx
  jb .Lmemrchr_avx512_tail
.Lmemrchr_avx512_loop:
  subq $256, %rdi
  subq $256, %rdx
  vpcmpeqb 192(%rdi), %zmm0, %k1
  vpcmpeqb 128(%rdi), %zmm0, %k2
  vpcmpeqb 64(%rdi), %zmm0, %k3
  vpcmpeqb 0(%rdi), %zmm0, %k4
  korq %k2, %k1, %k5
  korq %k4, %k3, %k6
  korq %k6, %k5, %k5
  kortestq %k5, %k5
  jnz .Lmemrchr_avx512_found
  cmpq $256, %rdx
  jae .Lmemrchr_avx512_loop
.Lmemrchr_avx512_tail:
  jmp .Lmemrchr_avx2_loop
.Lmemrchr_avx512_found:
  kortestq %k1, %k1
  jnz .Lmemrchr_avx512_found0
  kortestq %k2, %k2
  jnz .Lmemrchr_avx512_found1
  kortestq %k3, %k3
  jnz .Lmemrchr_avx512_found2
  kmovq %k4, %rax
  bsrq %rax, %rax
  addq %rdi, %rax
  vzeroupper
  ret
.Lmemrchr_avx512_found_last:
  decq %rdi
  jmp .Lmemrchr_found_scalar
.Lmemrchr_avx512_found_edge:
  kmovq %k1, %rax
  bsrq %rax, %rax
  addq %rdi, %rax
  vzeroupper
  ret
.Lmemrchr_avx512_found0:
  kmovq %k1, %rax
  bsrq %rax, %rax
  addq $192, %rax
  addq %rdi, %rax
  vzeroupper
  ret
.Lmemrchr_avx512_found1:
  kmovq %k2, %rax
  bsrq %rax, %rax
  addq $128, %rax
  addq %rdi, %rax
  vzeroupper
  ret
.Lmemrchr_avx512_found2:
  kmovq %k3, %rax
  bsrq %rax, %rax
  addq $64, %rax
  addq %rdi, %rax
  vzeroupper
  ret
#endif
.Lmemrchr_found:
  bsrl %eax, %eax
  addq %rax, %rdi
.Lmemrchr_found_scalar:
  movq %rdi, %rax
  ret
.size nelibc_tcc_sse_memrchr, .-nelibc_tcc_sse_memrchr
.size nelibc_x86_64_sse_memrchr, .-nelibc_x86_64_sse_memrchr

#ifndef NELIBC_DISABLE_AVX2
.section .bss,"aw",@nobits
.p2align 2
.Lmemcmp_avx2_state:
  .byte 0
.text
#endif

.globl nelibc_tcc_sse_memcmp
.type nelibc_tcc_sse_memcmp,@function
.globl nelibc_x86_64_sse_memcmp
.type nelibc_x86_64_sse_memcmp,@function
nelibc_tcc_sse_memcmp:
nelibc_x86_64_sse_memcmp:
  xorl %r8d, %r8d
#ifndef NELIBC_DISABLE_AVX2
  cmpq $4096, %rdx
  jb .Lmemcmp_vector128
  cmpb $0, .Lmemcmp_avx2_state(%rip)
  je .Lmemcmp_detect_avx2
  cmpb $2, .Lmemcmp_avx2_state(%rip)
  je .Lmemcmp_avx2_start
  jmp .Lmemcmp_vector128
.Lmemcmp_detect_avx2:
  pushq %rbx
  pushq %rdx
  xorl %eax, %eax
  cpuid
  cmpl $7, %eax
  jb .Lmemcmp_avx2_unavailable
  movl $1, %eax
  xorl %ecx, %ecx
  cpuid
  btl $27, %ecx
  jnc .Lmemcmp_avx2_unavailable
  btl $28, %ecx
  jnc .Lmemcmp_avx2_unavailable
  xorl %ecx, %ecx
  xgetbv
  andl $6, %eax
  cmpl $6, %eax
  jne .Lmemcmp_avx2_unavailable
  movl $7, %eax
  xorl %ecx, %ecx
  cpuid
  btl $5, %ebx
  jnc .Lmemcmp_avx2_unavailable
  movb $2, .Lmemcmp_avx2_state(%rip)
  popq %rdx
  popq %rbx
  jmp .Lmemcmp_avx2_start
.Lmemcmp_avx2_unavailable:
  movb $1, .Lmemcmp_avx2_state(%rip)
  popq %rdx
  popq %rbx
  jmp .Lmemcmp_vector128
#endif
.Lmemcmp_vector128:
  cmpq $128, %rdx
  jb .Lmemcmp_vector64
  # Resolve the first vector immediately; combine the next seven equality
  # masks and rescan that tail in order only when the combined mask differs.
  pcmpeqb %xmm0, %xmm0
  movdqu 0(%rdi,%r8), %xmm1
  movdqu 0(%rsi,%r8), %xmm2
  pcmpeqb %xmm2, %xmm1
  pmovmskb %xmm1, %eax
  cmpl $0xffff, %eax
  jne .Lmemcmp_difference
  pand %xmm1, %xmm0
  movdqu 16(%rdi,%r8), %xmm1
  movdqu 16(%rsi,%r8), %xmm2
  pcmpeqb %xmm2, %xmm1
  pand %xmm1, %xmm0
  movdqu 32(%rdi,%r8), %xmm1
  movdqu 32(%rsi,%r8), %xmm2
  pcmpeqb %xmm2, %xmm1
  pand %xmm1, %xmm0
  movdqu 48(%rdi,%r8), %xmm1
  movdqu 48(%rsi,%r8), %xmm2
  pcmpeqb %xmm2, %xmm1
  pand %xmm1, %xmm0
  movdqu 64(%rdi,%r8), %xmm1
  movdqu 64(%rsi,%r8), %xmm2
  pcmpeqb %xmm2, %xmm1
  pand %xmm1, %xmm0
  movdqu 80(%rdi,%r8), %xmm1
  movdqu 80(%rsi,%r8), %xmm2
  pcmpeqb %xmm2, %xmm1
  pand %xmm1, %xmm0
  movdqu 96(%rdi,%r8), %xmm1
  movdqu 96(%rsi,%r8), %xmm2
  pcmpeqb %xmm2, %xmm1
  pand %xmm1, %xmm0
  movdqu 112(%rdi,%r8), %xmm1
  movdqu 112(%rsi,%r8), %xmm2
  pcmpeqb %xmm2, %xmm1
  pand %xmm1, %xmm0
  pmovmskb %xmm0, %eax
  cmpl $0xffff, %eax
  jne .Lmemcmp_mismatch128
  addq $128, %r8
  subq $128, %rdx
  jmp .Lmemcmp_vector128
.Lmemcmp_mismatch128:
  addq $16, %r8
  movdqu 0(%rdi,%r8), %xmm1
  movdqu 0(%rsi,%r8), %xmm2
  pcmpeqb %xmm2, %xmm1
  pmovmskb %xmm1, %eax
  cmpl $0xffff, %eax
  jne .Lmemcmp_difference
  addq $16, %r8
  movdqu 0(%rdi,%r8), %xmm1
  movdqu 0(%rsi,%r8), %xmm2
  pcmpeqb %xmm2, %xmm1
  pmovmskb %xmm1, %eax
  cmpl $0xffff, %eax
  jne .Lmemcmp_difference
  addq $16, %r8
  movdqu 0(%rdi,%r8), %xmm1
  movdqu 0(%rsi,%r8), %xmm2
  pcmpeqb %xmm2, %xmm1
  pmovmskb %xmm1, %eax
  cmpl $0xffff, %eax
  jne .Lmemcmp_difference
  addq $16, %r8
  movdqu 0(%rdi,%r8), %xmm1
  movdqu 0(%rsi,%r8), %xmm2
  pcmpeqb %xmm2, %xmm1
  pmovmskb %xmm1, %eax
  cmpl $0xffff, %eax
  jne .Lmemcmp_difference
  addq $16, %r8
  movdqu 0(%rdi,%r8), %xmm1
  movdqu 0(%rsi,%r8), %xmm2
  pcmpeqb %xmm2, %xmm1
  pmovmskb %xmm1, %eax
  cmpl $0xffff, %eax
  jne .Lmemcmp_difference
  addq $16, %r8
  movdqu 0(%rdi,%r8), %xmm1
  movdqu 0(%rsi,%r8), %xmm2
  pcmpeqb %xmm2, %xmm1
  pmovmskb %xmm1, %eax
  cmpl $0xffff, %eax
  jne .Lmemcmp_difference
  addq $16, %r8
  movdqu 0(%rdi,%r8), %xmm1
  movdqu 0(%rsi,%r8), %xmm2
  pcmpeqb %xmm2, %xmm1
  pmovmskb %xmm1, %eax
  cmpl $0xffff, %eax
  jne .Lmemcmp_difference
  jmp .Lmemcmp_equal
.Lmemcmp_vector64:
  cmpq $64, %rdx
  jb .Lmemcmp_vector16
  movdqu 0(%rdi,%r8), %xmm1
  movdqu 0(%rsi,%r8), %xmm2
  pcmpeqb %xmm2, %xmm1
  pmovmskb %xmm1, %eax
  cmpl $0xffff, %eax
  jne .Lmemcmp_difference
  addq $16, %r8
  movdqu 0(%rdi,%r8), %xmm1
  movdqu 0(%rsi,%r8), %xmm2
  pcmpeqb %xmm2, %xmm1
  pmovmskb %xmm1, %eax
  cmpl $0xffff, %eax
  jne .Lmemcmp_difference
  addq $16, %r8
  movdqu 0(%rdi,%r8), %xmm1
  movdqu 0(%rsi,%r8), %xmm2
  pcmpeqb %xmm2, %xmm1
  pmovmskb %xmm1, %eax
  cmpl $0xffff, %eax
  jne .Lmemcmp_difference
  addq $16, %r8
  movdqu 0(%rdi,%r8), %xmm1
  movdqu 0(%rsi,%r8), %xmm2
  pcmpeqb %xmm2, %xmm1
  pmovmskb %xmm1, %eax
  cmpl $0xffff, %eax
  jne .Lmemcmp_difference
  addq $16, %r8
  subq $64, %rdx
  jmp .Lmemcmp_vector128
.Lmemcmp_vector16:
  cmpq $16, %rdx
  jb .Lmemcmp_tail
  movdqu (%rdi,%r8), %xmm1
  movdqu (%rsi,%r8), %xmm2
  pcmpeqb %xmm2, %xmm1
  pmovmskb %xmm1, %eax
  cmpl $0xffff, %eax
  jne .Lmemcmp_difference
  addq $16, %r8
  subq $16, %rdx
  jmp .Lmemcmp_vector64
.Lmemcmp_tail:
  testq %rdx, %rdx
  jz .Lmemcmp_equal
.Lmemcmp_tail_loop:
  movzbl (%rdi,%r8), %eax
  movzbl (%rsi,%r8), %ecx
  cmpl %ecx, %eax
  jne .Lmemcmp_scalar_difference
  incq %r8
  decq %rdx
  jnz .Lmemcmp_tail_loop
.Lmemcmp_equal:
  xorl %eax, %eax
  ret
.Lmemcmp_difference:
  notl %eax
  bsfl %eax, %eax
  addq %r8, %rax
  movzbl (%rdi,%rax), %ecx
  movzbl (%rsi,%rax), %edx
  subl %edx, %ecx
  movl %ecx, %eax
  ret
.Lmemcmp_scalar_difference:
  subl %ecx, %eax
  ret
#ifdef NELIBC_DISABLE_AVX2
.size nelibc_tcc_sse_memcmp, .-nelibc_tcc_sse_memcmp
.size nelibc_x86_64_sse_memcmp, .-nelibc_x86_64_sse_memcmp
#else
.Lmemcmp_avx2_start:
  cmpq $32768, %rdx
  jae .Lmemcmp_avx2_loop128
  jmp .Lmemcmp_avx2_loop256
.Lmemcmp_avx2_loop256:
  vmovdqu 0(%rdi,%r8), %ymm1
  vpcmpeqb 0(%rsi,%r8), %ymm1, %ymm1
  vpmovmskb %ymm1, %eax
  cmpl $-1, %eax
  jne .Lmemcmp_avx2_diff
  vmovdqu 32(%rdi,%r8), %ymm2
  vpcmpeqb 32(%rsi,%r8), %ymm2, %ymm2
  vmovdqu 64(%rdi,%r8), %ymm3
  vpcmpeqb 64(%rsi,%r8), %ymm3, %ymm3
  vmovdqu 96(%rdi,%r8), %ymm4
  vpcmpeqb 96(%rsi,%r8), %ymm4, %ymm4
  vmovdqu 128(%rdi,%r8), %ymm5
  vpcmpeqb 128(%rsi,%r8), %ymm5, %ymm5
  vmovdqu 160(%rdi,%r8), %ymm6
  vpcmpeqb 160(%rsi,%r8), %ymm6, %ymm6
  vmovdqu 192(%rdi,%r8), %ymm7
  vpcmpeqb 192(%rsi,%r8), %ymm7, %ymm7
  vmovdqu 224(%rdi,%r8), %ymm8
  vpcmpeqb 224(%rsi,%r8), %ymm8, %ymm8
  vpand %ymm3, %ymm2, %ymm2
  vpand %ymm4, %ymm2, %ymm2
  vpand %ymm5, %ymm2, %ymm2
  vpand %ymm6, %ymm2, %ymm2
  vpand %ymm7, %ymm2, %ymm2
  vpand %ymm8, %ymm2, %ymm2
  vpmovmskb %ymm2, %eax
  cmpl $-1, %eax
  jne .Lmemcmp_avx2_mismatch_tail256
  addq $256, %r8
  subq $256, %rdx
  cmpq $256, %rdx
  jae .Lmemcmp_avx2_loop256
  vzeroupper
  jmp .Lmemcmp_vector128
.Lmemcmp_avx2_loop128:
  vmovdqu 0(%rdi,%r8), %ymm1
  vpcmpeqb 0(%rsi,%r8), %ymm1, %ymm1
  vpmovmskb %ymm1, %eax
  cmpl $-1, %eax
  jne .Lmemcmp_avx2_diff
  vmovdqu 32(%rdi,%r8), %ymm2
  vpcmpeqb 32(%rsi,%r8), %ymm2, %ymm2
  vmovdqu 64(%rdi,%r8), %ymm3
  vpcmpeqb 64(%rsi,%r8), %ymm3, %ymm3
  vmovdqu 96(%rdi,%r8), %ymm4
  vpcmpeqb 96(%rsi,%r8), %ymm4, %ymm4
  vpand %ymm3, %ymm2, %ymm2
  vpand %ymm4, %ymm2, %ymm2
  vpmovmskb %ymm2, %eax
  cmpl $-1, %eax
  jne .Lmemcmp_avx2_mismatch_tail128
  addq $128, %r8
  subq $128, %rdx
  cmpq $128, %rdx
  jae .Lmemcmp_avx2_loop128
  vzeroupper
  jmp .Lmemcmp_vector128
.Lmemcmp_avx2_mismatch_tail128:
  addq $32, %r8
  vmovdqu (%rdi,%r8), %ymm1
  vpcmpeqb (%rsi,%r8), %ymm1, %ymm1
  vpmovmskb %ymm1, %eax
  cmpl $-1, %eax
  jne .Lmemcmp_avx2_diff
  addq $32, %r8
  vmovdqu (%rdi,%r8), %ymm1
  vpcmpeqb (%rsi,%r8), %ymm1, %ymm1
  vpmovmskb %ymm1, %eax
  cmpl $-1, %eax
  jne .Lmemcmp_avx2_diff
  addq $32, %r8
  vmovdqu (%rdi,%r8), %ymm1
  vpcmpeqb (%rsi,%r8), %ymm1, %ymm1
  vpmovmskb %ymm1, %eax
  cmpl $-1, %eax
  jne .Lmemcmp_avx2_diff
  ud2
.Lmemcmp_avx2_mismatch_tail256:
  addq $32, %r8
  vmovdqu (%rdi,%r8), %ymm1
  vpcmpeqb (%rsi,%r8), %ymm1, %ymm1
  vpmovmskb %ymm1, %eax
  cmpl $-1, %eax
  jne .Lmemcmp_avx2_diff
  addq $32, %r8
  vmovdqu (%rdi,%r8), %ymm1
  vpcmpeqb (%rsi,%r8), %ymm1, %ymm1
  vpmovmskb %ymm1, %eax
  cmpl $-1, %eax
  jne .Lmemcmp_avx2_diff
  addq $32, %r8
  vmovdqu (%rdi,%r8), %ymm1
  vpcmpeqb (%rsi,%r8), %ymm1, %ymm1
  vpmovmskb %ymm1, %eax
  cmpl $-1, %eax
  jne .Lmemcmp_avx2_diff
  addq $32, %r8
  vmovdqu (%rdi,%r8), %ymm1
  vpcmpeqb (%rsi,%r8), %ymm1, %ymm1
  vpmovmskb %ymm1, %eax
  cmpl $-1, %eax
  jne .Lmemcmp_avx2_diff
  addq $32, %r8
  vmovdqu (%rdi,%r8), %ymm1
  vpcmpeqb (%rsi,%r8), %ymm1, %ymm1
  vpmovmskb %ymm1, %eax
  cmpl $-1, %eax
  jne .Lmemcmp_avx2_diff
  addq $32, %r8
  vmovdqu (%rdi,%r8), %ymm1
  vpcmpeqb (%rsi,%r8), %ymm1, %ymm1
  vpmovmskb %ymm1, %eax
  cmpl $-1, %eax
  jne .Lmemcmp_avx2_diff
  addq $32, %r8
  vmovdqu (%rdi,%r8), %ymm1
  vpcmpeqb (%rsi,%r8), %ymm1, %ymm1
  vpmovmskb %ymm1, %eax
  cmpl $-1, %eax
  jne .Lmemcmp_avx2_diff
  ud2
.Lmemcmp_avx2_diff:
  vzeroupper
  notl %eax
  bsfl %eax, %eax
  addq %r8, %rax
  movzbl (%rdi,%rax), %ecx
  movzbl (%rsi,%rax), %edx
  subl %edx, %ecx
  movl %ecx, %eax
  ret
.size nelibc_tcc_sse_memcmp, .-nelibc_tcc_sse_memcmp
.size nelibc_x86_64_sse_memcmp, .-nelibc_x86_64_sse_memcmp
#endif

/* Bounded SSE2 fill path; the scalar tail never stores beyond n bytes. */
.globl nelibc_x86_64_sse_memset
.type nelibc_x86_64_sse_memset,@function
nelibc_x86_64_sse_memset:
  movq %rdi, %rax
  testq %rdx, %rdx
  jz .Lmemset_done
  movd %esi, %xmm0
  punpcklbw %xmm0, %xmm0
  pshuflw $0, %xmm0, %xmm0
  pshufd $0, %xmm0, %xmm0
.Lmemset_loop64:
  cmpq $64, %rdx
  jb .Lmemset_loop16
  movdqu %xmm0, 0(%rdi)
  movdqu %xmm0, 16(%rdi)
  movdqu %xmm0, 32(%rdi)
  movdqu %xmm0, 48(%rdi)
  addq $64, %rdi
  subq $64, %rdx
  jmp .Lmemset_loop64
.Lmemset_loop16:
  cmpq $16, %rdx
  jb .Lmemset_tail
  movdqu %xmm0, (%rdi)
  addq $16, %rdi
  subq $16, %rdx
  jmp .Lmemset_loop16
.Lmemset_tail:
  testq %rdx, %rdx
  jz .Lmemset_done
  movb %sil, (%rdi)
  incq %rdi
  decq %rdx
  jmp .Lmemset_tail
.Lmemset_done:
  ret
.size nelibc_x86_64_sse_memset, .-nelibc_x86_64_sse_memset

.globl nelibc_tcc_sse_memmove
.type nelibc_tcc_sse_memmove,@function
.globl nelibc_x86_64_sse_memmove
.type nelibc_x86_64_sse_memmove,@function
.globl nelibc_tcc_memmove_512
.type nelibc_tcc_memmove_512,@function
nelibc_tcc_sse_memmove:
nelibc_x86_64_sse_memmove:
  xorl %r10d, %r10d
  jmp .Lmemmove_common_entry
nelibc_tcc_memmove_512:
  movl $1, %r10d
.Lmemmove_common_entry:
  movq %rdi, %r9
#ifndef NELIBC_DISABLE_AVX2
  cmpq $NELIBC_MEMMOVE_AVX2_THRESHOLD, %rdx
  jb .Lmemmove_sse_entry
  cmpb $2, .Lmemscan_avx2_state(%rip)
  je .Lmemmove_avx2_entry
  cmpb $1, .Lmemscan_avx2_state(%rip)
  je .Lmemmove_sse_entry
  pushq %rdx
  call nelibc_x86_64_memscan_has_avx2
  popq %rdx
  testl %eax, %eax
  jnz .Lmemmove_avx2_entry
.Lmemmove_sse_entry:
#endif
  testq %rdx, %rdx
  jz .Lmemmove_done
  cmpq %rsi, %rdi
  je .Lmemmove_done
  cmpq %rsi, %rdi
  jbe .Lmemmove_forward
  movq %rdi, %r8
  subq %rsi, %r8
  cmpq %rdx, %r8
  jae .Lmemmove_forward

  leaq (%rdi,%rdx), %rdi
  leaq (%rsi,%rdx), %rsi
.Lmemmove_backward_vector:
  cmpq $64, %rdx
  jb .Lmemmove_backward_vector16
  subq $64, %rdi
  subq $64, %rsi
  movdqu 0(%rsi), %xmm0
  movdqu 16(%rsi), %xmm1
  movdqu 32(%rsi), %xmm2
  movdqu 48(%rsi), %xmm3
  movdqu %xmm0, 0(%rdi)
  movdqu %xmm1, 16(%rdi)
  movdqu %xmm2, 32(%rdi)
  movdqu %xmm3, 48(%rdi)
  subq $64, %rdx
  jmp .Lmemmove_backward_vector
.Lmemmove_backward_vector16:
  cmpq $16, %rdx
  jb .Lmemmove_backward_tail
  subq $16, %rdi
  subq $16, %rsi
  movdqu (%rsi), %xmm0
  movdqu %xmm0, (%rdi)
  subq $16, %rdx
  jmp .Lmemmove_backward_vector
.Lmemmove_backward_tail:
  testq %rdx, %rdx
  jz .Lmemmove_done
.Lmemmove_backward_byte:
  decq %rdi
  decq %rsi
  movb (%rsi), %cl
  movb %cl, (%rdi)
  decq %rdx
  jnz .Lmemmove_backward_byte
  jmp .Lmemmove_done

.Lmemmove_forward:
.Lmemmove_forward_vector:
  cmpq $64, %rdx
  jb .Lmemmove_forward_vector16
  movdqu 0(%rsi), %xmm0
  movdqu 16(%rsi), %xmm1
  movdqu 32(%rsi), %xmm2
  movdqu 48(%rsi), %xmm3
  movdqu %xmm0, 0(%rdi)
  movdqu %xmm1, 16(%rdi)
  movdqu %xmm2, 32(%rdi)
  movdqu %xmm3, 48(%rdi)
  addq $64, %rdi
  addq $64, %rsi
  subq $64, %rdx
  jmp .Lmemmove_forward_vector
.Lmemmove_forward_vector16:
  cmpq $16, %rdx
  jb .Lmemmove_forward_tail
  movdqu (%rsi), %xmm0
  movdqu %xmm0, (%rdi)
  addq $16, %rdi
  addq $16, %rsi
  subq $16, %rdx
  jmp .Lmemmove_forward_vector
.Lmemmove_forward_tail:
  testq %rdx, %rdx
  jz .Lmemmove_done
.Lmemmove_forward_byte:
  movb (%rsi), %cl
  movb %cl, (%rdi)
  incq %rdi
  incq %rsi
  decq %rdx
  jnz .Lmemmove_forward_byte
.Lmemmove_done:
  movq %r9, %rax
  ret
#ifndef NELIBC_DISABLE_AVX2
.Lmemmove_avx2_entry:
  testq %rdx, %rdx
  jz .Lmemmove_done
  testl %r10d, %r10d
  jnz .Lmemmove_avx2_512_entry
  cmpq %rsi, %rdi
  je .Lmemmove_done
  cmpq %rsi, %rdi
  jbe .Lmemmove_avx2_forward
  movq %rdi, %r8
  subq %rsi, %r8
  cmpq %rdx, %r8
  jae .Lmemmove_avx2_forward
  jmp .Lmemmove_backward_aligned_from_base
  nopl (%rax)
.Lmemmove_avx2_backward:
  cmpq $256, %rdx
  jb .Lmemmove_avx2_backward_tail
  subq $256, %rdi
  subq $256, %rsi
  vmovdqu 0(%rsi), %ymm0
  vmovdqu 32(%rsi), %ymm1
  vmovdqu 64(%rsi), %ymm2
  vmovdqu 96(%rsi), %ymm3
  vmovdqu 128(%rsi), %ymm4
  vmovdqu 160(%rsi), %ymm5
  vmovdqu 192(%rsi), %ymm6
  vmovdqu 224(%rsi), %ymm7
  vmovdqu %ymm0, 0(%rdi)
  vmovdqu %ymm1, 32(%rdi)
  vmovdqu %ymm2, 64(%rdi)
  vmovdqu %ymm3, 96(%rdi)
  vmovdqu %ymm4, 128(%rdi)
  vmovdqu %ymm5, 160(%rdi)
  vmovdqu %ymm6, 192(%rdi)
  vmovdqu %ymm7, 224(%rdi)
  subq $256, %rdx
  jmp .Lmemmove_avx2_backward
.Lmemmove_avx2_backward_tail:
  vzeroupper
  jmp .Lmemmove_backward_vector
.Lmemmove_avx2_forward:
.Lmemmove_avx2_forward_loop:
  cmpq $256, %rdx
  jb .Lmemmove_avx2_forward_tail
  vmovdqu 0(%rsi), %ymm0
  vmovdqu 32(%rsi), %ymm1
  vmovdqu 64(%rsi), %ymm2
  vmovdqu 96(%rsi), %ymm3
  vmovdqu 128(%rsi), %ymm4
  vmovdqu 160(%rsi), %ymm5
  vmovdqu 192(%rsi), %ymm6
  vmovdqu 224(%rsi), %ymm7
  vmovdqu %ymm0, 0(%rdi)
  vmovdqu %ymm1, 32(%rdi)
  vmovdqu %ymm2, 64(%rdi)
  vmovdqu %ymm3, 96(%rdi)
  vmovdqu %ymm4, 128(%rdi)
  vmovdqu %ymm5, 160(%rdi)
  vmovdqu %ymm6, 192(%rdi)
  vmovdqu %ymm7, 224(%rdi)
  addq $256, %rdi
  addq $256, %rsi
  subq $256, %rdx
  jmp .Lmemmove_avx2_forward_loop
.Lmemmove_avx2_forward_tail:
  vzeroupper
  jmp .Lmemmove_forward_vector
.Lmemmove_avx2_512_entry:
  testq %rdx, %rdx
  jz .Lmemmove_done
  cmpq %rsi, %rdi
  je .Lmemmove_done
  cmpq %rsi, %rdi
  jbe .Lmemmove_avx2_512_forward
  movq %rdi, %r8
  subq %rsi, %r8
  cmpq %rdx, %r8
  jae .Lmemmove_avx2_512_forward
  jmp .Lmemmove_backward_aligned_from_base
  nopl (%rax)
.Lmemmove_avx2_512_backward:
  cmpq $512, %rdx
  jb .Lmemmove_avx2_512_backward_tail
  subq $256, %rdi
  subq $256, %rsi
  vmovdqu 0(%rsi), %ymm0
  vmovdqu 32(%rsi), %ymm1
  vmovdqu 64(%rsi), %ymm2
  vmovdqu 96(%rsi), %ymm3
  vmovdqu 128(%rsi), %ymm4
  vmovdqu 160(%rsi), %ymm5
  vmovdqu 192(%rsi), %ymm6
  vmovdqu 224(%rsi), %ymm7
  vmovdqu %ymm0, 0(%rdi)
  vmovdqu %ymm1, 32(%rdi)
  vmovdqu %ymm2, 64(%rdi)
  vmovdqu %ymm3, 96(%rdi)
  vmovdqu %ymm4, 128(%rdi)
  vmovdqu %ymm5, 160(%rdi)
  vmovdqu %ymm6, 192(%rdi)
  vmovdqu %ymm7, 224(%rdi)
  subq $256, %rdi
  subq $256, %rsi
  vmovdqu 0(%rsi), %ymm0
  vmovdqu 32(%rsi), %ymm1
  vmovdqu 64(%rsi), %ymm2
  vmovdqu 96(%rsi), %ymm3
  vmovdqu 128(%rsi), %ymm4
  vmovdqu 160(%rsi), %ymm5
  vmovdqu 192(%rsi), %ymm6
  vmovdqu 224(%rsi), %ymm7
  vmovdqu %ymm0, 0(%rdi)
  vmovdqu %ymm1, 32(%rdi)
  vmovdqu %ymm2, 64(%rdi)
  vmovdqu %ymm3, 96(%rdi)
  vmovdqu %ymm4, 128(%rdi)
  vmovdqu %ymm5, 160(%rdi)
  vmovdqu %ymm6, 192(%rdi)
  vmovdqu %ymm7, 224(%rdi)
  subq $512, %rdx
  jmp .Lmemmove_avx2_512_backward
.Lmemmove_avx2_512_backward_tail:
  vzeroupper
  jmp .Lmemmove_backward_vector
.Lmemmove_avx2_512_forward:
.Lmemmove_avx2_512_forward_loop:
  cmpq $512, %rdx
  jb .Lmemmove_avx2_512_forward_tail
  vmovdqu 0(%rsi), %ymm0
  vmovdqu 32(%rsi), %ymm1
  vmovdqu 64(%rsi), %ymm2
  vmovdqu 96(%rsi), %ymm3
  vmovdqu 128(%rsi), %ymm4
  vmovdqu 160(%rsi), %ymm5
  vmovdqu 192(%rsi), %ymm6
  vmovdqu 224(%rsi), %ymm7
  vmovdqu %ymm0, 0(%rdi)
  vmovdqu %ymm1, 32(%rdi)
  vmovdqu %ymm2, 64(%rdi)
  vmovdqu %ymm3, 96(%rdi)
  vmovdqu %ymm4, 128(%rdi)
  vmovdqu %ymm5, 160(%rdi)
  vmovdqu %ymm6, 192(%rdi)
  vmovdqu %ymm7, 224(%rdi)
  vmovdqu 256(%rsi), %ymm0
  vmovdqu 288(%rsi), %ymm1
  vmovdqu 320(%rsi), %ymm2
  vmovdqu 352(%rsi), %ymm3
  vmovdqu 384(%rsi), %ymm4
  vmovdqu 416(%rsi), %ymm5
  vmovdqu 448(%rsi), %ymm6
  vmovdqu 480(%rsi), %ymm7
  vmovdqu %ymm0, 256(%rdi)
  vmovdqu %ymm1, 288(%rdi)
  vmovdqu %ymm2, 320(%rdi)
  vmovdqu %ymm3, 352(%rdi)
  vmovdqu %ymm4, 384(%rdi)
  vmovdqu %ymm5, 416(%rdi)
  vmovdqu %ymm6, 448(%rdi)
  vmovdqu %ymm7, 480(%rdi)
  addq $512, %rdi
  addq $512, %rsi
  subq $512, %rdx
  jmp .Lmemmove_avx2_512_forward_loop
.Lmemmove_avx2_512_forward_tail:
  vzeroupper
  jmp .Lmemmove_forward_vector
#endif
.size nelibc_tcc_sse_memmove, .-nelibc_tcc_sse_memmove
.size nelibc_x86_64_sse_memmove, .-nelibc_x86_64_sse_memmove
.size nelibc_tcc_memmove_512, .-nelibc_tcc_memmove_512

.globl nelibc_tcc_sse_memccpy
.type nelibc_tcc_sse_memccpy,@function
nelibc_tcc_sse_memccpy:
  movzbl %dl, %eax
  movd %eax, %xmm0
  punpcklbw %xmm0, %xmm0
  pshuflw $0, %xmm0, %xmm0
  pshufd $0, %xmm0, %xmm0
.Lmemccpy_vector:
  cmpq $16, %rcx
  jb .Lmemccpy_tail
  movdqu (%rsi), %xmm1
  movdqa %xmm1, %xmm2
  pcmpeqb %xmm0, %xmm2
  pmovmskb %xmm2, %eax
  testl %eax, %eax
  jnz .Lmemccpy_found_vector
  movdqu %xmm1, (%rdi)
  addq $16, %rsi
  addq $16, %rdi
  subq $16, %rcx
  jmp .Lmemccpy_vector
.Lmemccpy_found_vector:
  bsfl %eax, %eax
  leal 1(%rax), %ecx
  rep movsb
  movq %rdi, %rax
  ret
.Lmemccpy_tail:
  testq %rcx, %rcx
  jz .Lmemccpy_not_found
.Lmemccpy_tail_loop:
  movb (%rsi), %r8b
  movb %r8b, (%rdi)
  incq %rsi
  incq %rdi
  decq %rcx
  cmpb %dl, %r8b
  je .Lmemccpy_found_scalar
  testq %rcx, %rcx
  jnz .Lmemccpy_tail_loop
.Lmemccpy_not_found:
  xorl %eax, %eax
  ret
.Lmemccpy_found_scalar:
  movq %rdi, %rax
  ret
.size nelibc_tcc_sse_memccpy, .-nelibc_tcc_sse_memccpy

#ifndef NELIBC_DISABLE_AVX2
.Lmemmove_backward_aligned_from_base:
  leaq (%rdi,%rdx), %rdi
  leaq (%rsi,%rdx), %rsi
  testl %r10d, %r10d
  jz .Lmemmove_backward_aligned_align_tail
  cmpq $4096, %rdx
  jb .Lmemmove_avx2_512_backward
.Lmemmove_backward_aligned_align_tail:
  movq %rdi, %r8
  andq $31, %r8
  jz .Lmemmove_backward_aligned_align_tail_done
.Lmemmove_backward_aligned_align_tail_loop:
  decq %rdi
  decq %rsi
  movb (%rsi), %cl
  movb %cl, (%rdi)
  decq %rdx
  decq %r8
  jnz .Lmemmove_backward_aligned_align_tail_loop
.Lmemmove_backward_aligned_align_tail_done:
  testl %r10d, %r10d
  jnz .Lmemmove_backward_aligned_512_loop
.p2align 5
.Lmemmove_backward_aligned_256_loop:
  cmpq $256, %rdx
  jb .Lmemmove_backward_aligned_sse_tail
  subq $256, %rdi
  subq $256, %rsi
  vmovdqu 0(%rsi), %ymm0
  vmovdqu 32(%rsi), %ymm1
  vmovdqu 64(%rsi), %ymm2
  vmovdqu 96(%rsi), %ymm3
  vmovdqu 128(%rsi), %ymm4
  vmovdqu 160(%rsi), %ymm5
  vmovdqu 192(%rsi), %ymm6
  vmovdqu 224(%rsi), %ymm7
  vmovdqu %ymm0, 0(%rdi)
  vmovdqu %ymm1, 32(%rdi)
  vmovdqu %ymm2, 64(%rdi)
  vmovdqu %ymm3, 96(%rdi)
  vmovdqu %ymm4, 128(%rdi)
  vmovdqu %ymm5, 160(%rdi)
  vmovdqu %ymm6, 192(%rdi)
  vmovdqu %ymm7, 224(%rdi)
  subq $256, %rdx
  jmp .Lmemmove_backward_aligned_256_loop
.p2align 5
.Lmemmove_backward_aligned_512_loop:
  cmpq $512, %rdx
  jb .Lmemmove_backward_aligned_sse_tail
  subq $256, %rdi
  subq $256, %rsi
  vmovdqu 0(%rsi), %ymm0
  vmovdqu 32(%rsi), %ymm1
  vmovdqu 64(%rsi), %ymm2
  vmovdqu 96(%rsi), %ymm3
  vmovdqu 128(%rsi), %ymm4
  vmovdqu 160(%rsi), %ymm5
  vmovdqu 192(%rsi), %ymm6
  vmovdqu 224(%rsi), %ymm7
  vmovdqu %ymm0, 0(%rdi)
  vmovdqu %ymm1, 32(%rdi)
  vmovdqu %ymm2, 64(%rdi)
  vmovdqu %ymm3, 96(%rdi)
  vmovdqu %ymm4, 128(%rdi)
  vmovdqu %ymm5, 160(%rdi)
  vmovdqu %ymm6, 192(%rdi)
  vmovdqu %ymm7, 224(%rdi)
  subq $256, %rdi
  subq $256, %rsi
  vmovdqu 0(%rsi), %ymm0
  vmovdqu 32(%rsi), %ymm1
  vmovdqu 64(%rsi), %ymm2
  vmovdqu 96(%rsi), %ymm3
  vmovdqu 128(%rsi), %ymm4
  vmovdqu 160(%rsi), %ymm5
  vmovdqu 192(%rsi), %ymm6
  vmovdqu 224(%rsi), %ymm7
  vmovdqu %ymm0, 0(%rdi)
  vmovdqu %ymm1, 32(%rdi)
  vmovdqu %ymm2, 64(%rdi)
  vmovdqu %ymm3, 96(%rdi)
  vmovdqu %ymm4, 128(%rdi)
  vmovdqu %ymm5, 160(%rdi)
  vmovdqu %ymm6, 192(%rdi)
  vmovdqu %ymm7, 224(%rdi)
  subq $512, %rdx
  jmp .Lmemmove_backward_aligned_512_loop
.Lmemmove_backward_aligned_sse_tail:
  vzeroupper
  jmp .Lmemmove_backward_vector
#endif

.macro NELIBC_FOLD_ASCII X, T1, T2
  movdqa \X, \T1
  pcmpgtb %xmm1, \T1
  movdqa %xmm2, \T2
  pcmpgtb \X, \T2
  pand \T2, \T1
  pand %xmm3, \T1
  por \T1, \X
.endm

.macro NELIBC_FOLD_ASCII_BYTE V, TEMP, RANGE
  movl \V, \TEMP
  orl $32, \TEMP
  leal -97(\TEMP), \RANGE
  cmpl $25, \RANGE
  cmovbel \TEMP, \V
.endm

.macro NELIBC_FOLD_ASCII_AVX2 X, T
  # x+63 is signed 0x80..0x99 exactly for A..Z; add 0x20 for that range.
  vpaddb %ymm1, \X, \T
  vpcmpgtb %ymm2, \T, \T
  vpandn %ymm3, \T, \T
  vpaddb \T, \X, \X
.endm

.macro NELIBC_COMPARE_CASEFOLD_AVX2
  NELIBC_FOLD_ASCII_AVX2 %ymm4, %ymm6
  NELIBC_FOLD_ASCII_AVX2 %ymm5, %ymm6
  # A right-only NUL is a mismatch; left-NUL folded equality detects both.
  vpcmpeqb %ymm5, %ymm4, %ymm6
  vpcmpeqb %ymm0, %ymm4, %ymm7
  vpandn %ymm6, %ymm7, %ymm6
  vpmovmskb %ymm6, %eax
  notl %eax
.endm

.macro NELIBC_COMPARE_PAIR_AVX2
  vmovdqu (%rdi), %ymm4
  vmovdqu (%rsi), %ymm5
  NELIBC_COMPARE_CASEFOLD_AVX2
  movl %eax, %r8d
  vmovdqu 32(%rdi), %ymm4
  vmovdqu 32(%rsi), %ymm5
  NELIBC_COMPARE_CASEFOLD_AVX2
  movl %eax, %ecx
  shlq $32, %rcx
  orq %rcx, %r8
  testq %r8, %r8
  jnz .Lstrcasecmp_avx2_decide64
.endm

.globl nelibc_x86_64_sse_strcasecmp
.type nelibc_x86_64_sse_strcasecmp,@function
nelibc_x86_64_sse_strcasecmp:
  # Resolve short strings and early mismatches without setting up SIMD.
  movzbl (%rdi), %eax
  movzbl (%rsi), %edx
  movl %eax, %ecx
  xorl %edx, %ecx
  jz .Lstrcasecmp_prefix_same
  cmpl $32, %ecx
  jne .Lstrcasecmp_prefix_fold
  cmpl $65, %eax
  jb .Lstrcasecmp_prefix_check_right
  cmpl $90, %eax
  jbe .Lstrcasecmp_prefix_continue
.Lstrcasecmp_prefix_check_right:
  cmpl $65, %edx
  jb .Lstrcasecmp_prefix_fold
  cmpl $90, %edx
  jbe .Lstrcasecmp_prefix_continue
  jmp .Lstrcasecmp_prefix_fold
.Lstrcasecmp_prefix_same:
  testl %eax, %eax
  je .Lstrcasecmp_prefix_return
  jmp .Lstrcasecmp_prefix_continue
.Lstrcasecmp_prefix_fold:
  NELIBC_FOLD_ASCII_BYTE %eax, %ecx, %r8d
  NELIBC_FOLD_ASCII_BYTE %edx, %ecx, %r8d
  cmpl %edx, %eax
  jne .Lstrcasecmp_prefix_return
  testl %eax, %eax
  je .Lstrcasecmp_prefix_return
.Lstrcasecmp_prefix_continue:
  incq %rdi
  incq %rsi
.Lstrcasecmp_check_avx2:
#ifndef NELIBC_DISABLE_AVX2
  cmpb $2, .Lmemscan_avx2_state(%rip)
  jae .Lstrcasecmp_avx2_setup
  cmpb $1, .Lmemscan_avx2_state(%rip)
  je .Lstrcasecmp_sse_setup
  call nelibc_x86_64_memscan_has_avx2
  testl %eax, %eax
  jnz .Lstrcasecmp_avx2_setup
#endif
.Lstrcasecmp_sse_setup:
  movdqa .Lcasefold_zero(%rip), %xmm0
  movdqa .Lcasefold_lower(%rip), %xmm1
  movdqa .Lcasefold_upper(%rip), %xmm2
  movdqa .Lcasefold_bit(%rip), %xmm3
  jmp .Lstrcasecmp_vector32

#ifndef NELIBC_DISABLE_AVX2
.Lstrcasecmp_avx2_setup:
  vmovdqa .Lcasefold_avx2_zero(%rip), %ymm0
  vmovdqa .Lcasefold_avx2_lower(%rip), %ymm1
  vmovdqa .Lcasefold_avx2_upper(%rip), %ymm2
  vmovdqa .Lcasefold_avx2_bit(%rip), %ymm3
.Lstrcasecmp_avx2_256_loop:
  # Full 256-byte spans avoid repeated page checks on long strings.
  movq %rdi, %rax
  orq %rsi, %rax
  andl $4095, %eax
  cmpl $3840, %eax
  ja .Lstrcasecmp_avx2_128_loop
  NELIBC_COMPARE_PAIR_AVX2
  addq $64, %rdi
  addq $64, %rsi
  NELIBC_COMPARE_PAIR_AVX2
  addq $64, %rdi
  addq $64, %rsi
  NELIBC_COMPARE_PAIR_AVX2
  addq $64, %rdi
  addq $64, %rsi
  NELIBC_COMPARE_PAIR_AVX2
  addq $64, %rdi
  addq $64, %rsi
  jmp .Lstrcasecmp_avx2_256_loop

.Lstrcasecmp_avx2_128_loop:
  # Four 32-byte blocks per page check for long strings.
  movq %rdi, %rax
  orq %rsi, %rax
  andl $4095, %eax
  cmpl $3968, %eax
  ja .Lstrcasecmp_avx2_64_loop
  NELIBC_COMPARE_PAIR_AVX2
  addq $64, %rdi
  addq $64, %rsi
  NELIBC_COMPARE_PAIR_AVX2
  addq $64, %rdi
  addq $64, %rsi
  jmp .Lstrcasecmp_avx2_256_loop

.Lstrcasecmp_avx2_64_loop:
  # Two 32-byte blocks per check; never let an unaligned load cross a page.
  movq %rdi, %rax
  orq %rsi, %rax
  andl $4095, %eax
  cmpl $4032, %eax
  ja .Lstrcasecmp_avx2_one_block
  NELIBC_COMPARE_PAIR_AVX2
  addq $64, %rdi
  addq $64, %rsi
  jmp .Lstrcasecmp_avx2_128_loop

.Lstrcasecmp_avx2_one_block:
  cmpl $4064, %eax
  ja .Lstrcasecmp_avx2_scalar
  vmovdqu (%rdi), %ymm4
  vmovdqu (%rsi), %ymm5
  NELIBC_COMPARE_CASEFOLD_AVX2
  testl %eax, %eax
  jnz .Lstrcasecmp_avx2_decide
  addq $32, %rdi
  addq $32, %rsi
  jmp .Lstrcasecmp_avx2_128_loop

.Lstrcasecmp_avx2_scalar:
  movzbl (%rdi), %eax
  movzbl (%rsi), %edx
  cmpl $65, %eax
  jb .Lstrcasecmp_avx2_left_ready
  cmpl $90, %eax
  ja .Lstrcasecmp_avx2_left_ready
  addl $32, %eax
.Lstrcasecmp_avx2_left_ready:
  cmpl $65, %edx
  jb .Lstrcasecmp_avx2_right_ready
  cmpl $90, %edx
  ja .Lstrcasecmp_avx2_right_ready
  addl $32, %edx
.Lstrcasecmp_avx2_right_ready:
  cmpl %edx, %eax
  jne .Lstrcasecmp_avx2_scalar_return
  testl %eax, %eax
  je .Lstrcasecmp_avx2_scalar_return
  incq %rdi
  incq %rsi
  movq %rdi, %rax
  orq %rsi, %rax
  andl $4095, %eax
  cmpl $4064, %eax
  jbe .Lstrcasecmp_avx2_64_loop
  jmp .Lstrcasecmp_avx2_scalar

.Lstrcasecmp_avx2_decide64:
  bsfq %r8, %rcx
  jmp .Lstrcasecmp_avx2_decide_bytes
.Lstrcasecmp_avx2_decide:
  bsfl %eax, %ecx
.Lstrcasecmp_avx2_decide_bytes:
  movzbl (%rdi,%rcx), %eax
  movzbl (%rsi,%rcx), %edx
  cmpl $65, %eax
  jb .Lstrcasecmp_avx2_decide_left_ready
  cmpl $90, %eax
  ja .Lstrcasecmp_avx2_decide_left_ready
  addl $32, %eax
.Lstrcasecmp_avx2_decide_left_ready:
  cmpl $65, %edx
  jb .Lstrcasecmp_avx2_decide_right_ready
  cmpl $90, %edx
  ja .Lstrcasecmp_avx2_decide_right_ready
  addl $32, %edx
.Lstrcasecmp_avx2_decide_right_ready:
  subl %edx, %eax
  vzeroupper
  ret
.Lstrcasecmp_avx2_scalar_return:
  subl %edx, %eax
  vzeroupper
  ret
#endif

.Lstrcasecmp_vector32:
  # Two-block unroll amortizes page checks and loop branches for long strings.
  movq %rdi, %rax
  orq %rsi, %rax
  andl $4095, %eax
  cmpl $4064, %eax
  ja .Lstrcasecmp_vector16
  movdqu (%rdi), %xmm4
  movdqu (%rsi), %xmm5
  NELIBC_FOLD_ASCII %xmm4, %xmm6, %xmm7
  NELIBC_FOLD_ASCII %xmm5, %xmm6, %xmm7
  movdqa %xmm4, %xmm6
  pcmpeqb %xmm0, %xmm6
  movdqa %xmm5, %xmm7
  pcmpeqb %xmm0, %xmm7
  por %xmm7, %xmm6
  pmovmskb %xmm6, %ecx
  pcmpeqb %xmm5, %xmm4
  pmovmskb %xmm4, %eax
  notl %eax
  orl %ecx, %eax
  andl $0xFFFF, %eax
  jnz .Lstrcasecmp_decide
  addq $16, %rdi
  addq $16, %rsi
  movdqu (%rdi), %xmm4
  movdqu (%rsi), %xmm5
  NELIBC_FOLD_ASCII %xmm4, %xmm6, %xmm7
  NELIBC_FOLD_ASCII %xmm5, %xmm6, %xmm7
  movdqa %xmm4, %xmm6
  pcmpeqb %xmm0, %xmm6
  movdqa %xmm5, %xmm7
  pcmpeqb %xmm0, %xmm7
  por %xmm7, %xmm6
  pmovmskb %xmm6, %ecx
  pcmpeqb %xmm5, %xmm4
  pmovmskb %xmm4, %eax
  notl %eax
  orl %ecx, %eax
  andl $0xFFFF, %eax
  jnz .Lstrcasecmp_decide
  addq $16, %rdi
  addq $16, %rsi
  jmp .Lstrcasecmp_vector32

.Lstrcasecmp_vector16:
  # Keep every unaligned 16-byte load within its current 4 KiB page.
  movq %rdi, %rax
  orq %rsi, %rax
  andl $4095, %eax
  cmpl $4080, %eax
  ja .Lstrcasecmp_scalar
  movdqu (%rdi), %xmm4
  movdqu (%rsi), %xmm5
  NELIBC_FOLD_ASCII %xmm4, %xmm6, %xmm7
  NELIBC_FOLD_ASCII %xmm5, %xmm6, %xmm7
  movdqa %xmm4, %xmm6
  pcmpeqb %xmm0, %xmm6
  movdqa %xmm5, %xmm7
  pcmpeqb %xmm0, %xmm7
  por %xmm7, %xmm6
  pmovmskb %xmm6, %ecx
  pcmpeqb %xmm5, %xmm4
  pmovmskb %xmm4, %eax
  notl %eax
  orl %ecx, %eax
  andl $0xFFFF, %eax
  jnz .Lstrcasecmp_decide
  addq $16, %rdi
  addq $16, %rsi
  jmp .Lstrcasecmp_vector32
.Lstrcasecmp_decide:
  bsfl %eax, %ecx
  movzbl (%rdi,%rcx), %eax
  movzbl (%rsi,%rcx), %edx
  cmpl $65, %eax
  jb .Lstrcasecmp_left_ready
  cmpl $90, %eax
  ja .Lstrcasecmp_left_ready
  addl $32, %eax
.Lstrcasecmp_left_ready:
  cmpl $65, %edx
  jb .Lstrcasecmp_right_ready
  cmpl $90, %edx
  ja .Lstrcasecmp_right_ready
  addl $32, %edx
.Lstrcasecmp_right_ready:
  subl %edx, %eax
  ret
.Lstrcasecmp_scalar:
  movzbl (%rdi), %eax
  movzbl (%rsi), %edx
  cmpl $65, %eax
  jb .Lstrcasecmp_scalar_left_ready
  cmpl $90, %eax
  ja .Lstrcasecmp_scalar_left_ready
  addl $32, %eax
.Lstrcasecmp_scalar_left_ready:
  cmpl $65, %edx
  jb .Lstrcasecmp_scalar_right_ready
  cmpl $90, %edx
  ja .Lstrcasecmp_scalar_right_ready
  addl $32, %edx
.Lstrcasecmp_scalar_right_ready:
  cmpl %edx, %eax
  jne .Lstrcasecmp_scalar_return
  testl %eax, %eax
  je .Lstrcasecmp_scalar_return
  incq %rdi
  incq %rsi
  # Resume SIMD after a scalar page-edge bridge once both 32-byte loads fit.
  movq %rdi, %rax
  orq %rsi, %rax
  andl $4095, %eax
  cmpl $4064, %eax
  jbe .Lstrcasecmp_vector32
  jmp .Lstrcasecmp_scalar
.Lstrcasecmp_scalar_return:
  subl %edx, %eax
  ret
.Lstrcasecmp_prefix_return:
  subl %edx, %eax
  ret
.size nelibc_x86_64_sse_strcasecmp, .-nelibc_x86_64_sse_strcasecmp

.globl nelibc_x86_64_sse_strncasecmp
.type nelibc_x86_64_sse_strncasecmp,@function
nelibc_x86_64_sse_strncasecmp:
  # Handle full 32-byte prefixes with two vectors, avoiding a 16-byte vector
  # followed by a long scalar tail for common bounded comparisons.
  cmpq $32, %rdx
  jb .Lstrncasecmp_legacy
  movdqa .Lcasefold_zero(%rip), %xmm0
  movdqa .Lcasefold_lower(%rip), %xmm1
  movdqa .Lcasefold_upper(%rip), %xmm2
  movdqa .Lcasefold_bit(%rip), %xmm3
.Lstrncasecmp_vector32:
  movq %rdi, %rax
  orq %rsi, %rax
  andl $4095, %eax
  cmpl $4064, %eax
  ja .Lstrncasecmp_legacy
  movdqu 0(%rdi), %xmm4
  movdqu 16(%rdi), %xmm6
  movdqu 0(%rsi), %xmm5
  movdqu 16(%rsi), %xmm7
  NELIBC_FOLD_ASCII %xmm4, %xmm8, %xmm9
  NELIBC_FOLD_ASCII %xmm6, %xmm8, %xmm9
  NELIBC_FOLD_ASCII %xmm5, %xmm8, %xmm9
  NELIBC_FOLD_ASCII %xmm7, %xmm8, %xmm9

  # Build masks for folded mismatches and either string's terminator.
  movdqa %xmm4, %xmm8
  pcmpeqb %xmm5, %xmm8
  pmovmskb %xmm8, %ecx
  notl %ecx
  andl $0xFFFF, %ecx
  movl %ecx, %r8d
  movdqa %xmm6, %xmm8
  pcmpeqb %xmm7, %xmm8
  pmovmskb %xmm8, %ecx
  notl %ecx
  andl $0xFFFF, %ecx
  shll $16, %ecx
  orl %ecx, %r8d

  movdqa %xmm4, %xmm8
  pcmpeqb %xmm0, %xmm8
  pmovmskb %xmm8, %eax
  movdqa %xmm6, %xmm8
  pcmpeqb %xmm0, %xmm8
  pmovmskb %xmm8, %ecx
  shll $16, %ecx
  orl %ecx, %eax
  movdqa %xmm5, %xmm8
  pcmpeqb %xmm0, %xmm8
  pmovmskb %xmm8, %ecx
  orl %ecx, %eax
  movdqa %xmm7, %xmm8
  pcmpeqb %xmm0, %xmm8
  pmovmskb %xmm8, %ecx
  shll $16, %ecx
  orl %ecx, %eax
  orl %r8d, %eax
  jnz .Lstrncasecmp_vector32_decide

  addq $32, %rdi
  addq $32, %rsi
  subq $32, %rdx
  cmpq $32, %rdx
  jae .Lstrncasecmp_vector32
  jmp .Lstrncasecmp_legacy

.Lstrncasecmp_vector32_decide:
  bsfl %eax, %ecx
  movzbl (%rdi,%rcx), %eax
  movzbl (%rsi,%rcx), %edx
  cmpl $65, %eax
  jb .Lstrncasecmp_vector32_left_ready
  cmpl $90, %eax
  ja .Lstrncasecmp_vector32_left_ready
  addl $32, %eax
.Lstrncasecmp_vector32_left_ready:
  cmpl $65, %edx
  jb .Lstrncasecmp_vector32_right_ready
  cmpl $90, %edx
  ja .Lstrncasecmp_vector32_right_ready
  addl $32, %edx
.Lstrncasecmp_vector32_right_ready:
  subl %edx, %eax
  ret

.Lstrncasecmp_legacy:
  testq %rdx, %rdx
  jz .Lstrncasecmp_zero
  # Compare the first bounded byte before setting up SIMD state.
  movzbl (%rdi), %eax
  movzbl (%rsi), %ecx
  leal -65(%rax), %r8d
  cmpl $25, %r8d
  setbe %r8b
  movzbl %r8b, %r8d
  shll $5, %r8d
  addl %r8d, %eax
  leal -65(%rcx), %r8d
  cmpl $25, %r8d
  setbe %r8b
  movzbl %r8b, %r8d
  shll $5, %r8d
  addl %r8d, %ecx
  cmpl %ecx, %eax
  jne .Lstrncasecmp_prefix_return
  testl %eax, %eax
  je .Lstrncasecmp_prefix_return
  incq %rdi
  incq %rsi
  decq %rdx
  movdqa .Lcasefold_zero(%rip), %xmm0
  movdqa .Lcasefold_lower(%rip), %xmm1
  movdqa .Lcasefold_upper(%rip), %xmm2
  movdqa .Lcasefold_bit(%rip), %xmm3
.Lstrncasecmp_vector:
  cmpq $16, %rdx
  jb .Lstrncasecmp_scalar
  movq %rdi, %rax
  orq %rsi, %rax
  andl $4095, %eax
  cmpl $4080, %eax
  ja .Lstrncasecmp_scalar
  movdqu (%rdi), %xmm4
  movdqu (%rsi), %xmm5
  NELIBC_FOLD_ASCII %xmm4, %xmm6, %xmm7
  NELIBC_FOLD_ASCII %xmm5, %xmm6, %xmm7
  movdqa %xmm4, %xmm6
  pcmpeqb %xmm0, %xmm6
  movdqa %xmm5, %xmm7
  pcmpeqb %xmm0, %xmm7
  por %xmm7, %xmm6
  pmovmskb %xmm6, %ecx
  pcmpeqb %xmm5, %xmm4
  pmovmskb %xmm4, %eax
  notl %eax
  orl %ecx, %eax
  andl $0xFFFF, %eax
  jnz .Lstrncasecmp_decide
  addq $16, %rdi
  addq $16, %rsi
  subq $16, %rdx
  jmp .Lstrncasecmp_vector
.Lstrncasecmp_decide:
  bsfl %eax, %ecx
  movzbl (%rdi,%rcx), %eax
  movzbl (%rsi,%rcx), %edx
  cmpl $65, %eax
  jb .Lstrncasecmp_left_ready
  cmpl $90, %eax
  ja .Lstrncasecmp_left_ready
  addl $32, %eax
.Lstrncasecmp_left_ready:
  cmpl $65, %edx
  jb .Lstrncasecmp_right_ready
  cmpl $90, %edx
  ja .Lstrncasecmp_right_ready
  addl $32, %edx
.Lstrncasecmp_right_ready:
  subl %edx, %eax
  ret
.Lstrncasecmp_scalar:
  testq %rdx, %rdx
  jz .Lstrncasecmp_zero
  movzbl (%rdi), %eax
  movzbl (%rsi), %ecx
  cmpl $65, %eax
  jb .Lstrncasecmp_scalar_left_ready
  cmpl $90, %eax
  ja .Lstrncasecmp_scalar_left_ready
  addl $32, %eax
.Lstrncasecmp_scalar_left_ready:
  cmpl $65, %ecx
  jb .Lstrncasecmp_scalar_right_ready
  cmpl $90, %ecx
  ja .Lstrncasecmp_scalar_right_ready
  addl $32, %ecx
.Lstrncasecmp_scalar_right_ready:
  cmpl %ecx, %eax
  jne .Lstrncasecmp_scalar_return
  testl %eax, %eax
  je .Lstrncasecmp_scalar_return
  incq %rdi
  incq %rsi
  decq %rdx
  jmp .Lstrncasecmp_scalar
.Lstrncasecmp_scalar_return:
  subl %ecx, %eax
  ret
.Lstrncasecmp_zero:
  xorl %eax, %eax
  ret
.Lstrncasecmp_prefix_return:
  subl %ecx, %eax
  ret
.size nelibc_x86_64_sse_strncasecmp, .-nelibc_x86_64_sse_strncasecmp

.globl nelibc_tcc_sse_memset
.type nelibc_tcc_sse_memset,@function
nelibc_tcc_sse_memset:
  movq %rdi, %r9
  movzbl %sil, %r8d
  movd %r8d, %xmm0
  punpcklbw %xmm0, %xmm0
  punpcklwd %xmm0, %xmm0
  pshufd $0, %xmm0, %xmm0
.Lmemset_sse2_64:
  cmpq $64, %rdx
  jb .Lmemset_sse2_16
  movdqu %xmm0, 0(%rdi)
  movdqu %xmm0, 16(%rdi)
  movdqu %xmm0, 32(%rdi)
  movdqu %xmm0, 48(%rdi)
  addq $64, %rdi
  subq $64, %rdx
  jmp .Lmemset_sse2_64
.Lmemset_sse2_16:
  cmpq $16, %rdx
  jb .Lmemset_sse2_tail
  movdqu %xmm0, (%rdi)
  addq $16, %rdi
  subq $16, %rdx
  jmp .Lmemset_sse2_16
.Lmemset_sse2_tail:
  testq %rdx, %rdx
  jz .Lmemset_sse2_done
  movq %rdx, %rcx
  movl %r8d, %eax
  rep stosb
.Lmemset_sse2_done:
  movq %r9, %rax
  ret
.size nelibc_tcc_sse_memset, .-nelibc_tcc_sse_memset

.section .rodata
#ifndef NELIBC_DISABLE_AVX2
.p2align 5
.Lcasefold_avx2_zero:
  .rept 32
  .byte 0
  .endr
.Lcasefold_avx2_lower:
  .rept 32
  .byte 63
  .endr
.Lcasefold_avx2_upper:
  .rept 32
  .byte 153
  .endr
.Lcasefold_avx2_bit:
  .rept 32
  .byte 32
  .endr
#endif
.p2align 4
.Lcasefold_zero:
  .rept 16
  .byte 0
  .endr
.Lcasefold_lower:
  .rept 16
  .byte 64
  .endr
.Lcasefold_upper:
  .rept 16
  .byte 91
  .endr
.Lcasefold_bit:
  .rept 16
  .byte 32
  .endr

.section .note.GNU-stack,"",@progbits
