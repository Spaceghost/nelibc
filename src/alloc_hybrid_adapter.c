#include <stddef.h>
#include <stdlib.h>

#include "nelibc_alloc.h"

#if defined(NELIBC_STATIC_MUSL)
extern void __real_free(void *pointer);
extern void *__real_realloc(void *pointer, size_t size);
#define nelibc_host_free __real_free
#define nelibc_host_realloc __real_realloc
#else
/* Bypass executable-level malloc interposition for host-owned pointers. */
extern void __libc_free(void *pointer);
extern void *__libc_realloc(void *pointer, size_t size);
#define nelibc_host_free __libc_free
#define nelibc_host_realloc __libc_realloc
#endif

/* Route Nelibc mappings to their owner and all other pointers to host libc. */
void nelibc_hybrid_free(void *pointer) {
  if (nelibc_allocator_owns(pointer)) {
    nelibc_free(pointer);
  } else {
    nelibc_host_free(pointer);
  }
}

void *nelibc_hybrid_realloc(void *pointer, size_t size) {
  if (pointer == NULL) {
    /* Match public malloc in every hybrid profile. */
    return nelibc_malloc(size);
  }
  if (nelibc_allocator_owns(pointer)) {
    return nelibc_realloc(pointer, size);
  }
  return nelibc_host_realloc(pointer, size);
}

#if defined(NELIBC_STATIC_MUSL)
/* Linker --wrap=free/realloc bridges public calls into the hybrid dispatcher. */
void __wrap_free(void *pointer) { nelibc_hybrid_free(pointer); }
void *__wrap_realloc(void *pointer, size_t size) {
  return nelibc_hybrid_realloc(pointer, size);
}
#endif
