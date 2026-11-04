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

static int host_thread_pointer_set(void *context, void *thread_pointer) {
  (void)context;
  return (int)linux_syscall(158, 0x1002, (long)thread_pointer, 0, 0, 0, 0);
}

static void host_exit(void *context, int status) {
  (void)context;
  for (;;) linux_syscall(231, status, 0, 0, 0, 0, 0);
}

static void host_unsupported(void *context, unsigned long number) {
  static const char digits[] = "0123456789";
  char text[48] = "host_linux: unsupported syscall ";
  char number_text[24];
  size_t length = 32;
  size_t count = 0;
  (void)context;
  do {
    number_text[count++] = digits[number % 10];
    number /= 10;
  } while (number != 0);
  while (count > 0) text[length++] = number_text[--count];
  text[length++] = '\n';
  linux_syscall(1, 2, (long)text, (long)length, 0, 0, 0);
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

void host_start(long *initial_stack) {
  static nelibc_host host;
  int argc = (int)initial_stack[0];
  char **argv = (char **)(initial_stack + 1);
  const elf_phdr *phdr = (const elf_phdr *)((const char *)&__ehdr_start + __ehdr_start.phoff);
  unsigned short index;

  host.console_write = host_console_write;
  host.memory_reserve = host_memory_reserve;
  host.memory_protect = host_memory_protect;
  host.memory_release = host_memory_release;
  host.clock_monotonic = host_clock_monotonic;
  host.clock_realtime = host_clock_realtime;
  host.sleep = host_sleep;
  host.thread_pointer_set = host_thread_pointer_set;
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
