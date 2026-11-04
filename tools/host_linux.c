/* A Linux process acting as the host for the nelibc host profile.
 *
 * This is the hosted counterpart of a kernel: it fills nelibc_host with raw
 * Linux syscalls (the only `syscall` instructions in the executable), finds
 * the program's PT_TLS segment through the ELF header, and hands control to
 * nelibc_host_run. No libc is linked; the libc being exercised is the host
 * profile archive itself. Drift's tests/freestanding.c uses the same idea:
 * Linux only supplies memory, a console, and the exit status.
 */
#include "nelibc_host.h"

extern int main(int argc, char **argv, char **envp);

static long linux_syscall(long number, long a1, long a2, long a3, long a4,
                          long a5, long a6) {
  long result;
  register long r10 __asm__("r10") = a4;
  register long r8 __asm__("r8") = a5;
  register long r9 __asm__("r9") = a6;
  __asm__ volatile("syscall"
                   : "=a"(result)
                   : "a"(number), "D"(a1), "S"(a2), "d"(a3), "r"(r10), "r"(r8),
                     "r"(r9)
                   : "rcx", "r11", "memory");
  return result;
}

static long host_console_write(void *context, int fd, const void *buffer,
                               size_t length) {
  (void)context;
  return linux_syscall(1, fd, (long)buffer, (long)length, 0, 0, 0);
}

static void *host_memory_reserve(void *context, size_t length, size_t alignment) {
  long result;
  (void)context;
  if (alignment > 4096) length += alignment;
  result = linux_syscall(9, 0, (long)length, 3, 0x22 | 0x4000, -1, 0);
  if (result < 0 && result >= -4095) return NULL;
  if (alignment > 4096)
    result = (long)(((unsigned long)result + alignment - 1) & ~(alignment - 1));
  return (void *)result;
}

static int host_memory_protect(void *context, void *address, size_t length,
                               int protection) {
  (void)context;
  return (int)linux_syscall(10, (long)address, (long)length, protection, 0, 0, 0);
}

static int host_memory_release(void *context, void *address, size_t length) {
  (void)context;
  return (int)linux_syscall(11, (long)address, (long)length, 0, 0, 0, 0);
}

static int host_clock(long clock_id, uint64_t *nanoseconds) {
  long value[2];
  if (linux_syscall(228, clock_id, (long)value, 0, 0, 0, 0) != 0) return 1;
  *nanoseconds = (uint64_t)value[0] * 1000000000ull + (uint64_t)value[1];
  return 0;
}

static int host_clock_monotonic(void *context, uint64_t *nanoseconds) {
  (void)context;
  return host_clock(1, nanoseconds);
}

static int host_clock_realtime(void *context, uint64_t *nanoseconds) {
  (void)context;
  return host_clock(0, nanoseconds);
}

static void host_sleep(void *context, uint64_t nanoseconds) {
  long request[2];
  (void)context;
  request[0] = (long)(nanoseconds / 1000000000ull);
  request[1] = (long)(nanoseconds % 1000000000ull);
  while (linux_syscall(35, (long)request, (long)request, 0, 0, 0, 0) == -4) {
  }
}

/* NELIBC_HOST_STATS=1 in the real environment: report how many times the
 * profile installed a thread pointer (once at boot, then once per lane
 * switch) on descriptor 2 at exit, so a run can show whether lanes ran. */
static int host_stats;
static unsigned long host_thread_area_sets;

static int host_thread_area_set(void *context, void *thread_pointer) {
  (void)context;
  host_thread_area_sets++;
  return (int)linux_syscall(158, 0x1002, (long)thread_pointer, 0, 0, 0, 0);
}

static void host_write_number(char *text, size_t *length, unsigned long number) {
  static const char digits[] = "0123456789";
  char number_text[24];
  size_t count = 0;
  do {
    number_text[count++] = digits[number % 10];
    number /= 10;
  } while (number != 0);
  while (count > 0) text[(*length)++] = number_text[--count];
}

static void host_write_text(char *text, size_t *length, const char *words) {
  while (*words != 0) text[(*length)++] = *words++;
}

static void host_exit(void *context, int status) {
  (void)context;
  if (host_stats) {
    char text[160] = "host_linux: thread pointer set ";
    size_t length = 31;
    unsigned long created;
    unsigned long switches;
    nelibc_host_lane_stats(&created, &switches);
    host_write_number(text, &length, host_thread_area_sets);
    host_write_text(text, &length, " times, lanes created ");
    host_write_number(text, &length, created);
    host_write_text(text, &length, ", lane switches ");
    host_write_number(text, &length, switches);
    text[length++] = '\n';
    linux_syscall(1, 2, (long)text, (long)length, 0, 0, 0);
  }
  for (;;) linux_syscall(231, status, 0, 0, 0, 0, 0);
}

static void host_unsupported(void *context, unsigned long number) {
  char text[64] = "host_linux: unsupported operation ";
  size_t length = 32;
  (void)context;
  host_write_number(text, &length, number);
  text[length++] = '\n';
  linux_syscall(1, 2, (long)text, (long)length, 0, 0, 0);
}

/* Hardware faults: Linux reports them as signals. Catch SIGILL, SIGBUS,
 * SIGFPE, and SIGSEGV on a private alternate stack (a kernel would take the
 * exception on its own stack the same way) and hand them to the profile,
 * which runs the program's recorded handler. Returning resumes the faulting
 * instruction exactly as Linux would; SA_NODEFER keeps the signal deliverable
 * if the handler unwinds instead of returning. */
static unsigned char host_fault_stack[1 << 16] __attribute__((aligned(16)));

static void host_fault_handler(int signal, void *info, void *context) {
  nelibc_host_fault(signal, *(int *)((char *)info + 8),
                    *(void **)((char *)info + 16), context);
}

extern void host_fault_restorer(void);
__asm__(".text\n"
        ".globl host_fault_restorer\n"
        ".type host_fault_restorer,@function\n"
        "host_fault_restorer:\n"
        "  movq $15, %rax\n"
        "  syscall\n"
        ".size host_fault_restorer, .-host_fault_restorer\n");

static void host_install_faults(void) {
  struct { void *sp; int flags; size_t size; } stack;
  struct { void *handler; unsigned long flags; void *restorer; unsigned long mask; } action;
  static const int signals[] = { 4, 7, 8, 11 };
  unsigned index;
  stack.sp = host_fault_stack;
  stack.flags = 0;
  stack.size = sizeof host_fault_stack;
  linux_syscall(131, (long)&stack, 0, 0, 0, 0, 0);
  action.handler = (void *)host_fault_handler;
  action.flags = 0x4 | 0x08000000 | 0x40000000 | 0x04000000; /* SIGINFO ONSTACK NODEFER RESTORER */
  action.restorer = (void *)host_fault_restorer;
  action.mask = 0;
  for (index = 0; index < sizeof signals / sizeof signals[0]; index++)
    linux_syscall(13, signals[index], (long)&action, 0, 8, 0, 0);
}

/* ELF64 program headers of this static executable, via the linker-provided
 * __ehdr_start symbol. */
typedef struct {
  unsigned char ident[16];
  unsigned short type;
  unsigned short machine;
  unsigned int version;
  unsigned long entry;
  unsigned long phoff;
  unsigned long shoff;
  unsigned int flags;
  unsigned short ehsize;
  unsigned short phentsize;
  unsigned short phnum;
} elf_header;

typedef struct {
  unsigned int type;
  unsigned int flags;
  unsigned long offset;
  unsigned long vaddr;
  unsigned long paddr;
  unsigned long filesz;
  unsigned long memsz;
  unsigned long align;
} elf_phdr;

extern const elf_header __ehdr_start;

static char *host_environment[] = {
  "BENDRTS=--threads 1 --gpu off",
  "NELIBC_HOST=linux",
  0
};

static int host_text_equal(const char *a, const char *b) {
  while (*a != 0 && *a == *b) {
    a++;
    b++;
  }
  return *a == *b;
}

void host_start(long *initial_stack) {
  static nelibc_host host;
  int argc = (int)initial_stack[0];
  char **argv = (char **)(initial_stack + 1);
  char **real_environment = argv + argc + 1;
  const elf_phdr *phdr = (const elf_phdr *)((const char *)&__ehdr_start + __ehdr_start.phoff);
  unsigned short index;

  host.console_write = host_console_write;
  host.memory_reserve = host_memory_reserve;
  host.memory_protect = host_memory_protect;
  host.memory_release = host_memory_release;
  host.clock_monotonic = host_clock_monotonic;
  host.clock_realtime = host_clock_realtime;
  host.sleep = host_sleep;
  host.thread_area_set = host_thread_area_set;
  host.exit = host_exit;
  host.unsupported = host_unsupported;
  host.environment = host_environment;
  host.cpu_count = 2;
  host.tls.align = 16;
  for (index = 0; index < __ehdr_start.phnum; index++) {
    if (phdr[index].type == 7) { /* PT_TLS */
      host.tls.start = (const void *)phdr[index].vaddr;
      host.tls.filesz = phdr[index].filesz;
      host.tls.memsz = phdr[index].memsz;
      host.tls.align = phdr[index].align;
    }
  }
  for (index = 0; real_environment[index] != 0; index++)
    if (host_text_equal(real_environment[index], "NELIBC_HOST_STATS=1")) host_stats = 1;
  host_install_faults();
  nelibc_host_run(&host, main, argc, argv);
}

__asm__(".text\n"
        ".globl _start\n"
        ".type _start,@function\n"
        "_start:\n"
        "  xorq %rbp, %rbp\n"
        "  movq %rsp, %rdi\n"
        "  andq $-16, %rsp\n"
        "  call host_start\n"
        "  ud2\n"
        ".size _start, .-_start\n"
        ".section .note.GNU-stack,\"\",@progbits\n");
