/* Consumer check for the host profile: exercise the libc surface the Bend
 * runtime and ordinary C programs use, through a host table instead of
 * Linux syscalls. Linked by `make host-smoke-*` with tools/host_linux.c as
 * the host. Each failure returns a distinct status. */
#define _GNU_SOURCE
#include <errno.h>
#include <fcntl.h>
#include <math.h>
#include <pthread.h>
#include <sched.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/mman.h>
#include <sys/select.h>
#include <time.h>
#include <unistd.h>

static volatile sig_atomic_t handled_signal;
static int atexit_ran;

static void on_signal(int signal) {
  handled_signal = signal;
}

/* Faults: the host's exception path reaches the recorded SIGSEGV handler on
 * the recorded alternate stack. The first fault is repaired (the page is
 * unprotected) and the handler returns, so the write resumes; the second is
 * the fail-stop path and leaves through the libc exit so atexit still runs. */
static unsigned char alternate_stack[1 << 16] __attribute__((aligned(16)));
static void *fault_page;
static volatile int fault_count;
static void *volatile fault_address;
static volatile int fault_on_alternate_stack;

static void on_fault(int signal, siginfo_t *info, void *context) {
  char probe;
  (void)context;
  fault_count++;
  fault_address = info->si_addr;
  fault_on_alternate_stack = (unsigned char *)&probe >= alternate_stack &&
                             (unsigned char *)&probe < alternate_stack + sizeof alternate_stack;
  if (signal != SIGSEGV || info->si_signo != SIGSEGV) _exit(60);
  if (fault_count == 1) {
    if (mprotect(fault_page, 4096, PROT_READ | PROT_WRITE) != 0) _exit(61);
    return;
  }
  if (!fault_on_alternate_stack || fault_address != fault_page) _exit(62);
  printf("host smoke: OK\n");
  exit(7);
}

static void on_exit_handler(void) {
  atexit_ran = 1;
  printf("atexit handler ran\n");
  fflush(stdout);
}

static void *never_started(void *argument) {
  return argument;
}

static pthread_once_t once_control = PTHREAD_ONCE_INIT;
static int once_count;
static void once_routine(void) {
  once_count++;
}

int main(int argc, char **argv, char **envp) {
  char text[128];
  struct timespec first;
  struct timespec second;
  struct timespec pause = {0, 2000000};
  struct sigaction action;
  stack_t stack;
  int pipe_fds[2];
  fd_set reads;
  struct timeval timeout = {0, 0};
  char byte = 'x';
  char received = 0;
  void *mapping;
  void *block;
  pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;
  pthread_key_t key;
  pthread_t thread;
  cpu_set_t cpus;
  int result;
  size_t index;

  (void)argc;
  (void)argv;
  (void)envp;

  if (printf("host smoke: stdout through the host console\n") <= 0) return 2;
  if (fprintf(stderr, "host smoke: stderr through the host console\n") <= 0) return 3;

  /* Environment supplied by the host table. */
  if (getenv("BENDRTS") == NULL || strcmp(getenv("BENDRTS"), "--threads 1 --gpu off") != 0) return 4;
  if (getenv("MISSING") != NULL) return 5;

  /* Allocator over memory_reserve, including a growth path through mremap. */
  block = malloc(100);
  if (block == NULL) return 6;
  memset(block, 0xAB, 100);
  block = realloc(block, 1 << 20);
  if (block == NULL || ((unsigned char *)block)[99] != 0xAB) return 7;
  block = realloc(block, 8 << 20);
  if (block == NULL || ((unsigned char *)block)[99] != 0xAB) return 8;
  free(block);
  mapping = mmap(NULL, 1 << 16, PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
  if (mapping == MAP_FAILED) return 9;
  for (index = 0; index < (1 << 16); index++)
    if (((unsigned char *)mapping)[index] != 0) return 10;
  if (mprotect(mapping, 4096, PROT_NONE) != 0) return 11;
  if (munmap(mapping, 1 << 16) != 0) return 12;
  if (mmap(NULL, 4096, PROT_READ, MAP_PRIVATE, 3, 0) != MAP_FAILED || errno != ENODEV) return 13;

  /* Formatting and conversions are pure Nelua and must survive unchanged. */
  snprintf(text, sizeof text, "%d %s %.3f %x %zu", 42, "nelibc", 3.14159, 255u, (size_t)7);
  if (strcmp(text, "42 nelibc 3.142 ff 7") != 0) return 14;
  if (strtol("-123", NULL, 10) != -123 || strtod("2.5e2", NULL) != 250.0) return 15;
  if (sqrt(81.0) != 9.0 || fabs(cbrt(27.0) - 3.0) > 1e-12) return 16;

  /* Clock: monotonic, nondecreasing, and sleeps advance it. */
  if (clock_gettime(CLOCK_MONOTONIC, &first) != 0) return 17;
  if (nanosleep(&pause, NULL) != 0) return 18;
  if (clock_gettime(CLOCK_MONOTONIC, &second) != 0) return 19;
  if (second.tv_sec < first.tv_sec ||
      (second.tv_sec == first.tv_sec && second.tv_nsec < first.tv_nsec)) return 20;
  if (clock_gettime(CLOCK_REALTIME, &first) != 0 || first.tv_sec < 1700000000L) return 21;
  if (clock_gettime(99, &first) == 0 || errno != EINVAL) return 22;

  /* Self-pipe with select, the Bend event-loop wakeup. */
  if (pipe(pipe_fds) != 0) return 23;
  if (fcntl(pipe_fds[0], F_SETFL, O_NONBLOCK) != 0) return 24;
  if ((fcntl(pipe_fds[0], F_GETFL) & O_NONBLOCK) == 0) return 25;
  if (read(pipe_fds[0], &received, 1) != -1 || errno != EAGAIN) return 26;
  FD_ZERO(&reads);
  FD_SET(pipe_fds[0], &reads);
  if (select(pipe_fds[0] + 1, &reads, NULL, NULL, &timeout) != 0) return 27;
  if (write(pipe_fds[1], &byte, 1) != 1) return 28;
  FD_ZERO(&reads);
  FD_SET(pipe_fds[0], &reads);
  if (select(pipe_fds[0] + 1, &reads, NULL, NULL, NULL) != 1 || !FD_ISSET(pipe_fds[0], &reads)) return 29;
  if (read(pipe_fds[0], &received, 1) != 1 || received != 'x') return 30;
  if (close(pipe_fds[0]) != 0 || close(pipe_fds[1]) != 0) return 31;
  if (write(pipe_fds[1], &byte, 1) != -1 || errno != EBADF) return 32;

  /* Signals: registration is recorded and raise() delivers inline. */
  memset(&action, 0, sizeof action);
  action.sa_handler = on_signal;
  action.sa_flags = SA_ONSTACK;
  if (sigaction(SIGUSR1, &action, NULL) != 0) return 33;
  if (raise(SIGUSR1) != 0 || handled_signal != SIGUSR1) return 34;
  if (signal(SIGPIPE, SIG_IGN) == SIG_ERR) return 35;
  if (raise(SIGPIPE) != 0) return 36;

  /* Identity and CPU count come from the host table (cpu_count = 2). */
  if (getpid() != 1) return 37;
  if (sysconf(_SC_NPROCESSORS_ONLN) != 2) return 38;
  CPU_ZERO(&cpus);
  if (sched_getaffinity(0, sizeof cpus, &cpus) != 0 || CPU_COUNT(&cpus) != 2) return 39;
  if (sched_yield() != 0) return 40;
  if (sysconf(_SC_PAGESIZE) != 4096) return 41;

  /* Single-lane pthreads: synchronization works, creation is refused. */
  if (pthread_mutex_lock(&mutex) != 0 || pthread_mutex_unlock(&mutex) != 0) return 42;
  if (pthread_mutex_trylock(&mutex) != 0) return 43;
  if (pthread_mutex_unlock(&mutex) != 0) return 44;
  if (pthread_once(&once_control, once_routine) != 0 ||
      pthread_once(&once_control, once_routine) != 0 || once_count != 1) return 45;
  if (pthread_key_create(&key, NULL) != 0) return 46;
  if (pthread_setspecific(key, &key) != 0 || pthread_getspecific(key) != &key) return 47;
  if (pthread_key_delete(key) != 0) return 48;
  if (pthread_self() == NULL) return 49;
  result = pthread_create(&thread, NULL, never_started, NULL);
  if (result == 0) return 50;

  /* Faults reach the recorded handler on the recorded alternate stack. */
  stack.ss_sp = alternate_stack;
  stack.ss_size = sizeof alternate_stack;
  stack.ss_flags = 0;
  if (sigaltstack(&stack, NULL) != 0) return 52;
  memset(&action, 0, sizeof action);
  action.sa_sigaction = on_fault;
  action.sa_flags = SA_SIGINFO | SA_ONSTACK;
  if (sigaction(SIGSEGV, &action, NULL) != 0) return 53;
  fault_page = mmap(NULL, 4096, PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
  if (fault_page == MAP_FAILED || mprotect(fault_page, 4096, PROT_NONE) != 0) return 54;
  *(volatile int *)fault_page = 42; /* first fault: repaired, the write resumes */
  if (fault_count != 1 || *(volatile int *)fault_page != 42 ||
      fault_address != fault_page || !fault_on_alternate_stack) return 55;
  if (mprotect(fault_page, 4096, PROT_NONE) != 0) return 56;

  /* Exit path: atexit handler, stdio flush, then host exit status. The
   * second fault's handler prints the OK line and calls exit(7). */
  if (atexit(on_exit_handler) != 0) return 51;
  *(volatile int *)fault_page = 43;
  return 57;
}
