/* Host profile boot: bind the host table, build the initial thread's static
 * TLS block, install the thread pointer, and start stdio. This replaces the
 * Linux entry point (src/sys/start_linux_x86_64_standalone.S), which parsed
 * the ELF auxiliary vector and issued its own syscalls. */
#include "nelibc_host.h"

#define NELIBC_HOST_EINVAL 22
#define NELIBC_HOST_ENOMEM 12

/* Declared extern by environment_adapter.c and the Linux start files. */
char **environ;

static char *nelibc_host_empty_environment[1];
static const nelibc_host *nelibc_host_table;
static void *nelibc_host_thread_pointer;

extern void nelibc_stdio_init(void);
extern void exit(int status);

typedef struct nelibc_tls_layout_record {
  void *image_start;
  size_t image_filesz;
  size_t image_memsz;
  size_t image_align;
} nelibc_tls_layout_record;

const nelibc_host *nelibc_host_current(void) {
  return nelibc_host_table;
}

void *nelibc_host_thread_pointer_current(void) {
  return nelibc_host_thread_pointer;
}

void nelibc_host_thread_pointer_record(void *thread_pointer) {
  nelibc_host_thread_pointer = thread_pointer;
}

static size_t nelibc_host_cstring_length(const char *text) {
  size_t length = 0;
  while (text[length] != 0) length++;
  return length;
}

void nelibc_host_fail(const char *message, int status) {
  const nelibc_host *host = nelibc_host_table;
  if (host != NULL && host->console_write != NULL && message != NULL) {
    size_t length = nelibc_host_cstring_length(message);
    size_t offset = 0;
    while (offset < length) {
      long written = host->console_write(host->context, 2,
                                         message + offset, length - offset);
      if (written <= 0) break;
      offset += (size_t)written;
    }
  }
  if (host != NULL && host->exit != NULL) host->exit(host->context, status);
  for (;;) {
  }
}

/* thread.nelua asks for the TLS template when it spawns a thread. */
void nelibc_tls_layout(nelibc_tls_layout_record *layout) {
  const nelibc_host *host = nelibc_host_table;
  layout->image_start = NULL;
  layout->image_filesz = 0;
  layout->image_memsz = 0;
  layout->image_align = 16;
  if (host == NULL) return;
  layout->image_start = (void *)host->tls.start;
  layout->image_filesz = host->tls.filesz;
  layout->image_memsz = host->tls.memsz;
  layout->image_align = host->tls.align;
}

static int nelibc_host_setup_tls(const nelibc_host *host) {
  size_t align = host->tls.align;
  size_t memsz = host->tls.memsz;
  size_t filesz = host->tls.filesz;
  size_t rounded;
  size_t mapping_size;
  unsigned char *mapping;
  unsigned char *thread_pointer;
  unsigned char *image;
  size_t index;

  if (filesz > memsz) return -NELIBC_HOST_EINVAL;
  if (align == 0) align = 1;
  if ((align & (align - 1)) != 0) return -NELIBC_HOST_EINVAL;
  if (align < 16) align = 16;
  /* x86-64 local-exec offsets are relative to the align-rounded TLS size. */
  rounded = (memsz + align - 1) & ~(align - 1);
  /* Keep a writable TCB after the TLS image: self pointer at FS:0 plus the
   * reserved slots nelibc uses for thread identity, pthread context, and
   * thread keys. */
  mapping_size = rounded + align + 64;
  mapping = host->memory_reserve(host->context, mapping_size,
                                 align > 4096 ? align : 4096);
  if (mapping == NULL) return -NELIBC_HOST_ENOMEM;
  thread_pointer = (unsigned char *)
      (((uintptr_t)mapping + rounded + align - 1) & ~(uintptr_t)(align - 1));
  image = thread_pointer - rounded;
  for (index = 0; index < filesz; index++)
    image[index] = ((const unsigned char *)host->tls.start)[index];
  for (index = filesz; index < rounded; index++) image[index] = 0;
  for (index = 0; index < 64; index++) thread_pointer[index] = 0;
  *(void **)thread_pointer = thread_pointer;
  nelibc_host_thread_pointer = thread_pointer;
  return host->thread_pointer_set(host->context, thread_pointer);
}

int nelibc_host_boot(const nelibc_host *host) {
  int result;
  if (host == NULL || host->console_write == NULL ||
      host->memory_reserve == NULL || host->thread_pointer_set == NULL ||
      host->exit == NULL)
    return -NELIBC_HOST_EINVAL;
  nelibc_host_table = host;
  environ = host->environment != NULL ? host->environment
                                      : nelibc_host_empty_environment;
  result = nelibc_host_setup_tls(host);
  if (result != 0) {
    nelibc_host_table = NULL;
    return result;
  }
  nelibc_stdio_init();
  return 0;
}

void nelibc_host_run(const nelibc_host *host,
                     int (*main)(int, char **, char **),
                     int argc, char **argv) {
  static char *no_arguments[1];
  if (nelibc_host_boot(host) != 0) {
    if (host != NULL && host->exit != NULL)
      host->exit(host->context, NELIBC_HOST_EXIT_BOOT);
    for (;;) {
    }
  }
  if (argv == NULL) {
    argv = no_arguments;
    argc = 0;
  }
  exit(main(argc, argv, environ));
  for (;;) {
  }
}
