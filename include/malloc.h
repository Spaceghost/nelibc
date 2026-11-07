#ifndef NELIBC_MALLOC_WRAPPER_H
#define NELIBC_MALLOC_WRAPPER_H

#ifdef NELIBC_STANDALONE
#include <stddef.h>
#else
#include_next <malloc.h>
#endif

#ifdef __cplusplus
extern "C" void *memalign(size_t alignment, size_t size);
extern "C" size_t malloc_usable_size(void *ptr);
#else
void *memalign(size_t alignment, size_t size);
size_t malloc_usable_size(void *ptr);
#endif

/* Nelibc requires power-of-two alignment; sub-pointer alignment is rounded up. */

#endif
