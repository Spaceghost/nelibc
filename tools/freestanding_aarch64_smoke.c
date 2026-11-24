#include <asm/errno.h>
#include <linux/fcntl.h>
#include <linux/futex.h>
#include <linux/time_types.h>
#include <linux/mman.h>
#include <poll.h>
#include <sys/stat.h>
#include <sys/epoll.h>
#include <sys/select.h>

#include "../include/nelibc_sys.h"
#include "../src/sys/thread_linux_aarch64.h"

extern char **environ;

static int check_select(int fd) {
  fd_set readable;
  if (fd < 0 || fd >= FD_SETSIZE) return 0;
  FD_ZERO(&readable);
  FD_SET(fd, &readable);
  if (nelibc_sys6(NELIBC_SYS_SELECT, fd + 1, (unsigned long)&readable,
                  0, 0, 0, 0) != 1 || !FD_ISSET(fd, &readable)) return 0;

  struct timeval timeout = {.tv_sec = 0, .tv_usec = 1500000};
  if (nelibc_sys6(NELIBC_SYS_SELECT, fd + 1, (unsigned long)&readable,
                  0, 0, (unsigned long)&timeout, 0) != 1 ||
      !FD_ISSET(fd, &readable) || timeout.tv_sec > 1 ||
      timeout.tv_usec < 0 || timeout.tv_usec >= 1000000) return 0;
  timeout = (struct timeval){.tv_usec = 1000};
  if (nelibc_sys6(NELIBC_SYS_SELECT, 0, 0, 0, 0,
                  (unsigned long)&timeout, 0) || timeout.tv_sec || timeout.tv_usec)
    return 0;
  timeout.tv_usec = -1;
  return nelibc_sys6(NELIBC_SYS_SELECT, 0, 0, 0, 0,
                     (unsigned long)&timeout, 0) == -EINVAL && timeout.tv_usec == -1;
}

static int check_epoll(int fd) {
  _Static_assert(sizeof(struct epoll_event) == 16, "AArch64 epoll stride");
  long ep = nelibc_sys3(NELIBC_SYS_EPOLL_CREATE1, 0, 0, 0);
  long duplicate = nelibc_sys3(NELIBC_SYS_DUP, fd, 0, 0);
  if (ep < 0 || duplicate < 0) return 0;
  const uint64_t first = 0x100000002UL, second = 0x300000004UL;
  struct epoll_event entry = {.events = EPOLLIN, .data.u64 = first};
  if (nelibc_sys6(NELIBC_SYS_EPOLL_CTL, ep, EPOLL_CTL_ADD, fd,
                  (unsigned long)&entry, 0, 0)) return 0;
  entry.data.u64 = second;
  if (nelibc_sys6(NELIBC_SYS_EPOLL_CTL, ep, EPOLL_CTL_ADD, duplicate,
                  (unsigned long)&entry, 0, 0)) return 0;
  struct epoll_event received[2];
  if (nelibc_sys6(NELIBC_SYS_EPOLL_WAIT, ep, (unsigned long)received,
                  2, 0, 0, 0) != 2) return 0;
  uint64_t a = received[0].data.u64, b = received[1].data.u64;
  if (!(received[0].events & EPOLLIN) || !(received[1].events & EPOLLIN) ||
      !((a == first && b == second) || (a == second && b == first))) return 0;
  if (nelibc_sys6(NELIBC_SYS_EPOLL_PWAIT, ep, (unsigned long)received,
                  2, 0, 0, 0) != 2) return 0;
  const struct __kernel_timespec timeout = {0};
  long waited = nelibc_sys6(NELIBC_SYS_EPOLL_PWAIT2, ep,
      (unsigned long)received, 2, (unsigned long)&timeout, 0, 0);
  if (waited == -ENOSYS) {
    static const char skipped[] = "epoll_pwait2 unavailable (ENOSYS)\n";
    nelibc_sys3(NELIBC_SYS_WRITE, 1, (unsigned long)skipped, sizeof skipped - 1);
  } else if (waited != 2) {
    return 0;
  }
  return
         !nelibc_sys3(NELIBC_SYS_CLOSE, duplicate, 0, 0) &&
         !nelibc_sys3(NELIBC_SYS_CLOSE, ep, 0, 0);
}

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
struct thread_probe {
  int tid;
  int ready;
  int result;
};

static int worker(void *arg) {
  struct thread_probe *probe = arg;
  while (!__atomic_load_n(&probe->ready, __ATOMIC_ACQUIRE))
    nelibc_sys3(NELIBC_SYS_SCHED_YIELD, 0, 0, 0);
  int valid = initialized == 0x1234 && zeroed == 0 &&
              ((unsigned long)&zeroed & 127) == 0;
  initialized = 0x9999;
  zeroed = 123;
  __atomic_store_n(&probe->result, valid, __ATOMIC_RELEASE);
  return 0;
}

static int check_thread_tls(void) {
  struct nelibc_aarch64_tls tls;
  if (nelibc_aarch64_allocate_tls(&tls)) return 0;
  const unsigned long stack_size = 256 * 1024;
  long stack = nelibc_sys6(NELIBC_SYS_MMAP, 0, stack_size, PROT_READ | PROT_WRITE,
                           MAP_PRIVATE | MAP_ANONYMOUS, -1UL, 0);
  if ((unsigned long)stack >= -4095UL) return 0;
  static struct thread_probe probe;
  long child = nelibc_clone_thread(worker, &probe, (void *)(stack + stack_size),
                                    tls.tp, &probe.tid);
  if (child <= 0 || __atomic_load_n(&probe.tid, __ATOMIC_ACQUIRE) != child)
    return 0;
  __atomic_store_n(&probe.ready, 1, __ATOMIC_RELEASE);
  const struct __kernel_timespec timeout = {.tv_sec = 2};
  for (;;) {
    int running = __atomic_load_n(&probe.tid, __ATOMIC_ACQUIRE);
    if (!running) break;
    long status = nelibc_sys6(NELIBC_SYS_FUTEX, (unsigned long)&probe.tid, FUTEX_WAIT,
                              running, (unsigned long)&timeout, 0, 0);
    if (status != 0 && status != -EAGAIN && status != -EINTR) return 0;
  }
  if (nelibc_sys3(NELIBC_SYS_MUNMAP, stack, stack_size, 0) ||
      nelibc_sys3(NELIBC_SYS_MUNMAP, (unsigned long)tls.mapping, tls.size, 0))
    return 0;
  return __atomic_load_n(&probe.result, __ATOMIC_ACQUIRE) &&
         initialized == 0x5678 && zeroed == 42;
}
#endif

int main(int argc, char **argv, char **envp) {
  static const char env[] = "NELIBC_ARM_SMOKE=present";
  static const char ok[] = "Nelibc AArch64 startup and platform operations passed\n";
  static const unsigned long offset = 65536; /* aligned for all ARM64 page sizes */
  char value = 'N';

  if (envp != environ) return 17;
#ifdef NELIBC_ARM_TEST_TLS
  if (stage != 2 || initialized != 0x5678 || zeroed != 42) return 18;
  if (!check_thread_tls()) return 19;
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
  struct stat metadata;
  if (nelibc_sys3(NELIBC_SYS_FSTAT, fd, (unsigned long)&metadata, 0) ||
      !S_ISREG(metadata.st_mode) || metadata.st_size != (long)offset + 1 ||
      metadata.st_nlink != 0 || metadata.st_blksize <= 0) return 20;
  if (nelibc_sys3(NELIBC_SYS_FSTAT, -1UL, (unsigned long)&metadata, 0) != -EBADF ||
      metadata.st_size != (long)offset + 1 ||
      nelibc_sys3(NELIBC_SYS_FSTAT, fd, 0, 0) != -EFAULT) return 21;
  unsigned long dot = (unsigned long)".";
  if (nelibc_sys3(NELIBC_SYS_STAT, dot, (unsigned long)&metadata, 0) ||
      !S_ISDIR(metadata.st_mode)) return 22;
  unsigned long link = (unsigned long)"link";
  if (nelibc_sys3(NELIBC_SYS_LSTAT, link, (unsigned long)&metadata, 0) ||
      !S_ISLNK(metadata.st_mode) || metadata.st_size != 1) return 23;
  if (nelibc_sys6(NELIBC_SYS_NEWFSTATAT, AT_FDCWD, link,
                  (unsigned long)&metadata, 0, 0, 0) ||
      !S_ISDIR(metadata.st_mode)) return 24;
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
  if (!check_epoll(pipe[0])) return 25;
  if (!check_select(pipe[0])) return 26;
  if (nelibc_sys3(NELIBC_SYS_DUP2, pipe[0], pipe[0], 0) != pipe[0] ||
      nelibc_sys3(NELIBC_SYS_DUP2, -1UL, -1UL, 0) != -EBADF ||
      nelibc_sys3(NELIBC_SYS_DUP2, pipe[0], pipe[1], 0) != pipe[1]) return 12;
  if (nelibc_sys3(NELIBC_SYS_CLOSE, pipe[0], 0, 0) ||
      nelibc_sys3(NELIBC_SYS_CLOSE, pipe[1], 0, 0)) return 13;
  if (nelibc_sys3(NELIBC_SYS_COUNT, 0, 0, 0) != -ENOSYS) return 14;
  long child = nelibc_sys3(NELIBC_SYS_FORK, 0, 0, 0);
  if (child < 0) return 15;
  if (child == 0) return 42;
  int status = 0;
  if (nelibc_sys6(NELIBC_SYS_WAIT4, child, (unsigned long)&status, 0, 0,
                  0, 0) != child || status != (42 << 8)) return 16;
  return nelibc_sys3(NELIBC_SYS_WRITE, 1, (unsigned long)ok, sizeof ok - 1)
         != sizeof ok - 1;
}
