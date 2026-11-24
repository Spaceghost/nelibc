#include <asm/errno.h>
#include <linux/fcntl.h>
#include <linux/mman.h>
#include <asm/poll.h>

#include "../include/nelibc_sys.h"

extern char **environ;

#ifdef NELIBC_ARM_TEST_TLS
static _Thread_local volatile unsigned long initialized = 0x1234;
static _Alignas(128) _Thread_local volatile unsigned long zeroed;
static volatile int stage;

static void preinit(int argc, char **argv, char **envp) {
  if (argc > 0 && argv[0] && envp == environ && initialized == 0x1234 &&
      zeroed == 0 && ((unsigned long)&zeroed & 127) == 0) {
    initialized = 0x5678;
    stage = 1;
  }
}

static void init(int argc, char **argv, char **envp) {
  if (argc > 0 && argv[0] && envp == environ && stage == 1 &&
      initialized == 0x5678 && zeroed == 0) {
    zeroed = 42;
    stage = 2;
  }
}

static void (*const preinit_entry)(int, char **, char **)
    __attribute__((section(".preinit_array"), used)) = preinit;
static void (*const init_entry)(int, char **, char **)
    __attribute__((section(".init_array"), used)) = init;
#endif

int main(int argc, char **argv, char **envp) {
  static const char env[] = "NELIBC_ARM_SMOKE=present";
  static const char ok[] = "Nelibc AArch64 startup and platform operations passed\n";
  static const unsigned long offset = 65536; /* aligned for all ARM64 page sizes */
  char value = 'N';

  if (envp != environ) return 17;
#ifdef NELIBC_ARM_TEST_TLS
  if (stage != 2 || initialized != 0x5678 || zeroed != 42) return 18;
#endif
  if (argc != 2 || argv[1][0] != 'a' || argv[1][1] || argv[2]) return 1;
  while (*envp) {
    unsigned long i = 0;
    while (env[i] && (*envp)[i] == env[i]) ++i;
    if (!env[i] && !(*envp)[i]) break;
    ++envp;
  }
  if (!*envp) return 2;
  if (nelibc_sys3(NELIBC_SYS_GETPID, 0, 0, 0) <= 0 ||
      nelibc_sys3(NELIBC_SYS_CLOSE, -1UL, 0, 0) != -EBADF) return 3;

  long fd = nelibc_sys3(NELIBC_SYS_MEMFD_CREATE, (unsigned long)"nelibc", 0, 0);
  if (fd < 0) return 4;
  if (nelibc_sys6(NELIBC_SYS_PWRITE64, fd, (unsigned long)&value, 1,
                          offset, 0, 0) != 1) return 5;
  /* A nonzero file offset exercises the sixth kernel argument. */
  long address = nelibc_sys6(NELIBC_SYS_MMAP, 0, 1, PROT_READ, MAP_PRIVATE,
                                     fd, offset);
  if ((unsigned long)address >= -4095UL) return 6;
  if (*(const char *)address != value) return 7;
  if (nelibc_sys3(NELIBC_SYS_MUNMAP, address, 1, 0) ||
      nelibc_sys3(NELIBC_SYS_CLOSE, fd, 0, 0)) return 8;
  long directory = nelibc_sys3(NELIBC_SYS_OPEN, (unsigned long)".",
                               O_RDONLY | O_DIRECTORY, 0);
  if (directory < 0 || nelibc_sys3(NELIBC_SYS_CLOSE, directory, 0, 0)) return 9;
  int pipe[2];
  if (nelibc_sys3(NELIBC_SYS_PIPE, (unsigned long)pipe, 0, 0)) return 10;
  struct pollfd ready = {.fd = pipe[0], .events = POLLIN, .revents = 0};
  if (nelibc_sys3(NELIBC_SYS_POLL, (unsigned long)&ready, 1, 0) != 0 ||
      nelibc_sys3(NELIBC_SYS_WRITE, pipe[1], (unsigned long)&value, 1) != 1 ||
      nelibc_sys3(NELIBC_SYS_POLL, (unsigned long)&ready, 1, 50) != 1 ||
      !(ready.revents & POLLIN)) return 11;
  if (nelibc_sys3(NELIBC_SYS_DUP2, pipe[0], pipe[0], 0) != pipe[0] ||
      nelibc_sys3(NELIBC_SYS_DUP2, -1UL, -1UL, 0) != -EBADF ||
      nelibc_sys3(NELIBC_SYS_DUP2, pipe[0], pipe[1], 0) != pipe[1]) return 12;
  if (nelibc_sys3(NELIBC_SYS_CLOSE, pipe[0], 0, 0) ||
      nelibc_sys3(NELIBC_SYS_CLOSE, pipe[1], 0, 0)) return 13;
  if (nelibc_sys3(NELIBC_SYS_COUNT, 0, 0, 0) != -ENOSYS ||
      nelibc_sys3(NELIBC_SYS_FSTAT, 0, 0, 0) != -ENOSYS) return 14;
  long child = nelibc_sys3(NELIBC_SYS_FORK, 0, 0, 0);
  if (child < 0) return 15;
  if (child == 0) return 42;
  int status = 0;
  if (nelibc_sys6(NELIBC_SYS_WAIT4, child, (unsigned long)&status, 0, 0,
                  0, 0) != child || status != (42 << 8)) return 16;
  return nelibc_sys3(NELIBC_SYS_WRITE, 1, (unsigned long)ok, sizeof ok - 1)
         != sizeof ok - 1;
}
