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
  pmovmskb %xmm2, %r8d
  shlq $16, %r8
  orq %r8, %rax
  pmovmskb %xmm3, %r8d
  shlq $32, %r8
  orq %r8, %rax
  pmovmskb %xmm4, %r8d
  shlq $48, %r8
  orq %r8, %rax
  bsfq %rax, %rax
  addq %rdi, %rax
  ret
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
  shlq $48, %rax
  pmovmskb %xmm2, %r8d
  shlq $32, %r8
  orq %r8, %rax
  pmovmskb %xmm3, %r8d
  shlq $16, %r8
  orq %r8, %rax
  pmovmskb %xmm4, %r8d
  orq %r8, %rax
  bsrq %rax, %rax
  addq %rdi, %rax
  ret
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
  je .Lmemcmp_avx2_loop
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
  jmp .Lmemcmp_avx2_loop
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
.Lmemcmp_avx2_loop:
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
  jne .Lmemcmp_avx2_mismatch_tail
  addq $128, %r8
  subq $128, %rdx
  cmpq $128, %rdx
  jae .Lmemcmp_avx2_loop
  vzeroupper
  jmp .Lmemcmp_vector128
.Lmemcmp_avx2_mismatch_tail:
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

.section .note.GNU-stack,"",@progbits
