/* <setjmp.h> for x86-64 System V, standalone and host profiles.
 *
 * jmp_buf is musl's layout (include/setjmp.h): eight saved words
 * (rbx rbp r12 r13 r14 r15 rsp rip), then the sigsetjmp flag at
 * byte 64 and the saved signal mask at byte 72. setjmp does not save the
 * signal mask (POSIX leaves that unspecified; musl and glibc's default
 * don't either), so setjmp/_setjmp and longjmp/_longjmp are the same
 * functions. The x87 control word and MXCSR are not saved, as in musl. */

	.text

	.globl setjmp
	.globl _setjmp
	.type setjmp,@function
	.type _setjmp,@function
setjmp:
_setjmp:
	mov %rbx, (%rdi)
	mov %rbp, 8(%rdi)
	mov %r12, 16(%rdi)
	mov %r13, 24(%rdi)
	mov %r14, 32(%rdi)
	mov %r15, 40(%rdi)
	lea 8(%rsp), %rdx        /* caller's stack pointer after the return */
	mov %rdx, 48(%rdi)
	mov (%rsp), %rdx         /* return address */
	mov %rdx, 56(%rdi)
	xor %eax, %eax
	ret
	.size setjmp, .-setjmp
	.size _setjmp, .-_setjmp

	.globl longjmp
	.globl _longjmp
	.type longjmp,@function
	.type _longjmp,@function
longjmp:
_longjmp:
	xor %eax, %eax
	cmp $1, %esi             /* val == 0 returns 1: CF set only when esi == 0 */
	adc %esi, %eax
	mov (%rdi), %rbx
	mov 8(%rdi), %rbp
	mov 16(%rdi), %r12
	mov 24(%rdi), %r13
	mov 32(%rdi), %r14
	mov 40(%rdi), %r15
	mov 48(%rdi), %rsp
	jmp *56(%rdi)
	.size longjmp, .-longjmp
	.size _longjmp, .-_longjmp

/* int sigsetjmp(sigjmp_buf env, int savemask): when savemask is nonzero,
 * record the current mask before the registers. The mask is read first so
 * that setjmp, reached by a tail jump, sees the caller's frame unchanged. */
	.globl sigsetjmp
	.type sigsetjmp,@function
sigsetjmp:
	movslq %esi, %rax
	mov %rax, 64(%rdi)
	test %esi, %esi
	jz setjmp
	push %rdi                /* also realigns the stack to 16 for the call */
	lea 72(%rdi), %rdx       /* old set */
	xor %esi, %esi           /* new set: NULL, so `how` is ignored */
	xor %edi, %edi
	call sigprocmask@PLT
	pop %rdi
	jmp setjmp
	.size sigsetjmp, .-sigsetjmp

/* void siglongjmp(sigjmp_buf env, int val): restore the mask saved by
 * sigsetjmp (if any), then longjmp. */
	.globl siglongjmp
	.type siglongjmp,@function
siglongjmp:
	cmpq $0, 64(%rdi)
	je longjmp
	push %rdi
	push %rsi
	sub $8, %rsp             /* 16-byte alignment for the call */
	lea 72(%rdi), %rsi       /* set */
	xor %edx, %edx           /* old set: NULL */
	mov $2, %edi             /* SIG_SETMASK */
	call sigprocmask@PLT
	add $8, %rsp
	pop %rsi
	pop %rdi
	jmp longjmp
	.size siglongjmp, .-siglongjmp

	.section .note.GNU-stack,"",@progbits
