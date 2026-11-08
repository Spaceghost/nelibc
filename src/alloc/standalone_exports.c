/* Public allocator names for the no-CRT standalone profile only. Hybrid
   applications keep host malloc/free paired with host-owned pointers.

   Every public name here is a weak alias of a nelibc_standalone_* function,
   so an application that defines malloc, free, ... wins at static link time,
   also when the archive is linked with --whole-archive. The rest of the
   library allocates memory it hands to the caller (strdup, getline,
   asprintf, open_memstream, realpath(NULL), scandir, wordexp, ...) through
   the public names, so a replacement owns all of it.

   Partial replacement follows musl (src/malloc/calloc.c, memalign.c,
   posix_memalign.c and mallocng/glue.h). nelibc counts the allocator as
   replaced when the public malloc does not resolve to this file. Then:
   - calloc calls the public malloc and clears the result itself;
   - aligned_alloc, if not replaced too, fails with ENOMEM rather than
     return nelibc memory the replacement free would receive;
   - posix_memalign and memalign go through the public aligned_alloc when
     the application replaced it, and fail with ENOMEM when only malloc was
     replaced;
   - malloc_usable_size is unsupported: it returns 0 instead of reading
     another allocator's metadata.
   As in musl, replacing malloc without free and realloc is undefined. */

typedef __SIZE_TYPE__ size_t;

extern void *nelibc_malloc(size_t size);
extern void *nelibc_calloc(size_t count, size_t size);
extern void *nelibc_realloc(void *ptr, size_t size);
extern void nelibc_free(void *ptr);
extern void *nelibc_aligned_alloc(size_t alignment, size_t size);
extern int nelibc_posix_memalign(void **output, size_t alignment, size_t size);
extern size_t nelibc_malloc_usable_size(void *ptr);
extern int *nelibc_errno_location(void);
extern void *memset(void *dest, int value, size_t count);

void *malloc(size_t size);
void *aligned_alloc(size_t alignment, size_t size);

#define NELIBC_EINVAL 22
#define NELIBC_ENOMEM 12

void *nelibc_standalone_malloc(size_t size) { return nelibc_malloc(size); }
void *nelibc_standalone_realloc(void *ptr, size_t size) { return nelibc_realloc(ptr, size); }
void nelibc_standalone_free(void *ptr) { nelibc_free(ptr); }
void *nelibc_standalone_aligned_alloc(size_t alignment, size_t size);

/* The public symbols are weak, so the compiler must emit relocations against
   them rather than fold them to the alias targets. Read the addresses through
   volatile objects anyway so no optimiser decides the comparison early. */
static int public_differs(void *public_address, void *own_address) {
  void *volatile public_value = public_address;
  void *volatile own_value = own_address;
  return public_value != own_value;
}

int nelibc_malloc_replaced(void) {
  return public_differs((void *)&malloc, (void *)&nelibc_standalone_malloc);
}

static int aligned_alloc_replaced(void) {
  return public_differs((void *)&aligned_alloc, (void *)&nelibc_standalone_aligned_alloc);
}

static int power_of_two(size_t value) { return value != 0 && (value & (value - 1)) == 0; }

void *nelibc_standalone_calloc(size_t count, size_t size) {
  if (!nelibc_malloc_replaced()) return nelibc_calloc(count, size);
  if (count != 0 && size > (size_t)-1 / count) {
    *nelibc_errno_location() = NELIBC_ENOMEM;
    return 0;
  }
  void *memory = malloc(count * size);
  if (memory != 0) memset(memory, 0, count * size);
  return memory;
}

void *nelibc_standalone_aligned_alloc(size_t alignment, size_t size) {
  if (!nelibc_malloc_replaced()) return nelibc_aligned_alloc(alignment, size);
  /* Same argument checks as the nelibc path, then refuse to mix allocators. */
  if (!power_of_two(alignment) || size % alignment != 0) {
    *nelibc_errno_location() = NELIBC_EINVAL;
    return 0;
  }
  *nelibc_errno_location() = NELIBC_ENOMEM;
  return 0;
}

int nelibc_standalone_posix_memalign(void **output, size_t alignment, size_t size) {
  if (alignment < sizeof(void *) || !power_of_two(alignment)) return NELIBC_EINVAL;
  if (aligned_alloc_replaced()) {
    /* posix_memalign reports through its result and leaves errno alone. */
    int *error = nelibc_errno_location();
    int saved_errno = *error;
    void *memory = aligned_alloc(alignment, size);
    int result = memory == 0 ? *error : 0;
    *error = saved_errno;
    if (memory == 0) return result != 0 ? result : NELIBC_ENOMEM;
    *output = memory;
    return 0;
  }
  if (nelibc_malloc_replaced()) return NELIBC_ENOMEM;
  return nelibc_posix_memalign(output, alignment, size);
}

void *nelibc_standalone_memalign(size_t alignment, size_t size) {
  if (!power_of_two(alignment)) {
    *nelibc_errno_location() = NELIBC_EINVAL;
    return 0;
  }
  if (alignment < sizeof(void *)) alignment = sizeof(void *);
  if (aligned_alloc_replaced()) return aligned_alloc(alignment, size);
  void *memory = 0;
  int error = nelibc_standalone_posix_memalign(&memory, alignment, size);
  if (error != 0) {
    *nelibc_errno_location() = error;
    return 0;
  }
  return memory;
}

size_t nelibc_standalone_malloc_usable_size(void *ptr) {
  if (ptr == 0 || nelibc_malloc_replaced()) return 0;
  return nelibc_malloc_usable_size(ptr);
}

/* TinyCC accepts but ignores the alias attribute, so it gets the same weak
   alias from the assembler. */
#ifdef __TINYC__
#define NELIBC_WEAK_ALIAS(name, target) __asm__(".weak " #name "\n.set " #name ", " #target);
NELIBC_WEAK_ALIAS(malloc, nelibc_standalone_malloc)
NELIBC_WEAK_ALIAS(calloc, nelibc_standalone_calloc)
NELIBC_WEAK_ALIAS(realloc, nelibc_standalone_realloc)
NELIBC_WEAK_ALIAS(free, nelibc_standalone_free)
NELIBC_WEAK_ALIAS(aligned_alloc, nelibc_standalone_aligned_alloc)
NELIBC_WEAK_ALIAS(posix_memalign, nelibc_standalone_posix_memalign)
NELIBC_WEAK_ALIAS(memalign, nelibc_standalone_memalign)
NELIBC_WEAK_ALIAS(malloc_usable_size, nelibc_standalone_malloc_usable_size)
#else
void *malloc(size_t size) __attribute__((weak, alias("nelibc_standalone_malloc")));
void *calloc(size_t count, size_t size) __attribute__((weak, alias("nelibc_standalone_calloc")));
void *realloc(void *ptr, size_t size) __attribute__((weak, alias("nelibc_standalone_realloc")));
void free(void *ptr) __attribute__((weak, alias("nelibc_standalone_free")));
void *aligned_alloc(size_t alignment, size_t size)
    __attribute__((weak, alias("nelibc_standalone_aligned_alloc")));
int posix_memalign(void **output, size_t alignment, size_t size)
    __attribute__((weak, alias("nelibc_standalone_posix_memalign")));
void *memalign(size_t alignment, size_t size) __attribute__((weak, alias("nelibc_standalone_memalign")));
size_t malloc_usable_size(void *ptr) __attribute__((weak, alias("nelibc_standalone_malloc_usable_size")));
#endif
