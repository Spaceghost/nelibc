#include <asm/errno.h>
#include <asm/unistd.h>
#include <linux/mman.h>

extern long nelibc_raw_syscall3(unsigned long number, unsigned long arg1,
                                unsigned long arg2, unsigned long arg3);
extern long nelibc_raw_syscall6(unsigned long number, unsigned long arg1,
                                unsigned long arg2, unsigned long arg3,
                                unsigned long arg4, unsigned long arg5,
                                unsigned long arg6);

int main(int argc, char **argv, char **envp) {
  static const char env[] = "NELIBC_ARM_SMOKE=present";
  static const char ok[] = "Nelibc AArch64 startup and raw syscalls passed\n";
  static const unsigned long offset = 65536; /* aligned for all ARM64 page sizes */
  char value = 'N';

  if (argc != 2 || argv[1][0] != 'a' || argv[1][1] || argv[2]) return 1;
  while (*envp) {
    unsigned long i = 0;
    while (env[i] && (*envp)[i] == env[i]) ++i;
    if (!env[i] && !(*envp)[i]) break;
    ++envp;
  }
  if (!*envp) return 2;
  if (nelibc_raw_syscall3(__NR_getpid, 0, 0, 0) <= 0 ||
      nelibc_raw_syscall3(__NR_close, -1UL, 0, 0) != -EBADF) return 3;

  long fd = nelibc_raw_syscall3(__NR_memfd_create, (unsigned long)"nelibc", 0, 0);
  if (fd < 0) return 4;
  if (nelibc_raw_syscall6(__NR_pwrite64, fd, (unsigned long)&value, 1,
                          offset, 0, 0) != 1) return 5;
  /* A nonzero file offset exercises the sixth kernel argument. */
  long address = nelibc_raw_syscall6(__NR_mmap, 0, 1, PROT_READ, MAP_PRIVATE,
                                     fd, offset);
  if ((unsigned long)address >= -4095UL) return 6;
  if (*(const char *)address != value) return 7;
  if (nelibc_raw_syscall3(__NR_munmap, address, 1, 0) ||
      nelibc_raw_syscall3(__NR_close, fd, 0, 0)) return 8;
  return nelibc_raw_syscall3(__NR_write, 1, (unsigned long)ok, sizeof ok - 1)
         != sizeof ok - 1;
}
