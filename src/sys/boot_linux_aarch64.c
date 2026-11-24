/* Static ET_EXEC startup for the initial AArch64 thread. */
#include <asm/errno.h>
#include <asm/unistd.h>
#include <elf.h>
#include <linux/mman.h>
#include <stdint.h>

#include "thread_linux_aarch64.h"

#define TCB_SIZE 16UL

typedef void (*init_fn)(int, char **, char **);
extern init_fn __preinit_array_start[] __attribute__((weak));
extern init_fn __preinit_array_end[] __attribute__((weak));
extern init_fn __init_array_start[] __attribute__((weak));
extern init_fn __init_array_end[] __attribute__((weak));
#ifdef NELIBC_STANDALONE
typedef void (*fini_fn)(void);
extern fini_fn __fini_array_start[] __attribute__((weak));
extern fini_fn __fini_array_end[] __attribute__((weak));
extern fini_fn nelibc_exit_fini_hook;
extern void nelibc_stdio_init(void);
extern _Noreturn void exit(int);

static void run_fini_array(void) {
  fini_fn *fn = __fini_array_end;
  while (fn != __fini_array_start) (*--fn)();
}
#endif
extern long nelibc_raw_syscall6(unsigned long, unsigned long, unsigned long,
                                unsigned long, unsigned long, unsigned long,
                                unsigned long);
extern int main(int, char **, char **);
char **environ;
unsigned long nelibc_aarch64_pagesize = 4096;

void **nelibc_aarch64_thread_slots(void *tp) {
  return (void **)((uintptr_t)tp - NELIBC_AARCH64_THREAD_SLOTS * sizeof(void *));
}

static const Elf64_Phdr *tls_template;

static int find_tls_template(const Elf64_auxv_t *auxv) {
  const Elf64_Phdr *ph = 0;
  unsigned long count = 0;
  unsigned long entry_size = 0;
  for (; auxv->a_type != AT_NULL; ++auxv) {
    switch (auxv->a_type) {
      case AT_PAGESZ:
        if (auxv->a_un.a_val < 4096 || (auxv->a_un.a_val & (auxv->a_un.a_val - 1)))
          return -1;
        nelibc_aarch64_pagesize = auxv->a_un.a_val;
        break;
      case AT_PHDR: ph = (const Elf64_Phdr *)auxv->a_un.a_val; break;
      case AT_PHNUM: count = auxv->a_un.a_val; break;
      case AT_PHENT: entry_size = auxv->a_un.a_val; break;
    }
  }
  if (!ph || !count || entry_size != sizeof *ph) return -1;
  const Elf64_Phdr *tls = 0;
  for (unsigned long i = 0; i < count; ++i) {
    if (ph[i].p_type != PT_TLS) continue;
    if (tls) return -1;
    tls = &ph[i];
  }

  tls_template = tls;
  return 0;
}

long nelibc_aarch64_allocate_tls(struct nelibc_aarch64_tls *result) {
  const Elf64_Phdr *tls = tls_template;
  unsigned long align = tls && tls->p_align ? tls->p_align : 1;
  unsigned long size = tls ? tls->p_memsz : 0;
  unsigned long initialized = tls ? tls->p_filesz : 0;
  uintptr_t address = tls ? tls->p_vaddr : 0;
  if ((align & (align - 1)) || initialized > size) return -EINVAL;

  /* Variant 1 puts the TLS image after a 16-byte TCB. Preserve the
   * ELF template's alignment residue when computing the padding. */
  unsigned long padding = (address - TCB_SIZE) & (align - 1);
  if (align < TCB_SIZE) align = TCB_SIZE;
  const unsigned long prefix = NELIBC_AARCH64_THREAD_SLOTS * sizeof(void *);
  unsigned long overhead, allocation;
  if (__builtin_add_overflow(prefix + TCB_SIZE + padding, align - 1, &overhead) ||
      __builtin_add_overflow(overhead, size, &allocation)) return -ENOMEM;
  long mapped = nelibc_raw_syscall6(__NR_mmap, 0, allocation,
      PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_ANONYMOUS, -1UL, 0);
  if ((unsigned long)mapped >= -4095UL) return mapped;
  uintptr_t mapping = (uintptr_t)mapped;
  uintptr_t tp = (mapping + prefix + align - 1) & ~(align - 1);
  unsigned char *image = (unsigned char *)(tp + TCB_SIZE + padding);
  const unsigned char *template = (const unsigned char *)address;
  /* Anonymous mappings arrive zero-filled, including the TCB and .tbss. */
  for (unsigned long i = 0; i < initialized; ++i) image[i] = template[i];
  result->mapping = (void *)mapping;
  result->size = allocation;
  result->tp = (void *)tp;
  return 0;
}

int nelibc_boot_linux_aarch64(int argc, char **argv, char **envp) {
  environ = envp;
  char **end = envp;
  while (*end) ++end;
  struct nelibc_aarch64_tls tls;
  if (find_tls_template((const Elf64_auxv_t *)(end + 1)) ||
      nelibc_aarch64_allocate_tls(&tls)) return 127;
  __asm__ volatile("msr tpidr_el0, %0" : : "r"(tls.tp) : "memory");

#ifdef NELIBC_STANDALONE
  nelibc_stdio_init();
  nelibc_exit_fini_hook = run_fini_array;
#endif
  for (init_fn *fn = __preinit_array_start; fn != __preinit_array_end; ++fn)
    (*fn)(argc, argv, envp);
  for (init_fn *fn = __init_array_start; fn != __init_array_end; ++fn)
    (*fn)(argc, argv, envp);
  int result = main(argc, argv, envp);
#ifdef NELIBC_STANDALONE
  exit(result);
#else
  return result;
#endif
}
