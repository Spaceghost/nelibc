#include "syscall_interpose.h"

static int short_write_fd = -1;
static unsigned long short_write_successful_calls;
static int fail_next_mmap;

void nelibc_test_short_write_on_fd(int fd, unsigned long successful_calls) {
  short_write_fd = fd;
  short_write_successful_calls = successful_calls;
}

void nelibc_test_disable_short_write(void) { short_write_fd = -1; }
void nelibc_test_fail_next_mmap(void) { fail_next_mmap = 1; }

extern long nelibc_raw_syscall3_real(unsigned long number,
                                    unsigned long arg1,
                                    unsigned long arg2,
                                    unsigned long arg3);
extern long nelibc_raw_syscall6_real(unsigned long number,
                                    unsigned long arg1, unsigned long arg2,
                                    unsigned long arg3, unsigned long arg4,
                                    unsigned long arg5, unsigned long arg6);

long nelibc_raw_syscall3(unsigned long number, unsigned long arg1,
                         unsigned long arg2, unsigned long arg3) {
  if (number == 318 && arg3 == NELIBC_TEST_EINTR_FLAGS) return -4;
  if (number == 1 && short_write_fd >= 0 && (int)arg1 == short_write_fd) {
    if (short_write_successful_calls == 0) return -5;
    --short_write_successful_calls;
    if (arg3 > 1) arg3 = 1;
  }
  return nelibc_raw_syscall3_real(number, arg1, arg2, arg3);
}

long nelibc_raw_syscall6(unsigned long number, unsigned long arg1,
                         unsigned long arg2, unsigned long arg3,
                         unsigned long arg4, unsigned long arg5,
                         unsigned long arg6) {
  if (number == 9 && fail_next_mmap) {
    fail_next_mmap = 0;
    return -12;
  }
  return nelibc_raw_syscall6_real(number, arg1, arg2, arg3, arg4, arg5, arg6);
}
