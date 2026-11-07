#define _GNU_SOURCE
#include <stddef.h>
#include <stdlib.h>
#if !defined(NELIBC_STATIC_MUSL)
#include <dlfcn.h>
#endif

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

#if !defined(NELIBC_STATIC_MUSL)
/* Pointers Nelibc owns answer from their header; anything else is asked of
 * the host allocator, found past our own definition of the symbol. */
size_t nelibc_hybrid_malloc_usable_size(void *pointer) {
  if (pointer == NULL) return 0;
  if (nelibc_allocator_owns(pointer)) return nelibc_malloc_usable_size(pointer);
  static size_t (*host_usable_size)(void *);
  if (host_usable_size == NULL)
    host_usable_size = (size_t (*)(void *))dlsym(RTLD_NEXT, "malloc_usable_size");
  return host_usable_size != NULL ? host_usable_size(pointer) : 0;
}
#endif

#if defined(NELIBC_STATIC_MUSL)
/* Static musl: every malloc in the image is ours, so a foreign pointer has
 * no owner to ask. */
size_t nelibc_hybrid_malloc_usable_size(void *pointer) {
  if (pointer == NULL || !nelibc_allocator_owns(pointer)) return 0;
  return nelibc_malloc_usable_size(pointer);
}

/* Linker --wrap=free/realloc bridges public calls into the hybrid dispatcher. */
void __wrap_free(void *pointer) { nelibc_hybrid_free(pointer); }
void *__wrap_realloc(void *pointer, size_t size) {
  return nelibc_hybrid_realloc(pointer, size);
}
#endif
