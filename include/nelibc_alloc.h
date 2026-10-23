#ifndef NELIBC_ALLOC_H
#define NELIBC_ALLOC_H

#include <stddef.h>

/* Nelibc allocator API. Hybrid public free/realloc dispatch by ownership. */
void *nelibc_malloc(size_t size);
void *nelibc_calloc(size_t count, size_t size);
void *nelibc_realloc(void *ptr, size_t size);
void *nelibc_reallocarray(void *ptr, size_t count, size_t size);
void nelibc_free(void *ptr);
void *nelibc_aligned_alloc(size_t alignment, size_t size);
int nelibc_posix_memalign(void **memptr, size_t alignment, size_t size);
/* True only while pointer belongs to a live Nelibc slab or direct mapping. */
int nelibc_allocator_owns(const void *pointer);

/* Hybrid-only helpers for pointers that may belong to Nelibc or host libc. */
#ifndef NELIBC_STANDALONE
void nelibc_hybrid_free(void *pointer);
void *nelibc_hybrid_realloc(void *pointer, size_t size);
#endif

#endif
