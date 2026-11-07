/* syscall(2) for the standalone profile: a raw Linux x86-64 system call with
 * up to six word arguments; a result in [-4095, -1] becomes errno. Kept in
 * its own object so the host profile, which has no raw system calls, can
 * leave it out. */
#include <stdarg.h>

extern long nelibc_raw_syscall6(unsigned long number, unsigned long a1, unsigned long a2,
	unsigned long a3, unsigned long a4, unsigned long a5, unsigned long a6);
extern int *nelibc_errno_location(void);

long syscall(long n, ...)
{
	va_list ap;
	unsigned long a, b, c, d, e, f;
	long r;
	va_start(ap, n);
	a = va_arg(ap, unsigned long);
	b = va_arg(ap, unsigned long);
	c = va_arg(ap, unsigned long);
	d = va_arg(ap, unsigned long);
	e = va_arg(ap, unsigned long);
	f = va_arg(ap, unsigned long);
	va_end(ap);
	r = nelibc_raw_syscall6((unsigned long)n, a, b, c, d, e, f);
	if (r < 0 && r >= -4095) {
		*nelibc_errno_location() = (int)-r;
		return -1;
	}
	return r;
}
