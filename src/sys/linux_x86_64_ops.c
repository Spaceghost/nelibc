/* Linux x86-64 backend for the Nelibc platform operations: each operation
 * names a Linux syscall, so the backend is a number table in front of the
 * raw syscall entry points in linux_x86_64.S. The host profile does not link
 * this file; its dispatcher defines nelibc_sys3/6 itself. */
#include "../../include/nelibc_sys.h"
#include "linux_x86_64_ops.h"

extern long nelibc_raw_syscall3(unsigned long number, unsigned long a1,
                                unsigned long a2, unsigned long a3);
extern long nelibc_raw_syscall6(unsigned long number, unsigned long a1,
                                unsigned long a2, unsigned long a3,
                                unsigned long a4, unsigned long a5,
                                unsigned long a6);

unsigned long nelibc_sys_linux_x86_64_number(unsigned long op) {
  if (op >= NELIBC_SYS_COUNT) return ~0UL;
  return nelibc_sys_linux_x86_64_numbers[op];
}

long nelibc_sys3(unsigned long op, unsigned long a1, unsigned long a2,
                 unsigned long a3) {
  if (op >= NELIBC_SYS_COUNT) return -38; /* ENOSYS */
  return nelibc_raw_syscall3(nelibc_sys_linux_x86_64_numbers[op], a1, a2, a3);
}

long nelibc_sys6(unsigned long op, unsigned long a1, unsigned long a2,
                 unsigned long a3, unsigned long a4, unsigned long a5,
                 unsigned long a6) {
  if (op >= NELIBC_SYS_COUNT) return -38; /* ENOSYS */
  return nelibc_raw_syscall6(nelibc_sys_linux_x86_64_numbers[op], a1, a2, a3,
                             a4, a5, a6);
}
