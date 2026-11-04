/* Host profile syscall dispatch. The standalone Nelua modules issue Linux
 * x86-64 syscall numbers through nelibc_raw_syscall3/6; here those numbers
 * are answered from the bound nelibc_host table instead of a `syscall`
 * instruction. Results follow the kernel ABI: a value or a negative errno.
 *
 * Covered: console write, anonymous memory, clocks and sleeps, signal
 * bookkeeping with inline delivery for raise, a self-pipe with poll/select,
 * futex semantics for a single lane, identity and limit queries, and process
 * exit. Everything else returns -ENOSYS after the optional diagnostic hook.
 */
#include "nelibc_host.h"

#define EPERM 1
#define ENOENT 2
#define EBADF 9
#define EAGAIN 11
#define ENOMEM 12
#define EFAULT 14
#define EEXIST 17
#define ENODEV 19
#define EINVAL 22
#define EMFILE 24
#define ENOTTY 25
#define ESPIPE 29
#define EPIPE 32
#define ERANGE 34
#define ENOSYS 38
#define ETIMEDOUT 110
#define ECHILD 10

#define NELIBC_HOST_FD_COUNT 64
#define NELIBC_HOST_PIPE_COUNT 8
#define NELIBC_HOST_PIPE_CAPACITY 65536
#define NELIBC_HOST_SIGNAL_COUNT 65

extern void *nelibc_host_thread_pointer_current(void);
extern void nelibc_host_thread_pointer_record(void *thread_pointer);

enum {
  FD_FREE = 0,
  FD_STDIN,
  FD_CONSOLE,
  FD_PIPE_READ,
  FD_PIPE_WRITE
};

typedef struct host_fd {
  int kind;
  int pipe;
  int status_flags;      /* O_NONBLOCK and friends */
  int descriptor_flags;  /* FD_CLOEXEC */
} host_fd;

typedef struct host_pipe {
  int in_use;
  int readers;
  int writers;
  size_t head;
  size_t length;
  unsigned char buffer[NELIBC_HOST_PIPE_CAPACITY];
} host_pipe;

typedef struct kernel_sigaction {
  void *handler;
  unsigned long flags;
  void *restorer;
  unsigned long mask;
} kernel_sigaction;

typedef struct kernel_timespec {
  long tv_sec;
  long tv_nsec;
} kernel_timespec;

typedef struct kernel_timeval {
  long tv_sec;
  long tv_usec;
} kernel_timeval;

typedef struct kernel_iovec {
  void *base;
  size_t length;
} kernel_iovec;

typedef struct kernel_pollfd {
  int fd;
  short events;
  short revents;
} kernel_pollfd;

typedef struct kernel_stack {
  void *sp;
  int flags;
  size_t size;
} kernel_stack;

typedef struct kernel_rlimit {
  unsigned long current;
  unsigned long maximum;
} kernel_rlimit;

static host_fd fds[NELIBC_HOST_FD_COUNT] = {
  {FD_STDIN, -1, 0, 0}, {FD_CONSOLE, -1, 0, 0}, {FD_CONSOLE, -1, 0, 0}
};
static host_pipe pipes[NELIBC_HOST_PIPE_COUNT];
static kernel_sigaction signal_actions[NELIBC_HOST_SIGNAL_COUNT];
static unsigned long signal_mask;
static kernel_stack signal_stack;
static int unsupported_reported[512 / 32];

static const nelibc_host *table(void) {
  return nelibc_host_current();
}

static void copy_bytes(void *destination, const void *source, size_t length) {
  unsigned char *d = destination;
  const unsigned char *s = source;
  while (length-- > 0) *d++ = *s++;
}

static void zero_bytes(void *destination, size_t length) {
  unsigned char *d = destination;
  while (length-- > 0) *d++ = 0;
}

static void copy_string(char *destination, size_t capacity, const char *text) {
  size_t index = 0;
  while (index + 1 < capacity && text[index] != 0) {
    destination[index] = text[index];
    index++;
  }
  destination[index] = 0;
}

/* Clock helpers ------------------------------------------------------- */

static int monotonic_now(uint64_t *nanoseconds) {
  const nelibc_host *host = table();
  if (host == NULL || host->clock_monotonic == NULL) return -EINVAL;
  if (host->clock_monotonic(host->context, nanoseconds) != 0) return -EINVAL;
  return 0;
}

static int realtime_now(uint64_t *nanoseconds) {
  const nelibc_host *host = table();
  if (host != NULL && host->clock_realtime != NULL &&
      host->clock_realtime(host->context, nanoseconds) == 0)
    return 0;
  return monotonic_now(nanoseconds);
}

static int sleep_for(uint64_t nanoseconds) {
  const nelibc_host *host = table();
  uint64_t start;
  uint64_t now;
  if (host->sleep != NULL) {
    host->sleep(host->context, nanoseconds);
    return 0;
  }
  if (monotonic_now(&start) != 0) return -EINVAL;
  do {
    if (monotonic_now(&now) != 0) return -EINVAL;
  } while (now - start < nanoseconds);
  return 0;
}

static int timespec_valid(const kernel_timespec *value) {
  return value->tv_sec >= 0 && value->tv_nsec >= 0 && value->tv_nsec < 1000000000L;
}

static uint64_t timespec_nanoseconds(const kernel_timespec *value) {
  return (uint64_t)value->tv_sec * 1000000000ull + (uint64_t)value->tv_nsec;
}

static void store_timespec(kernel_timespec *value, uint64_t nanoseconds) {
  value->tv_sec = (long)(nanoseconds / 1000000000ull);
  value->tv_nsec = (long)(nanoseconds % 1000000000ull);
}

static long do_clock_gettime(int clock_id, kernel_timespec *value) {
  uint64_t nanoseconds;
  int result;
  if (value == NULL) return -EFAULT;
  switch (clock_id) {
  case 0: /* CLOCK_REALTIME */
  case 5: /* CLOCK_REALTIME_COARSE */
    result = realtime_now(&nanoseconds);
    break;
  case 1: /* CLOCK_MONOTONIC */
  case 2: /* CLOCK_PROCESS_CPUTIME_ID */
  case 3: /* CLOCK_THREAD_CPUTIME_ID */
  case 4: /* CLOCK_MONOTONIC_RAW */
  case 6: /* CLOCK_MONOTONIC_COARSE */
  case 7: /* CLOCK_BOOTTIME */
    result = monotonic_now(&nanoseconds);
    break;
  default:
    return -EINVAL;
  }
  if (result != 0) return result;
  store_timespec(value, nanoseconds);
  return 0;
}

static long do_nanosleep(const kernel_timespec *requested, kernel_timespec *remaining) {
  int result;
  if (requested == NULL) return -EFAULT;
  if (!timespec_valid(requested)) return -EINVAL;
  result = sleep_for(timespec_nanoseconds(requested));
  if (result == 0 && remaining != NULL) {
    remaining->tv_sec = 0;
    remaining->tv_nsec = 0;
  }
  return result;
}

static long do_clock_nanosleep(int clock_id, int flags,
                               const kernel_timespec *requested,
                               kernel_timespec *remaining) {
  uint64_t now;
  uint64_t target;
  if (requested == NULL) return -EFAULT;
  if (!timespec_valid(requested)) return -EINVAL;
  if (clock_id < 0 || clock_id > 7) return -EINVAL;
  if ((flags & 1) == 0) return do_nanosleep(requested, remaining);
  /* TIMER_ABSTIME */
  if (clock_id == 0 || clock_id == 5) {
    if (realtime_now(&now) != 0) return -EINVAL;
  } else if (monotonic_now(&now) != 0) {
    return -EINVAL;
  }
  target = timespec_nanoseconds(requested);
  if (target <= now) return 0;
  return sleep_for(target - now);
}

/* Descriptors ----------------------------------------------------------- */

static host_fd *fd_entry(long fd) {
  if (fd < 0 || fd >= NELIBC_HOST_FD_COUNT) return NULL;
  if (fds[fd].kind == FD_FREE) return NULL;
  return &fds[fd];
}

static int fd_allocate(int minimum) {
  int fd;
  if (minimum < 0) minimum = 0;
  for (fd = minimum; fd < NELIBC_HOST_FD_COUNT; fd++) {
    if (fds[fd].kind == FD_FREE) return fd;
  }
  return -EMFILE;
}

static int pipe_allocate(void) {
  int index;
  for (index = 0; index < NELIBC_HOST_PIPE_COUNT; index++) {
    if (!pipes[index].in_use) {
      pipes[index].in_use = 1;
      pipes[index].readers = 0;
      pipes[index].writers = 0;
      pipes[index].head = 0;
      pipes[index].length = 0;
      return index;
    }
  }
  return -EMFILE;
}

static void fd_release(long fd) {
  host_fd *entry = fd_entry(fd);
  if (entry == NULL) return;
  if (entry->kind == FD_PIPE_READ) pipes[entry->pipe].readers--;
  if (entry->kind == FD_PIPE_WRITE) pipes[entry->pipe].writers--;
  if ((entry->kind == FD_PIPE_READ || entry->kind == FD_PIPE_WRITE) &&
      pipes[entry->pipe].readers == 0 && pipes[entry->pipe].writers == 0)
    pipes[entry->pipe].in_use = 0;
  entry->kind = FD_FREE;
  entry->pipe = -1;
  entry->status_flags = 0;
  entry->descriptor_flags = 0;
}

static int fd_readable(const host_fd *entry) {
  if (entry->kind == FD_STDIN) return 1; /* immediate EOF */
  if (entry->kind == FD_PIPE_READ)
    return pipes[entry->pipe].length > 0 || pipes[entry->pipe].writers == 0;
  return 0;
}

static int fd_writable(const host_fd *entry) {
  if (entry->kind == FD_CONSOLE) return 1;
  if (entry->kind == FD_PIPE_WRITE)
    return pipes[entry->pipe].length < NELIBC_HOST_PIPE_CAPACITY;
  return 0;
}

static long do_pipe2(int *out, int flags) {
  int read_fd;
  int write_fd;
  int pipe_index;
  if (out == NULL) return -EFAULT;
  if ((flags & ~(0x800 | 0x80000 | 0x4000)) != 0) return -EINVAL;
  pipe_index = pipe_allocate();
  if (pipe_index < 0) return pipe_index;
  read_fd = fd_allocate(0);
  if (read_fd < 0) {
    pipes[pipe_index].in_use = 0;
    return read_fd;
  }
  fds[read_fd].kind = FD_PIPE_READ;
  write_fd = fd_allocate(0);
  if (write_fd < 0) {
    fds[read_fd].kind = FD_FREE;
    pipes[pipe_index].in_use = 0;
    return write_fd;
  }
  fds[write_fd].kind = FD_PIPE_WRITE;
  fds[read_fd].pipe = pipe_index;
  fds[write_fd].pipe = pipe_index;
  fds[read_fd].status_flags = flags & 0x800;
  fds[write_fd].status_flags = flags & 0x800;
  fds[read_fd].descriptor_flags = (flags & 0x80000) ? 1 : 0;
  fds[write_fd].descriptor_flags = (flags & 0x80000) ? 1 : 0;
  pipes[pipe_index].readers = 1;
  pipes[pipe_index].writers = 1;
  out[0] = read_fd;
  out[1] = write_fd;
  return 0;
}

static long pipe_read(host_fd *entry, unsigned char *buffer, size_t length) {
  host_pipe *pipe = &pipes[entry->pipe];
  size_t count = 0;
  if (pipe->length == 0) {
    if (pipe->writers == 0) return 0;
    if (entry->status_flags & 0x800) return -EAGAIN;
    nelibc_host_fail("nelibc host: blocking read on an empty pipe with a single lane\n",
                     NELIBC_HOST_EXIT_DEADLOCK);
  }
  while (count < length && pipe->length > 0) {
    buffer[count++] = pipe->buffer[pipe->head];
    pipe->head = (pipe->head + 1) % NELIBC_HOST_PIPE_CAPACITY;
    pipe->length--;
  }
  return (long)count;
}

static long pipe_write(host_fd *entry, const unsigned char *buffer, size_t length) {
  host_pipe *pipe = &pipes[entry->pipe];
  size_t count = 0;
  if (pipe->readers == 0) return -EPIPE;
  if (pipe->length == NELIBC_HOST_PIPE_CAPACITY) {
    if (entry->status_flags & 0x800) return -EAGAIN;
    nelibc_host_fail("nelibc host: blocking write on a full pipe with a single lane\n",
                     NELIBC_HOST_EXIT_DEADLOCK);
  }
  while (count < length && pipe->length < NELIBC_HOST_PIPE_CAPACITY) {
    pipe->buffer[(pipe->head + pipe->length) % NELIBC_HOST_PIPE_CAPACITY] = buffer[count++];
    pipe->length++;
  }
  return (long)count;
}

static long do_read(long fd, void *buffer, size_t length) {
  host_fd *entry = fd_entry(fd);
  if (entry == NULL) return -EBADF;
  if (length == 0) return 0;
  if (buffer == NULL) return -EFAULT;
  switch (entry->kind) {
  case FD_STDIN: return 0;
  case FD_PIPE_READ: return pipe_read(entry, buffer, length);
  default: return -EBADF;
  }
}

static long do_write(long fd, const void *buffer, size_t length) {
  const nelibc_host *host = table();
  host_fd *entry = fd_entry(fd);
  if (entry == NULL) return -EBADF;
  if (length == 0) return 0;
  if (buffer == NULL) return -EFAULT;
  switch (entry->kind) {
  case FD_CONSOLE: return host->console_write(host->context, (int)fd, buffer, length);
  case FD_PIPE_WRITE: return pipe_write(entry, buffer, length);
  default: return -EBADF;
  }
}

static long do_writev(long fd, const kernel_iovec *vectors, long count) {
  long total = 0;
  long index;
  if (count < 0 || count > 1024) return -EINVAL;
  if (fd_entry(fd) == NULL) return -EBADF;
  for (index = 0; index < count; index++) {
    long written;
    if (vectors[index].length == 0) continue;
    written = do_write(fd, vectors[index].base, vectors[index].length);
    if (written < 0) return total > 0 ? total : written;
    total += written;
    if ((size_t)written < vectors[index].length) break;
  }
  return total;
}

static long do_readv(long fd, const kernel_iovec *vectors, long count) {
  long total = 0;
  long index;
  if (count < 0 || count > 1024) return -EINVAL;
  if (fd_entry(fd) == NULL) return -EBADF;
  for (index = 0; index < count; index++) {
    long got;
    if (vectors[index].length == 0) continue;
    got = do_read(fd, vectors[index].base, vectors[index].length);
    if (got < 0) return total > 0 ? total : got;
    total += got;
    if ((size_t)got < vectors[index].length) break;
  }
  return total;
}

static long do_fcntl(long fd, long command, unsigned long argument) {
  host_fd *entry = fd_entry(fd);
  if (entry == NULL) return -EBADF;
  switch (command) {
  case 0: /* F_DUPFD */
  case 1030: { /* F_DUPFD_CLOEXEC */
    int next = fd_allocate((int)argument);
    if (next < 0) return next;
    fds[next] = *entry;
    fds[next].descriptor_flags = command == 1030 ? 1 : 0;
    if (entry->kind == FD_PIPE_READ) pipes[entry->pipe].readers++;
    if (entry->kind == FD_PIPE_WRITE) pipes[entry->pipe].writers++;
    return next;
  }
  case 1: return entry->descriptor_flags; /* F_GETFD */
  case 2: entry->descriptor_flags = (int)(argument & 1); return 0; /* F_SETFD */
  case 3: { /* F_GETFL */
    int access = entry->kind == FD_CONSOLE || entry->kind == FD_PIPE_WRITE ? 1 : 0;
    return access | entry->status_flags;
  }
  case 4: entry->status_flags = (int)(argument & (0x800 | 0x400 | 0x2000 | 0x4000)); return 0; /* F_SETFL */
  case 1032: /* F_GETPIPE_SZ */
    if (entry->kind == FD_PIPE_READ || entry->kind == FD_PIPE_WRITE) return NELIBC_HOST_PIPE_CAPACITY;
    return -EBADF;
  default: return -EINVAL;
  }
}

static long do_dup3(long old_fd, long new_fd, int flags) {
  host_fd *entry = fd_entry(old_fd);
  if (entry == NULL) return -EBADF;
  if (new_fd < 0 || new_fd >= NELIBC_HOST_FD_COUNT) return -EBADF;
  if (old_fd == new_fd) return -EINVAL;
  if (fds[new_fd].kind != FD_FREE) fd_release(new_fd);
  fds[new_fd] = *entry;
  fds[new_fd].descriptor_flags = (flags & 0x80000) ? 1 : 0;
  if (entry->kind == FD_PIPE_READ) pipes[entry->pipe].readers++;
  if (entry->kind == FD_PIPE_WRITE) pipes[entry->pipe].writers++;
  return new_fd;
}

/* poll and select -------------------------------------------------------- */

static long wait_or_fail(long ready, int has_timeout, uint64_t timeout_nanoseconds) {
  if (ready > 0) return ready;
  if (!has_timeout)
    nelibc_host_fail("nelibc host: indefinite wait with nothing ready on a single lane\n",
                     NELIBC_HOST_EXIT_DEADLOCK);
  if (timeout_nanoseconds > 0) {
    int result = sleep_for(timeout_nanoseconds);
    if (result != 0) return result;
  }
  return 0;
}

static long do_poll(kernel_pollfd *entries, unsigned long count,
                    int has_timeout, uint64_t timeout_nanoseconds) {
  long ready = 0;
  unsigned long index;
  if (count > 1024) return -EINVAL;
  if (count > 0 && entries == NULL) return -EFAULT;
  for (index = 0; index < count; index++) {
    kernel_pollfd *item = &entries[index];
    host_fd *entry;
    item->revents = 0;
    if (item->fd < 0) continue;
    entry = fd_entry(item->fd);
    if (entry == NULL) {
      item->revents = 0x20; /* POLLNVAL */
      ready++;
      continue;
    }
    if ((item->events & 0x1) && fd_readable(entry)) item->revents |= 0x1;
    if ((item->events & 0x4) && fd_writable(entry)) item->revents |= 0x4;
    if (entry->kind == FD_PIPE_READ && pipes[entry->pipe].writers == 0) item->revents |= 0x10;
    if (entry->kind == FD_PIPE_WRITE && pipes[entry->pipe].readers == 0) item->revents |= 0x8;
    if (item->revents != 0) ready++;
  }
  return wait_or_fail(ready, has_timeout, timeout_nanoseconds);
}

static int fdset_test(const unsigned long *set, long fd) {
  return set != NULL && (set[fd / 64] >> (fd % 64)) & 1;
}

static long do_select(long nfds, unsigned long *reads, unsigned long *writes,
                      unsigned long *excepts, int has_timeout,
                      uint64_t timeout_nanoseconds) {
  unsigned long ready_reads[16] = {0};
  unsigned long ready_writes[16] = {0};
  long ready = 0;
  long fd;
  if (nfds < 0 || nfds > 1024) return -EINVAL;
  for (fd = 0; fd < nfds; fd++) {
    host_fd *entry;
    int wants_read = fdset_test(reads, fd);
    int wants_write = fdset_test(writes, fd);
    if (!wants_read && !wants_write && !fdset_test(excepts, fd)) continue;
    entry = fd_entry(fd);
    if (entry == NULL) return -EBADF;
    if (wants_read && fd_readable(entry)) {
      ready_reads[fd / 64] |= 1ul << (fd % 64);
      ready++;
    }
    if (wants_write && fd_writable(entry)) {
      ready_writes[fd / 64] |= 1ul << (fd % 64);
      ready++;
    }
  }
  ready = wait_or_fail(ready, has_timeout, timeout_nanoseconds);
  if (ready < 0) return ready;
  if (reads != NULL) copy_bytes(reads, ready_reads, sizeof ready_reads[0] * ((nfds + 63) / 64));
  if (writes != NULL) copy_bytes(writes, ready_writes, sizeof ready_writes[0] * ((nfds + 63) / 64));
  if (excepts != NULL) zero_bytes(excepts, sizeof(unsigned long) * ((nfds + 63) / 64));
  return ready;
}

/* Signals --------------------------------------------------------------- */

static long do_rt_sigaction(long signal, const kernel_sigaction *action,
                            kernel_sigaction *old_action, size_t set_size) {
  if (signal <= 0 || signal >= NELIBC_HOST_SIGNAL_COUNT) return -EINVAL;
  if (set_size != 8) return -EINVAL;
  if ((signal == 9 || signal == 19) && action != NULL) return -EINVAL;
  if (old_action != NULL) *old_action = signal_actions[signal];
  if (action != NULL) signal_actions[signal] = *action;
  return 0;
}

static long do_rt_sigprocmask(long how, const unsigned long *set,
                              unsigned long *old_set, size_t set_size) {
  if (set_size != 8) return -EINVAL;
  if (old_set != NULL) *old_set = signal_mask;
  if (set == NULL) return 0;
  switch (how) {
  case 0: signal_mask |= *set; break;
  case 1: signal_mask &= ~*set; break;
  case 2: signal_mask = *set; break;
  default: return -EINVAL;
  }
  signal_mask &= ~((1ul << 8) | (1ul << 18)); /* SIGKILL, SIGSTOP */
  return 0;
}

static long do_sigaltstack(const kernel_stack *stack, kernel_stack *old_stack) {
  if (old_stack != NULL) {
    *old_stack = signal_stack;
    if (signal_stack.sp == NULL) old_stack->flags = 2; /* SS_DISABLE */
  }
  if (stack != NULL) {
    if (stack->flags & 2) {
      signal_stack.sp = NULL;
      signal_stack.size = 0;
      signal_stack.flags = 2;
    } else {
      if (stack->size < 2048) return -ENOMEM;
      signal_stack = *stack;
    }
  }
  return 0;
}

static int signal_default_ignored(long signal) {
  return signal == 17 || signal == 23 || signal == 28 || signal == 18;
}

/* Deliver a thread-directed signal inline: there is no other lane to
 * interrupt, so raise() is a direct call and a default terminating action
 * stops the host with the conventional 128 + signal status. */
static long deliver_signal(long signal) {
  kernel_sigaction *action;
  void *handler;
  if (signal == 0) return 0;
  if (signal < 0 || signal >= NELIBC_HOST_SIGNAL_COUNT) return -EINVAL;
  action = &signal_actions[signal];
  handler = action->handler;
  if ((unsigned long)handler == 1ul) return 0; /* SIG_IGN */
  if (handler == NULL) {
    if (signal_default_ignored(signal)) return 0;
    nelibc_host_fail("nelibc host: terminating signal with the default action\n",
                     128 + (int)signal);
  }
  if (action->flags & 0x80000000ul) action->handler = NULL; /* SA_RESETHAND */
  if (action->flags & 4) { /* SA_SIGINFO */
    unsigned char info[128];
    zero_bytes(info, sizeof info);
    *(int *)info = (int)signal;
    ((void (*)(int, void *, void *))handler)((int)signal, info, NULL);
  } else {
    ((void (*)(int))handler)((int)signal);
  }
  return 0;
}

/* Faults ---------------------------------------------------------------- */

typedef struct fault_call {
  void *handler;
  int signal;
  int siginfo;
  unsigned char info[128];
} fault_call;

static void fault_invoke(void *argument) {
  fault_call *call = argument;
  if (call->siginfo)
    ((void (*)(int, void *, void *))call->handler)(call->signal, call->info, NULL);
  else
    ((void (*)(int))call->handler)(call->signal);
}

/* Run function(argument) with the stack pointer moved to stack_top, then
 * return to the original stack. rbx keeps the old stack pointer across the
 * call; everything the SysV ABI lets a callee clobber is declared. */
static void call_on_stack(void *stack_top, void (*function)(void *), void *argument) {
  __asm__ volatile(
      "movq %%rsp, %%rbx\n\t"
      "movq %%rdx, %%rsp\n\t"
      "andq $-16, %%rsp\n\t"
      "call *%%rax\n\t"
      "movq %%rbx, %%rsp\n\t"
      : "+a"(function), "+d"(stack_top), "+D"(argument)
      :
      : "rbx", "rcx", "rsi", "r8", "r9", "r10", "r11", "memory", "cc",
        "xmm0", "xmm1", "xmm2", "xmm3", "xmm4", "xmm5", "xmm6", "xmm7",
        "xmm8", "xmm9", "xmm10", "xmm11", "xmm12", "xmm13", "xmm14", "xmm15");
}

int nelibc_host_fault(int signal, int code, void *address) {
  kernel_sigaction *action;
  void *handler;
  fault_call call;
  if (signal <= 0 || signal >= NELIBC_HOST_SIGNAL_COUNT) return -EINVAL;
  action = &signal_actions[signal];
  handler = action->handler;
  if (handler == NULL || (unsigned long)handler == 1ul)
    nelibc_host_fail("nelibc host: fault with no handler, stopping\n", 128 + signal);
  if (action->flags & 0x80000000ul) action->handler = NULL; /* SA_RESETHAND */
  call.handler = handler;
  call.signal = signal;
  call.siginfo = (action->flags & 4) != 0; /* SA_SIGINFO */
  zero_bytes(call.info, sizeof call.info);
  *(int *)call.info = signal;           /* si_signo */
  *(int *)(call.info + 8) = code;       /* si_code */
  copy_bytes(call.info + 16, &address, sizeof address); /* si_addr */
  if ((action->flags & 0x08000000ul) && signal_stack.sp != NULL &&
      !(signal_stack.flags & 2)) { /* SA_ONSTACK with a usable stack */
    char *base = signal_stack.sp;
    char *current = (char *)__builtin_frame_address(0);
    if (current < base || current >= base + signal_stack.size) {
      call_on_stack(base + signal_stack.size, fault_invoke, &call);
      return 0;
    }
  }
  fault_invoke(&call);
  return 0;
}

/* Memory ---------------------------------------------------------------- */

static long do_mmap(void *address, size_t length, long protection, long flags,
                    long fd, long offset) {
  const nelibc_host *host = table();
  void *result;
  (void)address;
  (void)protection;
  if (length == 0) return -EINVAL;
  if (offset != 0) return -EINVAL;
  if ((flags & 0x20) == 0 || fd != -1) return -ENODEV; /* file mappings */
  if (flags & 0x10) return -ENOMEM;                      /* MAP_FIXED */
  length = (length + 4095) & ~(size_t)4095;
  result = host->memory_reserve(host->context, length, 4096);
  if (result == NULL) return -ENOMEM;
  return (long)(uintptr_t)result;
}

static long do_mprotect(void *address, size_t length, long protection) {
  const nelibc_host *host = table();
  if (((uintptr_t)address & 4095) != 0) return -EINVAL;
  if (host->memory_protect == NULL) return 0;
  return host->memory_protect(host->context, address, length, (int)protection);
}

static long do_munmap(void *address, size_t length) {
  const nelibc_host *host = table();
  if (((uintptr_t)address & 4095) != 0) return -EINVAL;
  if (length == 0) return -EINVAL;
  if (host->memory_release == NULL) return 0;
  return host->memory_release(host->context, address, length);
}

static long do_mremap(void *old_address, size_t old_size, size_t new_size,
                      long flags) {
  const nelibc_host *host = table();
  void *result;
  if (((uintptr_t)old_address & 4095) != 0) return -EINVAL;
  if (new_size == 0) return -EINVAL;
  new_size = (new_size + 4095) & ~(size_t)4095;
  if (new_size <= old_size) return (long)(uintptr_t)old_address;
  if ((flags & 1) == 0) return -ENOMEM; /* MREMAP_MAYMOVE required */
  result = host->memory_reserve(host->context, new_size, 4096);
  if (result == NULL) return -ENOMEM;
  copy_bytes(result, old_address, old_size);
  if (host->memory_release != NULL)
    host->memory_release(host->context, old_address, old_size);
  return (long)(uintptr_t)result;
}

/* Futex for one lane ------------------------------------------------------ */

static long do_futex(int *address, long operation, int value,
                     const kernel_timespec *timeout) {
  long op = operation & 0x7f;
  if (address == NULL) return -EFAULT;
  switch (op) {
  case 0: /* FUTEX_WAIT */
  case 9: /* FUTEX_WAIT_BITSET */
    if (__atomic_load_n(address, __ATOMIC_ACQUIRE) != value) return -EAGAIN;
    if (timeout != NULL) {
      uint64_t now;
      uint64_t target;
      if (!timespec_valid(timeout)) return -EINVAL;
      target = timespec_nanoseconds(timeout);
      if (op == 9) { /* absolute */
        int result = (operation & 256) ? realtime_now(&now) : monotonic_now(&now);
        if (result != 0) return result;
        if (target > now) sleep_for(target - now);
      } else {
        sleep_for(target);
      }
      return -ETIMEDOUT;
    }
    nelibc_host_fail("nelibc host: futex wait that no other lane can wake\n",
                     NELIBC_HOST_EXIT_DEADLOCK);
    return -EAGAIN;
  case 1:  /* FUTEX_WAKE */
  case 10: /* FUTEX_WAKE_BITSET */
  case 5:  /* FUTEX_WAKE_OP */
    return 0;
  case 3: /* FUTEX_REQUEUE */
    return 0;
  case 4: /* FUTEX_CMP_REQUEUE */
    return 0;
  case 6: /* FUTEX_LOCK_PI */
  case 13: /* FUTEX_LOCK_PI2 */
    if (__atomic_load_n(address, __ATOMIC_ACQUIRE) == 0) {
      __atomic_store_n(address, 1, __ATOMIC_RELEASE);
      return 0;
    }
    if (timeout != NULL) return -ETIMEDOUT;
    nelibc_host_fail("nelibc host: priority-inheritance lock held by the only lane\n",
                     NELIBC_HOST_EXIT_DEADLOCK);
    return -EAGAIN;
  case 7: /* FUTEX_UNLOCK_PI */
    __atomic_store_n(address, 0, __ATOMIC_RELEASE);
    return 0;
  case 8: /* FUTEX_TRYLOCK_PI */
    if (__atomic_load_n(address, __ATOMIC_ACQUIRE) == 0) {
      __atomic_store_n(address, 1, __ATOMIC_RELEASE);
      return 0;
    }
    return -EAGAIN;
  default:
    return -ENOSYS;
  }
}

/* Identity, limits, and misc -------------------------------------------- */

static int host_cpu_count(void) {
  const nelibc_host *host = table();
  int count = host != NULL ? host->cpu_count : 0;
  if (count < 1) count = 1;
  if (count > 1024) count = 1024;
  return count;
}

static long do_sched_getaffinity(size_t length, unsigned long *mask) {
  int count = host_cpu_count();
  int cpu;
  if (mask == NULL) return -EFAULT;
  if (length < 8) return -EINVAL;
  if (length > 128) length = 128;
  zero_bytes(mask, length);
  for (cpu = 0; cpu < count && (size_t)cpu / 8 < length; cpu++)
    mask[cpu / 64] |= 1ul << (cpu % 64);
  return (long)((count + 63) / 64 * 8);
}

static long fill_rlimit(long resource, kernel_rlimit *limit) {
  const unsigned long infinity = ~0ul;
  if (limit == NULL) return 0;
  switch (resource) {
  case 3: limit->current = 8ul << 20; limit->maximum = infinity; break;   /* STACK */
  case 7: limit->current = NELIBC_HOST_FD_COUNT; limit->maximum = NELIBC_HOST_FD_COUNT; break; /* NOFILE */
  case 6: limit->current = 1; limit->maximum = 1; break;                   /* NPROC */
  case 11: limit->current = 0; limit->maximum = 0; break;                  /* SIGPENDING */
  case 12: limit->current = 0; limit->maximum = 0; break;                  /* MSGQUEUE */
  default:
    if (resource < 0 || resource > 15) return -EINVAL;
    limit->current = infinity;
    limit->maximum = infinity;
    break;
  }
  return 0;
}

static long do_uname(char *buffer) {
  if (buffer == NULL) return -EFAULT;
  zero_bytes(buffer, 390);
  copy_string(buffer + 0 * 65, 65, "Linux");
  copy_string(buffer + 1 * 65, 65, "nelibc-host");
  copy_string(buffer + 2 * 65, 65, "0.0.0-nelibc-host");
  copy_string(buffer + 3 * 65, 65, "#1 single-lane");
  copy_string(buffer + 4 * 65, 65, "x86_64");
  return 0;
}

static void report_unsupported(unsigned long number) {
  const nelibc_host *host = table();
  if (number < 512) {
    unsigned int word = (unsigned int)(number / 32);
    unsigned int bit = 1u << (number % 32);
    if (unsupported_reported[word] & bit) return;
    unsupported_reported[word] |= bit;
  }
  if (host != NULL && host->unsupported != NULL)
    host->unsupported(host->context, number);
}

/* Dispatch -------------------------------------------------------------- */

long nelibc_raw_syscall6(unsigned long number, unsigned long a1,
                         unsigned long a2, unsigned long a3, unsigned long a4,
                         unsigned long a5, unsigned long a6) {
  const nelibc_host *host = table();
  if (host == NULL) {
    if (number == 60 || number == 231) {
      for (;;) {
      }
    }
    return -ENOSYS;
  }
  switch (number) {
  case 0: return do_read((long)a1, (void *)a2, a3);
  case 1: return do_write((long)a1, (const void *)a2, a3);
  case 2: return -ENOENT;                                      /* open */
  case 3: /* close */
    if (fd_entry((long)a1) == NULL) return -EBADF;
    fd_release((long)a1);
    return 0;
  case 4: case 5: case 6: return -ENOENT;                      /* stat */
  case 7: return do_poll((kernel_pollfd *)a1, a2, (int)a3 >= 0,
                         (int)a3 > 0 ? (uint64_t)(int)a3 * 1000000ull : 0);
  case 8: return -ESPIPE;                                      /* lseek */
  case 9: return do_mmap((void *)a1, a2, (long)a3, (long)a4, (long)a5, (long)a6);
  case 10: return do_mprotect((void *)a1, a2, (long)a3);
  case 11: return do_munmap((void *)a1, a2);
  case 12: return -ENOMEM;                                     /* brk */
  case 13: return do_rt_sigaction((long)a1, (const kernel_sigaction *)a2,
                                  (kernel_sigaction *)a3, a4);
  case 14: return do_rt_sigprocmask((long)a1, (const unsigned long *)a2,
                                    (unsigned long *)a3, a4);
  case 16: return fd_entry((long)a1) == NULL ? -EBADF : -ENOTTY; /* ioctl */
  case 17: case 18: return -ESPIPE;                            /* pread, pwrite */
  case 19: return do_readv((long)a1, (const kernel_iovec *)a2, (long)a3);
  case 20: return do_writev((long)a1, (const kernel_iovec *)a2, (long)a3);
  case 21: return -ENOENT;                                     /* access */
  case 22: return do_pipe2((int *)a1, 0);
  case 23: {
    const kernel_timeval *timeout = (const kernel_timeval *)a5;
    uint64_t nanoseconds = 0;
    if (timeout != NULL) {
      if (timeout->tv_sec < 0 || timeout->tv_usec < 0 || timeout->tv_usec >= 1000000) return -EINVAL;
      nanoseconds = (uint64_t)timeout->tv_sec * 1000000000ull + (uint64_t)timeout->tv_usec * 1000ull;
    }
    return do_select((long)a1, (unsigned long *)a2, (unsigned long *)a3,
                     (unsigned long *)a4, timeout != NULL, nanoseconds);
  }
  case 24: return 0;                                           /* sched_yield */
  case 25: return do_mremap((void *)a1, a2, a3, (long)a4);
  case 26: return 0;                                           /* msync */
  case 28: return 0;                                           /* madvise */
  case 32: return do_fcntl((long)a1, 0, 0);                    /* dup */
  case 33: return a1 == a2 ? (fd_entry((long)a1) == NULL ? -EBADF : (long)a2)
                           : do_dup3((long)a1, (long)a2, 0);
  case 35: return do_nanosleep((const kernel_timespec *)a1, (kernel_timespec *)a2);
  case 37: return 0;                                           /* alarm */
  case 39: return 1;                                           /* getpid */
  case 56: return -ENOSYS;                                     /* clone */
  case 57: case 58: case 59: return -ENOSYS;                   /* fork, vfork, execve */
  case 60: case 231:                                           /* exit, exit_group */
    host->exit(host->context, (int)a1);
    for (;;) {
    }
  case 61: return -ECHILD;                                   /* wait4 */
  case 62: return deliver_signal((long)a2);                    /* kill */
  case 63: return do_uname((char *)a1);
  case 72: return do_fcntl((long)a1, (long)a2, a3);
  case 79: /* getcwd */
    if (a2 < 2) return -ERANGE;
    copy_string((char *)a1, a2, "/");
    return 2;
  case 89: case 267: return -ENOENT;                           /* readlink */
  case 95: return 022;                                         /* umask */
  case 96: { /* gettimeofday */
    kernel_timeval *value = (kernel_timeval *)a1;
    uint64_t nanoseconds;
    int result;
    if (value == NULL) return 0;
    result = realtime_now(&nanoseconds);
    if (result != 0) return result;
    value->tv_sec = (long)(nanoseconds / 1000000000ull);
    value->tv_usec = (long)(nanoseconds % 1000000000ull / 1000ull);
    return 0;
  }
  case 97: return fill_rlimit((long)a1, (kernel_rlimit *)a2);  /* getrlimit */
  case 98: if (a2 != 0) zero_bytes((void *)a2, 144); return 0;   /* getrusage */
  case 99: return -ENOSYS;                                     /* sysinfo */
  case 100: { /* times */
    uint64_t nanoseconds = 0;
    if (a1 != 0) zero_bytes((void *)a1, 32);
    if (monotonic_now(&nanoseconds) != 0) return 0;
    return (long)(nanoseconds / 10000000ull);
  }
  case 102: case 104: case 107: case 108: return 0;            /* uid, gid */
  case 109: case 110: case 111: case 121: return 1;            /* setpgid, getppid, getpgrp, getpgid */
  case 112: return 1;                                          /* setsid */
  case 131: return do_sigaltstack((const kernel_stack *)a1, (kernel_stack *)a2);
  case 143: if (a2 != 0) zero_bytes((void *)a2, 4); return 0;  /* sched_getparam */
  case 144: return (long)a2 == 0 ? 0 : -EPERM;                 /* sched_setscheduler */
  case 145: return 0;                                          /* sched_getscheduler */
  case 146: return (long)a1 == 1 || (long)a1 == 2 ? 99 : 0;    /* priority max */
  case 147: return (long)a1 == 1 || (long)a1 == 2 ? 1 : 0;     /* priority min */
  case 157: /* prctl */
    if ((long)a1 == 15) return 0;                              /* PR_SET_NAME */
    if ((long)a1 == 16) { copy_string((char *)a2, 16, "nelibc"); return 0; }
    return -EINVAL;
  case 158: /* arch_prctl */
    if (a1 == 0x1002) {
      int result = host->thread_pointer_set(host->context, (void *)a2);
      if (result == 0) nelibc_host_thread_pointer_record((void *)a2);
      return result;
    }
    if (a1 == 0x1003) {
      if (a2 == 0) return -EFAULT;
      *(void **)a2 = nelibc_host_thread_pointer_current();
      return 0;
    }
    return -EINVAL;
  case 186: return 1;                                          /* gettid */
  case 200: return deliver_signal((long)a2);                   /* tkill */
  case 201: { /* time */
    uint64_t nanoseconds;
    int result = realtime_now(&nanoseconds);
    if (result != 0) return result;
    if (a1 != 0) *(long *)a1 = (long)(nanoseconds / 1000000000ull);
    return (long)(nanoseconds / 1000000000ull);
  }
  case 202: return do_futex((int *)a1, (long)a2, (int)a3, (const kernel_timespec *)a4);
  case 203: return 0;                                          /* sched_setaffinity */
  case 204: return do_sched_getaffinity(a2, (unsigned long *)a3);
  case 218: return 1;                                          /* set_tid_address */
  case 228: return do_clock_gettime((int)a1, (kernel_timespec *)a2);
  case 229: { /* clock_getres */
    kernel_timespec *value = (kernel_timespec *)a2;
    if ((long)a1 < 0 || (long)a1 > 7) return -EINVAL;
    if (value != NULL) { value->tv_sec = 0; value->tv_nsec = 1; }
    return 0;
  }
  case 230: return do_clock_nanosleep((int)a1, (int)a2, (const kernel_timespec *)a3,
                                      (kernel_timespec *)a4);
  case 234: return deliver_signal((long)a3);                   /* tgkill */
  case 257: return -ENOENT;                                    /* openat */
  case 262: case 332: return -ENOENT;                          /* newfstatat, statx */
  case 269: return -ENOENT;                                    /* faccessat */
  case 270: { /* pselect6 */
    const kernel_timespec *timeout = (const kernel_timespec *)a5;
    if (timeout != NULL && !timespec_valid(timeout)) return -EINVAL;
    return do_select((long)a1, (unsigned long *)a2, (unsigned long *)a3,
                     (unsigned long *)a4, timeout != NULL,
                     timeout != NULL ? timespec_nanoseconds(timeout) : 0);
  }
  case 271: { /* ppoll */
    const kernel_timespec *timeout = (const kernel_timespec *)a3;
    if (timeout != NULL && !timespec_valid(timeout)) return -EINVAL;
    return do_poll((kernel_pollfd *)a1, a2, timeout != NULL,
                   timeout != NULL ? timespec_nanoseconds(timeout) : 0);
  }
  case 273: return 0;                                          /* set_robust_list */
  case 292: return do_dup3((long)a1, (long)a2, (int)a3);
  case 293: return do_pipe2((int *)a1, (int)a2);
  case 302: /* prlimit64 */
    if (a3 != 0) {
      const kernel_rlimit *requested = (const kernel_rlimit *)a3;
      if (requested->current > requested->maximum) return -EINVAL;
    }
    return fill_rlimit((long)a2, (kernel_rlimit *)a4);
  case 318: return -ENOSYS;                                    /* getrandom */
  default:
    report_unsupported(number);
    return -ENOSYS;
  }
}

long nelibc_raw_syscall3(unsigned long number, unsigned long a1,
                         unsigned long a2, unsigned long a3) {
  return nelibc_raw_syscall6(number, a1, a2, a3, 0, 0, 0);
}
