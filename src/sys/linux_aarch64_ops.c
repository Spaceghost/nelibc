/* Translate Nelibc operations into the AArch64 Linux syscall ABI. */
#include <asm/errno.h>
#include <asm/unistd.h>
#include <linux/fcntl.h>
#include <linux/signal.h>
#include <linux/time_types.h>

#include "../../include/nelibc_sys.h"
#include "linux_aarch64_ops.h"

extern long nelibc_raw_syscall3(unsigned long number, unsigned long a1,
                                unsigned long a2, unsigned long a3);
extern long nelibc_raw_syscall6(unsigned long number, unsigned long a1,
                                unsigned long a2, unsigned long a3,
                                unsigned long a4, unsigned long a5,
                                unsigned long a6);

long nelibc_sys6(unsigned long op, unsigned long a1, unsigned long a2,
                 unsigned long a3, unsigned long a4, unsigned long a5,
                 unsigned long a6) {
  switch (op) {
    case NELIBC_SYS_OPEN:
      return nelibc_raw_syscall6(__NR_openat, AT_FDCWD, a1, a2, a3, 0, 0);
    case NELIBC_SYS_ACCESS:
      return nelibc_raw_syscall3(__NR_faccessat, AT_FDCWD, a1, a2);
    case NELIBC_SYS_PIPE:
      return nelibc_raw_syscall3(__NR_pipe2, a1, 0, 0);
    case NELIBC_SYS_DUP2:
      if ((int)a1 == (int)a2) {
        long result = nelibc_raw_syscall3(__NR_fcntl, a1, F_GETFD, 0);
        return result < 0 ? result : (int)a1;
      }
      return nelibc_raw_syscall3(__NR_dup3, a1, a2, 0);
    case NELIBC_SYS_READLINK:
      return nelibc_raw_syscall6(__NR_readlinkat, AT_FDCWD, a1, a2, a3, 0, 0);
    case NELIBC_SYS_CHMOD:
      return nelibc_raw_syscall3(__NR_fchmodat, AT_FDCWD, a1, a2);
    case NELIBC_SYS_GETPGRP:
      return nelibc_raw_syscall3(__NR_getpgid, 0, 0, 0);
    case NELIBC_SYS_FORK:
      /* With no child stack and no sharing flags, clone duplicates the
       * calling process and both copies return through the same stack. */
      return nelibc_raw_syscall6(__NR_clone, SIGCHLD, 0, 0, 0, 0, 0);
    case NELIBC_SYS_POLL: {
      int milliseconds = (int)a3;
      struct __kernel_timespec timeout;
      unsigned long timeout_address = 0;
      if (milliseconds >= 0) {
        timeout.tv_sec = milliseconds / 1000;
        timeout.tv_nsec = (milliseconds % 1000) * 1000000L;
        timeout_address = (unsigned long)&timeout;
      }
      return nelibc_raw_syscall6(__NR_ppoll, a1, a2, timeout_address, 0, 0, 0);
    }
  }

  if (op >= NELIBC_SYS_COUNT || !linux_aarch64_numbers[op]) return -ENOSYS;
  return nelibc_raw_syscall6(linux_aarch64_numbers[op] - 1, a1, a2, a3,
                             a4, a5, a6);
}

long nelibc_sys3(unsigned long op, unsigned long a1, unsigned long a2,
                 unsigned long a3) {
  return nelibc_sys6(op, a1, a2, a3, 0, 0, 0);
}
