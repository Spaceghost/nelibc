/* Nelibc host profile: the platform operations a kernel or other
 * freestanding environment supplies beneath the Nelua libc.
 *
 * The host profile is the standalone Linux x86-64 archive with its Linux
 * entry point and raw `syscall` instruction removed. Every kernel request the
 * standalone modules make still arrives as a Linux x86-64 syscall number at
 * `nelibc_raw_syscall3`/`nelibc_raw_syscall6`; the host profile dispatches
 * those numbers to the operations below and answers the rest with -ENOSYS.
 * The libc modules themselves are not modified.
 *
 * The shape follows Drift's `drift_hw`: one table of callbacks plus a context
 * pointer, bound once, with no global state owned by the host. A kernel that
 * already supplies `drift_hw` for its drivers supplies this table from the
 * same platform code (clock, console, memory).
 *
 * Threads: the host profile multiplexes every thread the program creates
 * onto the one hardware thread the host supplies (cooperative lanes). A
 * lane runs until it blocks in the libc (futex wait, sleep, empty pipe,
 * poll with nothing ready), calls sched_yield, or exits; the profile then
 * switches stack and thread pointer (through thread_pointer_set) to the
 * next runnable lane. Nothing preempts a lane. A wait that no lane could
 * end stops the machine through `exit` with status
 * NELIBC_HOST_EXIT_DEADLOCK. Atomics are compiler-provided and unaffected.
 */
#ifndef NELIBC_HOST_H
#define NELIBC_HOST_H

#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/* Static TLS image of the linked program (the PT_TLS segment). On Linux the
 * loader exposes it through the program headers; a kernel exposes it through
 * linker-script symbols. filesz bytes at start are copied, memsz - filesz
 * bytes are zeroed, and the block is aligned to align (at least 16). */
typedef struct nelibc_host_tls {
  const void *start;
  size_t filesz;
  size_t memsz;
  size_t align;
} nelibc_host_tls;

typedef struct nelibc_host {
  void *context;

  /* Required. Write bytes for descriptor 1 or 2. Return the count written
   * (may be short) or a negative errno. */
  long (*console_write)(void *context, int fd, const void *buffer, size_t length);

  /* Required. Reserve length bytes of zero-filled memory aligned to
   * alignment (a power of two, at least 4096). Return NULL on exhaustion.
   * Backs mmap(MAP_ANONYMOUS), thread stacks, and the allocator. A caller
   * may reserve far more than it touches (MAP_NORESERVE); the host decides
   * whether that is lazy or physical. */
  void *(*memory_reserve)(void *context, size_t length, size_t alignment);

  /* Optional. Change protection of a page range (PROT_* bits). NULL means
   * mprotect succeeds without effect, so guard pages are advisory. */
  int (*memory_protect)(void *context, void *address, size_t length, int protection);

  /* Optional. Return a reservation. NULL means munmap succeeds without
   * effect. */
  int (*memory_release)(void *context, void *address, size_t length);

  /* Optional. Monotonic nanoseconds since an arbitrary origin. Return
   * nonzero if no usable clock exists; clock_gettime then fails with EINVAL
   * and sleeps fail the same way. */
  int (*clock_monotonic)(void *context, uint64_t *nanoseconds);

  /* Optional. Nanoseconds since the Unix epoch for CLOCK_REALTIME. NULL
   * falls back to the monotonic clock. */
  int (*clock_realtime)(void *context, uint64_t *nanoseconds);

  /* Optional. Sleep or yield for the duration. NULL spins on the
   * monotonic clock. */
  void (*sleep)(void *context, uint64_t nanoseconds);

  /* Required. Install the x86-64 thread pointer (FS base). On Linux this is
   * arch_prctl(ARCH_SET_FS); in a kernel it is a write to MSR 0xC0000100.
   * Called at boot and on every switch between lanes. */
  int (*thread_pointer_set)(void *context, void *thread_pointer);

  /* Required. Terminate with a status. Must not return. */
  void (*exit)(void *context, int status);

  /* Optional. Called once per unsupported syscall number before -ENOSYS is
   * returned, for diagnostics. */
  void (*unsupported)(void *context, unsigned long number);

  /* NULL-terminated "NAME=value" vector for getenv, or NULL. */
  char **environment;

  /* CPUs reported by sched_getaffinity and sysconf(_SC_NPROCESSORS_ONLN);
   * 0 means 1. The profile still runs on one hardware thread. */
  int cpu_count;

  nelibc_host_tls tls;
} nelibc_host;

/* Exit statuses the host profile itself uses through host->exit. */
#define NELIBC_HOST_EXIT_DEADLOCK 125   /* a wait no other lane can satisfy */
#define NELIBC_HOST_EXIT_BOOT 126       /* nelibc_host_boot could not finish */

/* Bind the table, build the initial thread's TLS block, install the thread
 * pointer, and initialize stdio. Returns 0 or a negative errno. Must run
 * before any other libc call. The table must stay valid for the lifetime of
 * the program. */
int nelibc_host_boot(const nelibc_host *host);

/* Boot, run main, and exit through the libc exit (atexit handlers and stdio
 * flush) and then host->exit. Never returns. */
void nelibc_host_run(const nelibc_host *host,
                     int (*main)(int, char **, char **),
                     int argc, char **argv);

/* The bound table, or NULL before boot. */
const nelibc_host *nelibc_host_current(void);

/* Write a message to descriptor 2 and stop with status; never returns. */
void nelibc_host_fail(const char *message, int status);

/* Lane counters for diagnostics: lanes created so far (threads the program
 * made) and switches between lanes. Either pointer may be NULL. */
void nelibc_host_lane_stats(unsigned long *created, unsigned long *switches);

/* Fault delivery. A host calls this from its exception path (page fault,
 * general protection, divide error, invalid opcode) with the Linux signal
 * the fault maps to (SIGSEGV 11, SIGBUS 7, SIGFPE 8, SIGILL 4), a si_code,
 * and the faulting address. If the program recorded a handler for that
 * signal with sigaction, the profile calls it: on the stack recorded with
 * sigaltstack when the action has SA_ONSTACK and the fault did not occur on
 * that stack already, otherwise on the current stack; with a siginfo_t
 * carrying si_signo, si_code, and si_addr when SA_SIGINFO is set, and a
 * context passed through untouched as the handler's third argument (the
 * host's own machine context, a ucontext_t on the Linux harness, or NULL).
 * A handler that returns makes this return 0, and the host
 * resumes the faulting instruction (so the handler must have repaired the
 * fault, as on Linux). A handler that exits or unwinds never comes back.
 * With no handler, SIG_DFL, or SIG_IGN, the profile stops through
 * host->exit with status 128 + signal after a message on descriptor 2.
 * Returns -EINVAL for a signal outside 1..64. */
int nelibc_host_fault(int signal, int code, void *address, void *context);

#ifdef __cplusplus
}
#endif

#endif
