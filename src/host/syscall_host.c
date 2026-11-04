/* Host profile operation dispatch. The standalone Nelua modules and C
 * adapters ask for platform operations by NELIBC_SYS_* number through
 * nelibc_sys3/6 (include/nelibc_sys.h); here those operations are answered
 * from the bound nelibc_host table. No Linux syscall number and no
 * `syscall` instruction is involved. Results follow the convention the
 * modules were written against: a value or a negative errno.
 *
 * Covered: console write, anonymous memory, clocks and sleeps, signal
 * bookkeeping with inline delivery for raise, a self-pipe with poll/select,
 * cooperative lanes behind clone/futex/exit, identity and limit queries, and
 * process exit. Everything else returns -ENOSYS after the optional
 * diagnostic hook.
 *
 * Lanes: every thread the program creates is a lane on the one hardware
 * thread the host supplies. A lane runs until it blocks (futex wait, sleep,
 * empty pipe, poll with nothing ready), yields, or exits; the dispatcher then
 * switches the stack and thread pointer to the next runnable lane. Nothing
 * preempts a lane, so a spinning loop that never enters the dispatcher
 * starves the others, exactly as on a one-CPU kernel without a timer tick.
 */
#include "nelibc_host.h"
#include "nelibc_sys.h"

#define EPERM 1
#define ENOENT 2
#define ESRCH 3
#define EINTR 4
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
#define EDEADLK 35
#define ENOSYS 38
#define ETIMEDOUT 110
#define ECHILD 10

#define NELIBC_HOST_LANE_COUNT 64
#define NELIBC_HOST_FD_COUNT 64
#define NELIBC_HOST_PIPE_COUNT 8
#define NELIBC_HOST_PIPE_CAPACITY 65536
#define NELIBC_HOST_SIGNAL_COUNT 65

extern void *nelibc_host_thread_area_current(void);
extern void nelibc_host_thread_area_record(void *thread_pointer);

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

/* Stop the hardware thread for a while: the host's sleep, or a spin on its
 * clock. Used only when no lane can run in the meantime. */
static int host_sleep_raw(uint64_t nanoseconds) {
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

/* Lanes ----------------------------------------------------------------- */

enum {
  LANE_FREE = 0,
  LANE_RUNNABLE,
  LANE_BLOCKED,
  LANE_EXITED
};

typedef struct host_lane {
  void *sp;                 /* saved stack pointer while not running */
  void *thread_pointer;     /* FS base installed when the lane runs */
  int tid;
  int state;
  int *wait_address;        /* futex word the lane sleeps on, or NULL */
  int has_deadline;
  uint64_t deadline;        /* monotonic nanoseconds */
  long wake_result;         /* what the blocking call returns */
  int pi_handoff;           /* FUTEX_UNLOCK_PI gave this lane the lock */
  int *clear_tid;           /* CLONE_CHILD_CLEARTID / set_tid_address */
  void *robust_list;        /* set_robust_list head */
  unsigned long pending_signals;
  int (*start)(void *);
  void *argument;
  void *release[3];         /* regions to unmap once the lane is off them */
  size_t release_size[3];
  unsigned long mask;       /* rt_sigprocmask, inherited at clone */
  kernel_stack altstack;    /* sigaltstack, per lane as on Linux */
} host_lane;

static host_lane lanes[NELIBC_HOST_LANE_COUNT];
static host_lane *current_lane;
static host_lane *switched_from;
static int next_tid = 2;
static unsigned long lanes_created;
static unsigned long lane_switches;

void nelibc_host_lane_stats(unsigned long *created, unsigned long *switches) {
  if (created != NULL) *created = lanes_created;
  if (switches != NULL) *switches = lane_switches;
}

/* Save callee-saved registers and the stack pointer of the running lane,
 * load the other lane's, and return into it. A fresh lane's stack holds six
 * zero slots and lane_entry as the return address. */
extern void host_lane_switch(void **save_sp, void *load_sp);
__asm__(".text\n"
        ".globl host_lane_switch\n"
        ".type host_lane_switch,@function\n"
        "host_lane_switch:\n"
        "  pushq %rbp\n"
        "  pushq %rbx\n"
        "  pushq %r12\n"
        "  pushq %r13\n"
        "  pushq %r14\n"
        "  pushq %r15\n"
        "  movq %rsp, (%rdi)\n"
        "  movq %rsi, %rsp\n"
        "  popq %r15\n"
        "  popq %r14\n"
        "  popq %r13\n"
        "  popq %r12\n"
        "  popq %rbx\n"
        "  popq %rbp\n"
        "  ret\n"
        ".size host_lane_switch, .-host_lane_switch\n");

static host_lane *lane_current(void) {
  if (current_lane == NULL) {
    lanes[0].state = LANE_RUNNABLE;
    lanes[0].tid = 1;
    lanes[0].thread_pointer = nelibc_host_thread_area_current();
    current_lane = &lanes[0];
  }
  return current_lane;
}

/* The signal mask and alternate stack are per lane, as per thread on Linux. */
#define signal_mask (lane_current()->mask)
#define signal_stack (lane_current()->altstack)

static host_lane *lane_by_tid(int tid) {
  unsigned index;
  if (tid <= 0) return NULL;
  for (index = 0; index < NELIBC_HOST_LANE_COUNT; index++)
    if (lanes[index].state != LANE_FREE && lanes[index].state != LANE_EXITED &&
        lanes[index].tid == tid)
      return &lanes[index];
  return NULL;
}

static int lanes_other_live(host_lane *self) {
  unsigned index;
  for (index = 0; index < NELIBC_HOST_LANE_COUNT; index++)
    if (&lanes[index] != self && lanes[index].state != LANE_FREE &&
        lanes[index].state != LANE_EXITED)
      return 1;
  return 0;
}

static long deliver_signal(long signal);
static long do_munmap(void *address, size_t length);
static long futex_wake(int *address, int count);

/* Runs in the lane that was just switched to: free what the lane we left
 * can no longer free itself, then deliver signals queued for this lane. */
static void lane_after_switch(void) {
  host_lane *from = switched_from;
  host_lane *self = current_lane;
  unsigned index;
  switched_from = NULL;
  if (from != NULL && from->state == LANE_EXITED) {
    for (index = 0; index < 3; index++) {
      if (from->release[index] != NULL) do_munmap(from->release[index], from->release_size[index]);
      from->release[index] = NULL;
    }
    from->state = LANE_FREE;
  }
  while (self->pending_signals != 0) {
    long signal;
    for (signal = 1; signal < NELIBC_HOST_SIGNAL_COUNT; signal++)
      if (self->pending_signals & (1ul << signal)) break;
    self->pending_signals &= ~(1ul << signal);
    deliver_signal(signal);
  }
}

static void switch_to(host_lane *next) {
  host_lane *previous = lane_current();
  const nelibc_host *host = table();
  if (next == previous) return;
  current_lane = next;
  if (next->thread_pointer != previous->thread_pointer) {
    host->thread_area_set(host->context, next->thread_pointer);
    nelibc_host_thread_area_record(next->thread_pointer);
  }
  switched_from = previous;
  lane_switches++;
  host_lane_switch(&previous->sp, next->sp);
  lane_after_switch();
}

static int lanes_timed_waiters(void) {
  unsigned index;
  for (index = 0; index < NELIBC_HOST_LANE_COUNT; index++)
    if (lanes[index].state == LANE_BLOCKED && lanes[index].has_deadline) return 1;
  return 0;
}

static void lanes_expire(uint64_t now) {
  unsigned index;
  for (index = 0; index < NELIBC_HOST_LANE_COUNT; index++) {
    host_lane *lane = &lanes[index];
    if (lane->state == LANE_BLOCKED && lane->has_deadline && lane->deadline <= now) {
      lane->state = LANE_RUNNABLE;
      lane->has_deadline = 0;
      lane->wait_address = NULL;
      lane->wake_result = -ETIMEDOUT;
    }
  }
}

static int lanes_earliest_deadline(uint64_t *deadline) {
  unsigned index;
  int found = 0;
  for (index = 0; index < NELIBC_HOST_LANE_COUNT; index++) {
    host_lane *lane = &lanes[index];
    if (lane->state == LANE_BLOCKED && lane->has_deadline &&
        (!found || lane->deadline < *deadline)) {
      *deadline = lane->deadline;
      found = 1;
    }
  }
  return found;
}

/* Round robin: the first runnable lane after self, self last. */
static host_lane *lane_pick(host_lane *self) {
  unsigned start = (unsigned)(self - lanes);
  unsigned step;
  for (step = 1; step <= NELIBC_HOST_LANE_COUNT; step++) {
    host_lane *lane = &lanes[(start + step) % NELIBC_HOST_LANE_COUNT];
    if (lane->state == LANE_RUNNABLE) return lane;
  }
  return NULL;
}

/* Run another lane until this one is runnable again. Returns only when the
 * calling lane is runnable; an exited lane never comes back. */
static void schedule(void) {
  host_lane *self = lane_current();
  for (;;) {
    host_lane *next;
    uint64_t now;
    uint64_t earliest;
    if (lanes_timed_waiters() && monotonic_now(&now) == 0) lanes_expire(now);
    next = lane_pick(self);
    if (next != NULL) {
      switch_to(next);
      if (self->state == LANE_RUNNABLE) return;
      continue;
    }
    if (!lanes_earliest_deadline(&earliest) || monotonic_now(&now) != 0)
      nelibc_host_fail("nelibc host: every lane is waiting for another, stopping\n",
                       NELIBC_HOST_EXIT_DEADLOCK);
    if (earliest > now) host_sleep_raw(earliest - now);
  }
}

/* Block the calling lane on a futex word (or just time, when address is
 * NULL). Returns 0 when woken, -ETIMEDOUT, or -EINTR for a queued signal. */
static long lane_block(int *address, int has_deadline, uint64_t deadline) {
  host_lane *self = lane_current();
  self->state = LANE_BLOCKED;
  self->wait_address = address;
  self->has_deadline = has_deadline;
  self->deadline = deadline;
  self->wake_result = 0;
  schedule();
  self->wait_address = NULL;
  self->has_deadline = 0;
  return self->wake_result;
}

/* Let other lanes run. Returns 0 when no lane can make progress before the
 * caller's own deadline would (or without one, ever): the caller then fails
 * as a deadlock or sleeps the deadline out itself. */
static int lane_yield(int has_deadline, uint64_t deadline) {
  host_lane *self = lane_current();
  host_lane *next;
  uint64_t now;
  uint64_t earliest;
  if (lanes_timed_waiters() && monotonic_now(&now) == 0) lanes_expire(now);
  next = lane_pick(self);
  if (next != self) {
    switch_to(next);
    return 1;
  }
  if (!lanes_earliest_deadline(&earliest)) {
    if (!has_deadline) return 0;
    earliest = deadline;
  } else if (has_deadline && deadline < earliest) {
    earliest = deadline;
  }
  if (monotonic_now(&now) != 0) return 0;
  if (earliest > now) host_sleep_raw(earliest - now);
  return 1;
}

static long futex_wake(int *address, int count) {
  long woken = 0;
  unsigned index;
  for (index = 0; index < NELIBC_HOST_LANE_COUNT && woken < count; index++) {
    host_lane *lane = &lanes[index];
    if (lane->state == LANE_BLOCKED && lane->wait_address == address) {
      lane->state = LANE_RUNNABLE;
      lane->wait_address = NULL;
      lane->has_deadline = 0;
      lane->wake_result = 0;
      woken++;
    }
  }
  return woken;
}

static long futex_requeue(int *from, int *to, int count) {
  long moved = 0;
  unsigned index;
  for (index = 0; index < NELIBC_HOST_LANE_COUNT && moved < count; index++) {
    host_lane *lane = &lanes[index];
    if (lane->state == LANE_BLOCKED && lane->wait_address == from) {
      lane->wait_address = to;
      moved++;
    }
  }
  return moved;
}

/* Linux robust-list walk at thread exit: every lock the lane still owns is
 * marked FUTEX_OWNER_DIED and one waiter is woken. */
static void lane_robust_exit(host_lane *lane) {
  void **head = lane->robust_list;
  void **entry;
  long offset;
  void *pending;
  unsigned count = 0;
  if (head == NULL) return;
  offset = (long)head[1];
  pending = head[2];
  entry = head[0];
  while (entry != NULL && entry != (void **)head && count++ < 2048) {
    void **next = entry[0];
    if (entry != pending) {
      int *lock = (int *)((char *)entry + offset);
      int value = __atomic_load_n(lock, __ATOMIC_ACQUIRE);
      if ((value & 0x3fffffff) == lane->tid) {
        __atomic_store_n(lock, (value & (int)0x80000000) | 0x40000000, __ATOMIC_RELEASE);
        if (value & (int)0x80000000) futex_wake(lock, 1);
      }
    }
    entry = next;
  }
  lane->robust_list = NULL;
}

/* Thread exit (syscall 60 and the detached-exit helper). The lane's stack
 * stays mapped until the next lane runs lane_after_switch. */
static void lane_exit(int status) {
  const nelibc_host *host = table();
  host_lane *self = lane_current();
  lane_robust_exit(self);
  if (self->clear_tid != NULL) {
    __atomic_store_n(self->clear_tid, 0, __ATOMIC_RELEASE);
    futex_wake(self->clear_tid, 1);
    self->clear_tid = NULL;
  }
  self->pending_signals = 0;
  self->state = LANE_EXITED;
  if (!lanes_other_live(self)) host->exit(host->context, status);
  schedule();
  for (;;) {
  }
}

static void lane_entry(void) {
  host_lane *self = current_lane;
  lane_after_switch();
  lane_exit(self->start(self->argument));
}

/* thread.nelua's clone wrapper: a new lane on the caller-provided stack and
 * TLS block. The new lane runs first, until it blocks, yields, or exits
 * (Linux promises no order either way); a program whose main thread never
 * waits would otherwise starve every worker it made. tid, when given,
 * receives the thread id before either lane continues and is cleared and
 * woken when the lane exits (CLONE_PARENT_SETTID | CLONE_CHILD_CLEARTID). */
long nelibc_clone_thread(void *start, void *arg, void *stack_top,
                         void *tls_tp, int *tid) {
  host_lane *self = lane_current();
  host_lane *lane = NULL;
  unsigned long *sp;
  unsigned index;
  if (start == NULL || stack_top == NULL) return -EINVAL;
  for (index = 0; index < NELIBC_HOST_LANE_COUNT; index++)
    if (lanes[index].state == LANE_FREE) {
      lane = &lanes[index];
      break;
    }
  if (lane == NULL) return -EAGAIN;
  zero_bytes(lane, sizeof *lane);
  do {
    lane->tid = next_tid++;
    if (next_tid > 0x3fffffff) next_tid = 2;
  } while (lane_by_tid(lane->tid) != NULL);
  lane->thread_pointer = tls_tp != NULL ? tls_tp : self->thread_pointer;
  lane->mask = self->mask;
  lane->start = (int (*)(void *))start;
  lane->argument = arg;
  lane->clear_tid = tid;
  sp = (unsigned long *)((uintptr_t)stack_top & ~(uintptr_t)15);
  sp -= 1;                                  /* entry sees rsp = 8 mod 16 */
  *--sp = (unsigned long)(uintptr_t)lane_entry;
  for (index = 0; index < 6; index++) *--sp = 0;
  lane->sp = sp;
  if (tid != NULL) __atomic_store_n(tid, lane->tid, __ATOMIC_RELEASE);
  lane->state = LANE_RUNNABLE;
  lanes_created++;
  {
    int child_tid = lane->tid;
    switch_to(lane);
    return child_tid;
  }
}

/* environment_adapter.c's version unmaps its own stack and leaves through a
 * `syscall` instruction; the host archive weakens it so this one is linked.
 * The three regions are released by the next lane, after the switch. */
void nelibc_thread_detached_exit(void *stack, size_t stack_size, void *tls,
                                 size_t tls_size, void *cleanup_stack,
                                 size_t cleanup_stack_size) {
  host_lane *self = lane_current();
  self->release[0] = stack;
  self->release_size[0] = stack_size;
  self->release[1] = tls;
  self->release_size[1] = tls_size;
  self->release[2] = cleanup_stack;
  self->release_size[2] = cleanup_stack_size;
  lane_exit(0);
}

/* Sleep: other lanes run meanwhile. */
static int sleep_for(uint64_t nanoseconds) {
  uint64_t now;
  long result;
  if (monotonic_now(&now) != 0 || !lanes_other_live(lane_current()))
    return host_sleep_raw(nanoseconds);
  result = lane_block(NULL, 1, now + nanoseconds);
  return result == -EINTR ? -EINTR : 0;
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
  {
    uint64_t start = 0;
    uint64_t total = timespec_nanoseconds(requested);
    int clocked = monotonic_now(&start) == 0;
    result = sleep_for(total);
    if (remaining != NULL) {
      uint64_t left = 0;
      uint64_t now;
      if (result == -EINTR && clocked && monotonic_now(&now) == 0 && now - start < total)
        left = total - (now - start);
      store_timespec(remaining, left);
    }
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
  while (pipe->length == 0) {
    if (pipe->writers == 0) return 0;
    if (entry->status_flags & 0x800) return -EAGAIN;
    if (!lane_yield(0, 0))
      nelibc_host_fail("nelibc host: blocking read on an empty pipe that no lane can fill\n",
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
  while (pipe->length == NELIBC_HOST_PIPE_CAPACITY) {
    if (entry->status_flags & 0x800) return -EAGAIN;
    if (!lane_yield(0, 0))
      nelibc_host_fail("nelibc host: blocking write on a full pipe that no lane can drain\n",
                       NELIBC_HOST_EXIT_DEADLOCK);
    if (pipe->readers == 0) return -EPIPE;
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

/* Wait for scan() to report something ready, letting other lanes run. With
 * a timeout the wait ends at the deadline (result 0); without one, a wait
 * that no lane could satisfy stops the host. */
static long wait_ready(long (*scan)(void *), void *argument, int has_timeout,
                       uint64_t timeout_nanoseconds) {
  uint64_t deadline = 0;
  uint64_t now;
  int clocked = 0;
  if (has_timeout) {
    if (timeout_nanoseconds == 0) return scan(argument);
    clocked = monotonic_now(&now) == 0;
    deadline = now + timeout_nanoseconds;
    if (!clocked) {
      long ready = scan(argument);
      if (ready != 0) return ready;
      host_sleep_raw(timeout_nanoseconds);
      return scan(argument);
    }
  }
  for (;;) {
    long ready = scan(argument);
    if (ready != 0) return ready;
    if (has_timeout) {
      if (monotonic_now(&now) != 0 || now >= deadline) return scan(argument);
      if (!lane_yield(1, deadline)) {
        host_sleep_raw(deadline - now);
        return scan(argument);
      }
    } else if (!lane_yield(0, 0)) {
      nelibc_host_fail("nelibc host: indefinite wait with nothing ready that no lane can change\n",
                       NELIBC_HOST_EXIT_DEADLOCK);
    }
  }
}

typedef struct poll_request {
  kernel_pollfd *entries;
  unsigned long count;
} poll_request;

static long poll_scan(void *argument) {
  poll_request *request = argument;
  kernel_pollfd *entries = request->entries;
  unsigned long count = request->count;
  long ready = 0;
  unsigned long index;
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
  return ready;
}

static long do_poll(kernel_pollfd *entries, unsigned long count,
                    int has_timeout, uint64_t timeout_nanoseconds) {
  poll_request request;
  if (count > 1024) return -EINVAL;
  if (count > 0 && entries == NULL) return -EFAULT;
  request.entries = entries;
  request.count = count;
  return wait_ready(poll_scan, &request, has_timeout, timeout_nanoseconds);
}

static int fdset_test(const unsigned long *set, long fd) {
  return set != NULL && (set[fd / 64] >> (fd % 64)) & 1;
}

typedef struct select_request {
  long nfds;
  const unsigned long *reads;
  const unsigned long *writes;
  const unsigned long *excepts;
  unsigned long ready_reads[16];
  unsigned long ready_writes[16];
} select_request;

static long select_scan(void *argument) {
  select_request *request = argument;
  long ready = 0;
  long fd;
  zero_bytes(request->ready_reads, sizeof request->ready_reads);
  zero_bytes(request->ready_writes, sizeof request->ready_writes);
  for (fd = 0; fd < request->nfds; fd++) {
    host_fd *entry;
    int wants_read = fdset_test(request->reads, fd);
    int wants_write = fdset_test(request->writes, fd);
    if (!wants_read && !wants_write && !fdset_test(request->excepts, fd)) continue;
    entry = fd_entry(fd);
    if (entry == NULL) return -EBADF;
    if (wants_read && fd_readable(entry)) {
      request->ready_reads[fd / 64] |= 1ul << (fd % 64);
      ready++;
    }
    if (wants_write && fd_writable(entry)) {
      request->ready_writes[fd / 64] |= 1ul << (fd % 64);
      ready++;
    }
  }
  return ready;
}

static long do_select(long nfds, unsigned long *reads, unsigned long *writes,
                      unsigned long *excepts, int has_timeout,
                      uint64_t timeout_nanoseconds) {
  select_request request;
  long ready;
  if (nfds < 0 || nfds > 1024) return -EINVAL;
  request.nfds = nfds;
  request.reads = reads;
  request.writes = writes;
  request.excepts = excepts;
  ready = wait_ready(select_scan, &request, has_timeout, timeout_nanoseconds);
  if (ready < 0) return ready;
  if (reads != NULL) copy_bytes(reads, request.ready_reads, sizeof request.ready_reads[0] * ((nfds + 63) / 64));
  if (writes != NULL) copy_bytes(writes, request.ready_writes, sizeof request.ready_writes[0] * ((nfds + 63) / 64));
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

/* Deliver a signal to the calling lane inline: raise() is a direct call and
 * a default terminating action stops the host with the conventional
 * 128 + signal status. */
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

/* tkill/tgkill: a signal for another lane is queued and delivered when that
 * lane next runs; a blocking wait it is in ends with EINTR. Signal 0 is the
 * liveness probe thread.nelua uses to reclaim robust-list slots. */
static long deliver_signal_to(long tid, long signal) {
  host_lane *lane = lane_by_tid((int)tid);
  kernel_sigaction *action;
  if (lane == NULL) return -ESRCH;
  if (signal == 0) return 0;
  if (signal < 0 || signal >= NELIBC_HOST_SIGNAL_COUNT) return -EINVAL;
  if (lane == lane_current()) return deliver_signal(signal);
  action = &signal_actions[signal];
  if ((unsigned long)action->handler == 1ul) return 0;
  if (action->handler == NULL) {
    if (signal_default_ignored(signal)) return 0;
    nelibc_host_fail("nelibc host: terminating signal with the default action\n",
                     128 + (int)signal);
  }
  lane->pending_signals |= 1ul << signal;
  if (lane->state == LANE_BLOCKED) {
    lane->state = LANE_RUNNABLE;
    lane->wait_address = NULL;
    lane->has_deadline = 0;
    lane->wake_result = -EINTR;
  }
  return 0;
}

/* Faults ---------------------------------------------------------------- */

typedef struct fault_call {
  void *handler;
  int signal;
  int siginfo;
  void *context;
  unsigned char info[128];
} fault_call;

static void fault_invoke(void *argument) {
  fault_call *call = argument;
  if (call->siginfo)
    ((void (*)(int, void *, void *))call->handler)(call->signal, call->info, call->context);
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

int nelibc_host_fault(int signal, int code, void *address, void *context) {
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
  call.context = context;
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

#define FUTEX_WAITERS ((int)0x80000000)
#define FUTEX_OWNER_DIED 0x40000000
#define FUTEX_TID_MASK 0x3fffffff

/* Deadline for a futex wait in monotonic nanoseconds. FUTEX_WAIT takes a
 * relative timeout; the bitset and PI forms take an absolute one on the
 * monotonic clock, or on the realtime clock with FUTEX_CLOCK_REALTIME. */
static long futex_deadline(long operation, int relative, const kernel_timespec *timeout,
                           uint64_t *deadline) {
  uint64_t now;
  uint64_t target;
  if (!timespec_valid(timeout)) return -EINVAL;
  target = timespec_nanoseconds(timeout);
  if (monotonic_now(&now) != 0) return -EINVAL;
  if (relative) {
    *deadline = now + target;
    return 0;
  }
  if (operation & 256) { /* FUTEX_CLOCK_REALTIME */
    uint64_t real;
    if (realtime_now(&real) != 0) return -EINVAL;
    *deadline = target > real ? now + (target - real) : now;
    return 0;
  }
  *deadline = target;
  return 0;
}

static int futex_waiters_on(int *address) {
  unsigned index;
  for (index = 0; index < NELIBC_HOST_LANE_COUNT; index++)
    if (lanes[index].state == LANE_BLOCKED && lanes[index].wait_address == address)
      return 1;
  return 0;
}

static long futex_wake_op(int *address, int count, int *address2, int count2, int encoded) {
  int op = (encoded >> 28) & 0xf;
  int cmp = (encoded >> 24) & 0xf;
  int oparg = (encoded >> 12) & 0xfff;
  int cmparg = encoded & 0xfff;
  int old;
  int result;
  int hit;
  if (address2 == NULL) return -EFAULT;
  if (op & 8) oparg = 1 << (oparg & 7); /* FUTEX_OP_OPARG_SHIFT */
  old = __atomic_load_n(address2, __ATOMIC_ACQUIRE);
  switch (op & 7) {
  case 0: __atomic_store_n(address2, oparg, __ATOMIC_RELEASE); break;
  case 1: __atomic_store_n(address2, old + oparg, __ATOMIC_RELEASE); break;
  case 2: __atomic_store_n(address2, old | oparg, __ATOMIC_RELEASE); break;
  case 3: __atomic_store_n(address2, old & ~oparg, __ATOMIC_RELEASE); break;
  case 4: __atomic_store_n(address2, old ^ oparg, __ATOMIC_RELEASE); break;
  default: return -ENOSYS;
  }
  switch (cmp) {
  case 0: hit = old == cmparg; break;
  case 1: hit = old != cmparg; break;
  case 2: hit = old < cmparg; break;
  case 3: hit = old <= cmparg; break;
  case 4: hit = old > cmparg; break;
  case 5: hit = old >= cmparg; break;
  default: return -ENOSYS;
  }
  result = (int)futex_wake(address, count);
  if (hit) result += (int)futex_wake(address2, count2);
  return result;
}

static long do_futex(int *address, long operation, int value,
                     const kernel_timespec *timeout, int *address2, int value3) {
  long op = operation & 0x7f;
  host_lane *self = lane_current();
  uint64_t deadline = 0;
  long result;
  if (address == NULL) return -EFAULT;
  switch (op) {
  case 0: /* FUTEX_WAIT */
  case 9: /* FUTEX_WAIT_BITSET */
    if (__atomic_load_n(address, __ATOMIC_ACQUIRE) != value) return -EAGAIN;
    if (timeout != NULL) {
      result = futex_deadline(operation, op == 0, timeout, &deadline);
      if (result != 0) return result;
    }
    return lane_block(address, timeout != NULL, deadline);
  case 1:  /* FUTEX_WAKE */
  case 10: /* FUTEX_WAKE_BITSET */
    return futex_wake(address, value);
  case 5:  /* FUTEX_WAKE_OP */
    return futex_wake_op(address, value, address2, (int)(uintptr_t)timeout, value3);
  case 3: /* FUTEX_REQUEUE */
  case 4: /* FUTEX_CMP_REQUEUE */
    if (address2 == NULL) return -EFAULT;
    if (op == 4 && __atomic_load_n(address, __ATOMIC_ACQUIRE) != value3) return -EAGAIN;
    result = futex_wake(address, value);
    {
      long moved = futex_requeue(address, address2, (int)(uintptr_t)timeout);
      return op == 4 ? result + moved : result;
    }
  case 6: /* FUTEX_LOCK_PI */
  case 13: /* FUTEX_LOCK_PI2 */
    if (timeout != NULL) {
      result = futex_deadline(operation, 0, timeout, &deadline);
      if (result != 0) return result;
    }
    for (;;) {
      int current = __atomic_load_n(address, __ATOMIC_ACQUIRE);
      if ((current & FUTEX_TID_MASK) == 0) {
        int next = self->tid | (current & FUTEX_OWNER_DIED) |
                   (futex_waiters_on(address) ? FUTEX_WAITERS : 0);
        if (__atomic_compare_exchange_n(address, &current, next, 0,
                                        __ATOMIC_ACQ_REL, __ATOMIC_ACQUIRE))
          return 0;
        continue;
      }
      if ((current & FUTEX_TID_MASK) == self->tid) return -EDEADLK;
      if (lane_by_tid(current & FUTEX_TID_MASK) == NULL) return -ESRCH;
      if (!(current & FUTEX_WAITERS) &&
          !__atomic_compare_exchange_n(address, &current, current | FUTEX_WAITERS, 0,
                                       __ATOMIC_ACQ_REL, __ATOMIC_ACQUIRE))
        continue;
      self->pi_handoff = 0;
      result = lane_block(address, timeout != NULL, deadline);
      if (self->pi_handoff) {
        self->pi_handoff = 0;
        return 0;
      }
      if (result == -ETIMEDOUT || result == -EINTR) return result;
    }
  case 7: { /* FUTEX_UNLOCK_PI */
    int current = __atomic_load_n(address, __ATOMIC_ACQUIRE);
    unsigned index;
    host_lane *waiter = NULL;
    if ((current & FUTEX_TID_MASK) != self->tid) return -EPERM;
    for (index = 0; index < NELIBC_HOST_LANE_COUNT; index++)
      if (lanes[index].state == LANE_BLOCKED && lanes[index].wait_address == address) {
        waiter = &lanes[index];
        break;
      }
    if (waiter == NULL) {
      __atomic_store_n(address, 0, __ATOMIC_RELEASE);
      return 0;
    }
    waiter->state = LANE_RUNNABLE;
    waiter->wait_address = NULL;
    waiter->has_deadline = 0;
    waiter->wake_result = 0;
    waiter->pi_handoff = 1;
    __atomic_store_n(address, waiter->tid | (futex_waiters_on(address) ? FUTEX_WAITERS : 0),
                     __ATOMIC_RELEASE);
    return 0;
  }
  case 8: { /* FUTEX_TRYLOCK_PI */
    int current = __atomic_load_n(address, __ATOMIC_ACQUIRE);
    int next;
    if ((current & FUTEX_TID_MASK) != 0) return -EAGAIN;
    next = self->tid | (current & FUTEX_OWNER_DIED) |
           (futex_waiters_on(address) ? FUTEX_WAITERS : 0);
    if (__atomic_compare_exchange_n(address, &current, next, 0,
                                    __ATOMIC_ACQ_REL, __ATOMIC_ACQUIRE))
      return 0;
    return -EAGAIN;
  }
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

long nelibc_sys6(unsigned long number, unsigned long a1,
                         unsigned long a2, unsigned long a3, unsigned long a4,
                         unsigned long a5, unsigned long a6) {
  const nelibc_host *host = table();
  if (host == NULL) {
    if (number == NELIBC_SYS_EXIT || number == NELIBC_SYS_EXIT_GROUP) {
      for (;;) {
      }
    }
    return -ENOSYS;
  }
  switch (number) {
  case NELIBC_SYS_READ: return do_read((long)a1, (void *)a2, a3);
  case NELIBC_SYS_WRITE: return do_write((long)a1, (const void *)a2, a3);
  case NELIBC_SYS_OPEN: return -ENOENT;                                      /* open */
  case NELIBC_SYS_CLOSE: /* close */
    if (fd_entry((long)a1) == NULL) return -EBADF;
    fd_release((long)a1);
    return 0;
  case NELIBC_SYS_STAT: case NELIBC_SYS_FSTAT: case NELIBC_SYS_LSTAT: return -ENOENT;                      /* stat */
  case NELIBC_SYS_POLL: return do_poll((kernel_pollfd *)a1, a2, (int)a3 >= 0,
                         (int)a3 > 0 ? (uint64_t)(int)a3 * 1000000ull : 0);
  case NELIBC_SYS_LSEEK: return -ESPIPE;                                      /* lseek */
  case NELIBC_SYS_MMAP: return do_mmap((void *)a1, a2, (long)a3, (long)a4, (long)a5, (long)a6);
  case NELIBC_SYS_MPROTECT: return do_mprotect((void *)a1, a2, (long)a3);
  case NELIBC_SYS_MUNMAP: return do_munmap((void *)a1, a2);
  case NELIBC_SYS_BRK: return -ENOMEM;                                     /* brk */
  case NELIBC_SYS_RT_SIGACTION: return do_rt_sigaction((long)a1, (const kernel_sigaction *)a2,
                                  (kernel_sigaction *)a3, a4);
  case NELIBC_SYS_RT_SIGPROCMASK: return do_rt_sigprocmask((long)a1, (const unsigned long *)a2,
                                    (unsigned long *)a3, a4);
  case NELIBC_SYS_IOCTL: return fd_entry((long)a1) == NULL ? -EBADF : -ENOTTY; /* ioctl */
  case NELIBC_SYS_PREAD64: case NELIBC_SYS_PWRITE64: return -ESPIPE;                            /* pread, pwrite */
  case NELIBC_SYS_READV: return do_readv((long)a1, (const kernel_iovec *)a2, (long)a3);
  case NELIBC_SYS_WRITEV: return do_writev((long)a1, (const kernel_iovec *)a2, (long)a3);
  case NELIBC_SYS_ACCESS: return -ENOENT;                                     /* access */
  case NELIBC_SYS_PIPE: return do_pipe2((int *)a1, 0);
  case NELIBC_SYS_SELECT: {
    const kernel_timeval *timeout = (const kernel_timeval *)a5;
    uint64_t nanoseconds = 0;
    if (timeout != NULL) {
      if (timeout->tv_sec < 0 || timeout->tv_usec < 0 || timeout->tv_usec >= 1000000) return -EINVAL;
      nanoseconds = (uint64_t)timeout->tv_sec * 1000000000ull + (uint64_t)timeout->tv_usec * 1000ull;
    }
    return do_select((long)a1, (unsigned long *)a2, (unsigned long *)a3,
                     (unsigned long *)a4, timeout != NULL, nanoseconds);
  }
  case NELIBC_SYS_SCHED_YIELD: lane_yield(0, 0); return 0;                         /* sched_yield */
  case NELIBC_SYS_MREMAP: return do_mremap((void *)a1, a2, a3, (long)a4);
  case NELIBC_SYS_MSYNC: return 0;                                           /* msync */
  case NELIBC_SYS_MADVISE: return 0;                                           /* madvise */
  case NELIBC_SYS_DUP: return do_fcntl((long)a1, 0, 0);                    /* dup */
  case NELIBC_SYS_DUP2: return a1 == a2 ? (fd_entry((long)a1) == NULL ? -EBADF : (long)a2)
                           : do_dup3((long)a1, (long)a2, 0);
  case NELIBC_SYS_NANOSLEEP: return do_nanosleep((const kernel_timespec *)a1, (kernel_timespec *)a2);
  case NELIBC_SYS_ALARM: return 0;                                           /* alarm */
  case NELIBC_SYS_GETPID: return 1;                                           /* getpid */
  case NELIBC_SYS_CLONE: return -ENOSYS;                                     /* raw clone: threads use nelibc_clone_thread */
  case NELIBC_SYS_FORK: case NELIBC_SYS_VFORK: case NELIBC_SYS_EXECVE: return -ENOSYS;                   /* fork, vfork, execve */
  case NELIBC_SYS_EXIT:                                                     /* exit: this lane */
    lane_exit((int)a1);
    for (;;) {
    }
  case NELIBC_SYS_EXIT_GROUP:                                                    /* exit_group */
    host->exit(host->context, (int)a1);
    for (;;) {
    }
  case NELIBC_SYS_WAIT4: return -ECHILD;                                   /* wait4 */
  case NELIBC_SYS_KILL: return deliver_signal((long)a2);                    /* kill */
  case NELIBC_SYS_UNAME: return do_uname((char *)a1);
  case NELIBC_SYS_FCNTL: return do_fcntl((long)a1, (long)a2, a3);
  case NELIBC_SYS_GETCWD: /* getcwd */
    if (a2 < 2) return -ERANGE;
    copy_string((char *)a1, a2, "/");
    return 2;
  case NELIBC_SYS_READLINK: case NELIBC_SYS_READLINKAT: return -ENOENT;                           /* readlink */
  case NELIBC_SYS_UMASK: return 022;                                         /* umask */
  case NELIBC_SYS_GETTIMEOFDAY: { /* gettimeofday */
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
  case NELIBC_SYS_GETRLIMIT: return fill_rlimit((long)a1, (kernel_rlimit *)a2);  /* getrlimit */
  case NELIBC_SYS_GETRUSAGE: if (a2 != 0) zero_bytes((void *)a2, 144); return 0;   /* getrusage */
  case NELIBC_SYS_SYSINFO: return -ENOSYS;                                     /* sysinfo */
  case NELIBC_SYS_TIMES: { /* times */
    uint64_t nanoseconds = 0;
    if (a1 != 0) zero_bytes((void *)a1, 32);
    if (monotonic_now(&nanoseconds) != 0) return 0;
    return (long)(nanoseconds / 10000000ull);
  }
  case NELIBC_SYS_GETUID: case NELIBC_SYS_GETGID: case NELIBC_SYS_GETEUID: case NELIBC_SYS_GETEGID: return 0;            /* uid, gid */
  case NELIBC_SYS_SETPGID: case NELIBC_SYS_GETPPID: case NELIBC_SYS_GETPGRP: case NELIBC_SYS_GETPGID: return 1;            /* setpgid, getppid, getpgrp, getpgid */
  case NELIBC_SYS_SETSID: return 1;                                          /* setsid */
  case NELIBC_SYS_SIGALTSTACK: return do_sigaltstack((const kernel_stack *)a1, (kernel_stack *)a2);
  case NELIBC_SYS_SCHED_GETPARAM: if (a2 != 0) zero_bytes((void *)a2, 4); return 0;  /* sched_getparam */
  case NELIBC_SYS_SCHED_SETSCHEDULER: return (long)a2 == 0 ? 0 : -EPERM;                 /* sched_setscheduler */
  case NELIBC_SYS_SCHED_GETSCHEDULER: return 0;                                          /* sched_getscheduler */
  case NELIBC_SYS_SCHED_GET_PRIORITY_MAX: return (long)a1 == 1 || (long)a1 == 2 ? 99 : 0;    /* priority max */
  case NELIBC_SYS_SCHED_GET_PRIORITY_MIN: return (long)a1 == 1 || (long)a1 == 2 ? 1 : 0;     /* priority min */
  case NELIBC_SYS_PRCTL: /* prctl */
    if ((long)a1 == 15) return 0;                              /* PR_SET_NAME */
    if ((long)a1 == 16) { copy_string((char *)a2, 16, "nelibc"); return 0; }
    return -EINVAL;
  case 158: /* arch_prctl */
    if (a1 == 0x1002) {
      int result = host->thread_area_set(host->context, (void *)a2);
      if (result == 0) {
        nelibc_host_thread_area_record((void *)a2);
        lane_current()->thread_pointer = (void *)a2;
      }
      return result;
    }
    if (a1 == 0x1003) {
      if (a2 == 0) return -EFAULT;
      *(void **)a2 = nelibc_host_thread_area_current();
      return 0;
    }
    return -EINVAL;
  case NELIBC_SYS_GETTID: return lane_current()->tid;                        /* gettid */
  case NELIBC_SYS_TKILL: return deliver_signal_to((long)a1, (long)a2);      /* tkill */
  case NELIBC_SYS_TIME: { /* time */
    uint64_t nanoseconds;
    int result = realtime_now(&nanoseconds);
    if (result != 0) return result;
    if (a1 != 0) *(long *)a1 = (long)(nanoseconds / 1000000000ull);
    return (long)(nanoseconds / 1000000000ull);
  }
  case NELIBC_SYS_FUTEX: return do_futex((int *)a1, (long)a2, (int)a3, (const kernel_timespec *)a4,
                            (int *)a5, (int)a6);
  case NELIBC_SYS_SCHED_SETAFFINITY: return 0;                                          /* sched_setaffinity */
  case NELIBC_SYS_SCHED_GETAFFINITY: return do_sched_getaffinity(a2, (unsigned long *)a3);
  case NELIBC_SYS_SET_TID_ADDRESS: lane_current()->clear_tid = (int *)a1; return lane_current()->tid; /* set_tid_address */
  case NELIBC_SYS_CLOCK_GETTIME: return do_clock_gettime((int)a1, (kernel_timespec *)a2);
  case NELIBC_SYS_CLOCK_GETRES: { /* clock_getres */
    kernel_timespec *value = (kernel_timespec *)a2;
    if ((long)a1 < 0 || (long)a1 > 7) return -EINVAL;
    if (value != NULL) { value->tv_sec = 0; value->tv_nsec = 1; }
    return 0;
  }
  case NELIBC_SYS_CLOCK_NANOSLEEP: return do_clock_nanosleep((int)a1, (int)a2, (const kernel_timespec *)a3,
                                      (kernel_timespec *)a4);
  case NELIBC_SYS_TGKILL: return deliver_signal_to((long)a2, (long)a3);      /* tgkill */
  case NELIBC_SYS_OPENAT: return -ENOENT;                                    /* openat */
  case NELIBC_SYS_NEWFSTATAT: case NELIBC_SYS_STATX: return -ENOENT;                          /* newfstatat, statx */
  case NELIBC_SYS_FACCESSAT: return -ENOENT;                                    /* faccessat */
  case NELIBC_SYS_PSELECT6: { /* pselect6 */
    const kernel_timespec *timeout = (const kernel_timespec *)a5;
    if (timeout != NULL && !timespec_valid(timeout)) return -EINVAL;
    return do_select((long)a1, (unsigned long *)a2, (unsigned long *)a3,
                     (unsigned long *)a4, timeout != NULL,
                     timeout != NULL ? timespec_nanoseconds(timeout) : 0);
  }
  case NELIBC_SYS_PPOLL: { /* ppoll */
    const kernel_timespec *timeout = (const kernel_timespec *)a3;
    if (timeout != NULL && !timespec_valid(timeout)) return -EINVAL;
    return do_poll((kernel_pollfd *)a1, a2, timeout != NULL,
                   timeout != NULL ? timespec_nanoseconds(timeout) : 0);
  }
  case NELIBC_SYS_SET_ROBUST_LIST: lane_current()->robust_list = (void *)a1; return 0; /* set_robust_list */
  case NELIBC_SYS_GET_ROBUST_LIST: /* get_robust_list */
    if (a1 != 0 && a1 != (unsigned long)lane_current()->tid) return -ESRCH;
    if (a2 == 0 || a3 == 0) return -EFAULT;
    *(void **)a2 = lane_current()->robust_list;
    *(size_t *)a3 = 24;
    return 0;
  case NELIBC_SYS_DUP3: return do_dup3((long)a1, (long)a2, (int)a3);
  case NELIBC_SYS_PIPE2: return do_pipe2((int *)a1, (int)a2);
  case NELIBC_SYS_PRLIMIT64: /* prlimit64 */
    if (a3 != 0) {
      const kernel_rlimit *requested = (const kernel_rlimit *)a3;
      if (requested->current > requested->maximum) return -EINVAL;
    }
    return fill_rlimit((long)a2, (kernel_rlimit *)a4);
  case NELIBC_SYS_GETRANDOM: return -ENOSYS;                                    /* getrandom */
  default:
    report_unsupported(number);
    return -ENOSYS;
  }
}

long nelibc_sys3(unsigned long number, unsigned long a1,
                         unsigned long a2, unsigned long a3) {
  return nelibc_sys6(number, a1, a2, a3, 0, 0, 0);
}
