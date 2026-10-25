#define _GNU_SOURCE
#include "string.h"
#include <stdint.h>
#include <stdio.h>
#include <sys/mman.h>
#include <unistd.h>

static unsigned char source[640];
static unsigned char left[704];
static unsigned char right[704];
static unsigned char long_left[65552];
static unsigned char long_right[65552];
static const int boundary_values[] = {-257, -1, 256, 511};

static void *ref_memchr(const void *memory, int value, size_t size) {
  const unsigned char *bytes = memory;
  unsigned char target = (unsigned char)value;
  for (size_t i = 0; i < size; ++i) if (bytes[i] == target) return (void *)(bytes + i);
  return NULL;
}

static void *ref_memrchr(const void *memory, int value, size_t size) {
  const unsigned char *bytes = memory;
  unsigned char target = (unsigned char)value;
  while (size != 0) {
    --size;
    if (bytes[size] == target) return (void *)(bytes + size);
  }
  return NULL;
}

static int ref_memcmp(const void *a, const void *b, size_t size) {
  const unsigned char *left_bytes = a;
  const unsigned char *right_bytes = b;
  for (size_t i = 0; i < size; ++i) {
    if (left_bytes[i] != right_bytes[i]) return (int)left_bytes[i] - (int)right_bytes[i];
  }
  return 0;
}

static void *ref_memmove(void *destination, const void *source_bytes, size_t size) {
  unsigned char *dst = destination;
  const unsigned char *src = source_bytes;
  if ((uintptr_t)dst <= (uintptr_t)src || (uintptr_t)dst - (uintptr_t)src >= size) {
    for (size_t i = 0; i < size; ++i) dst[i] = src[i];
  } else {
    while (size != 0) {
      --size;
      dst[size] = src[size];
    }
  }
  return destination;
}

static void *ref_memccpy(void *destination, const void *source_bytes, int value, size_t size) {
  unsigned char *dst = destination;
  const unsigned char *src = source_bytes;
  unsigned char target = (unsigned char)value;
  for (size_t i = 0; i < size; ++i) {
    dst[i] = src[i];
    if (src[i] == target) return dst + i + 1;
  }
  return NULL;
}

static void *ref_memmem(const void *haystack, size_t haystack_size,
                        const void *needle, size_t needle_size) {
  const unsigned char *h = haystack;
  const unsigned char *n = needle;
  if (needle_size == 0) return (void *)haystack;
  if (needle_size > haystack_size) return NULL;
  for (size_t i = 0; i <= haystack_size - needle_size; ++i) {
    if (ref_memcmp(h + i, n, needle_size) == 0) return (void *)(h + i);
  }
  return NULL;
}

static int fail(const char *name, size_t offset, size_t size) {
  fprintf(stderr, "%s mismatch at offset %zu, size %zu\n", name, offset, size);
  return 1;
}

static int long_memmem_checks(void) {
  static const size_t lengths[] = {8, 16, 31, 32, 64, 127, 128, 255, 256, 512};
  unsigned char haystack[2048];
  unsigned char needle[512];

  for (size_t k = 0; k < sizeof lengths / sizeof lengths[0]; ++k) {
    size_t n = lengths[k];

    /* Periodic absent match is a pathological case for naive search. */
    memset(haystack, 'a', sizeof haystack);
    memset(needle, 'a', n);
    needle[n - 1] = 'b';
    if (memmem(haystack, sizeof haystack, needle, n) !=
        ref_memmem(haystack, sizeof haystack, needle, n)) return fail("memmem periodic absent", k, n);

    /* Periodic match at the final legal start exercises both search edges. */
    memset(needle, 'a', n);
    if (memmem(haystack, sizeof haystack, needle, n) != haystack) return fail("memmem periodic found", k, n);

    for (size_t i = 0; i < sizeof haystack; ++i) haystack[i] = (unsigned char)(i * 73u + i / 7u + k * 19u);
    for (size_t i = 0; i < n; ++i) needle[i] = haystack[i * 3u + 11u];
    memcpy(haystack + sizeof haystack - n, needle, n);
    if (memmem(haystack, sizeof haystack, needle, n) !=
        ref_memmem(haystack, sizeof haystack, needle, n)) return fail("memmem long found", k, n);

    memset(needle, 0xFE, n);
    if (memmem(haystack, sizeof haystack, needle, n) !=
        ref_memmem(haystack, sizeof haystack, needle, n)) return fail("memmem long absent", k, n);
  }

  /* Exercise the large-haystack missing-tail filter and a tail-only hit. */
  static const size_t large_lengths[] = {4, 8, 16, 64, 256};
  static unsigned char large_haystack[65536];
  static unsigned char large_needle[256];
  for (size_t k = 0; k < sizeof large_lengths / sizeof large_lengths[0]; ++k) {
    size_t n = large_lengths[k];
    memset(large_haystack, 'a', sizeof large_haystack);
    memset(large_needle, 'a', n);
    large_needle[n - 1] = 'b';
    if (memmem(large_haystack, sizeof large_haystack, large_needle, n) != NULL)
      return fail("memmem large missing tail", k, n);

    memset(large_haystack, 0x11, sizeof large_haystack);
    for (size_t i = 0; i < n; ++i) large_needle[i] = (unsigned char)(0x80u + i);
    memcpy(large_haystack + sizeof large_haystack - n, large_needle, n);
    if (memmem(large_haystack, sizeof large_haystack, large_needle, n) !=
        large_haystack + sizeof large_haystack - n)
      return fail("memmem large tail hit", k, n);
  }
  return 0;
}

static uint32_t random_state = 0x6D656D6Du;

static uint32_t next_random(void) {
  uint32_t value = random_state;
  value ^= value << 13;
  value ^= value >> 17;
  value ^= value << 5;
  random_state = value;
  return value;
}

static int randomized_memmem_checks(void) {
  unsigned char haystack[4096];
  unsigned char needle[512];

  for (size_t trial = 0; trial < 256; ++trial) {
    size_t haystack_size = 1024 + next_random() % (sizeof haystack - 1024 + 1);
    size_t needle_size = 1 + next_random() % sizeof needle;
    if (needle_size > haystack_size) needle_size = haystack_size;

    for (size_t i = 0; i < haystack_size; ++i) haystack[i] = (unsigned char)next_random();
    for (size_t i = 0; i < needle_size; ++i) needle[i] = (unsigned char)next_random();

    if ((trial & 1u) == 0) {
      size_t start = next_random() % (haystack_size - needle_size + 1);
      memcpy(haystack + start, needle, needle_size);
    } else {
      /* Keep 0xFF out of the haystack so this long needle is definitely absent. */
      for (size_t i = 0; i < haystack_size; ++i) haystack[i] &= 0x7Fu;
      memset(needle, 0xFF, needle_size);
    }

    if (memmem(haystack, haystack_size, needle, needle_size) !=
        ref_memmem(haystack, haystack_size, needle, needle_size)) {
      return fail("memmem randomized", trial, needle_size);
    }
  }
  return 0;
}

static int explicit_bzero_checks(void) {
  static const size_t lengths[] = {0, 1, 2, 7, 16, 31, 64, 127, 128, 129, 255, 512};
  static const size_t offsets[] = {0, 1, 7, 15};

  for (size_t oi = 0; oi < sizeof offsets / sizeof offsets[0]; ++oi) {
    size_t offset = offsets[oi];
    for (size_t li = 0; li < sizeof lengths / sizeof lengths[0]; ++li) {
      size_t size = lengths[li];
      for (size_t i = 0; i < sizeof left; ++i)
        left[i] = right[i] = (unsigned char)(i * 73u + i / 9u + li * 17u);

      explicit_bzero(left + offset, size);
      for (size_t i = 0; i < size; ++i) right[offset + i] = 0;
      for (size_t i = 0; i < sizeof left; ++i) {
        if (left[i] != right[i]) return fail("explicit_bzero bounds or bytes", offset, size);
      }
    }
  }
  return 0;
}

static int long_memcmp_checks(void) {
  static const size_t lengths[] = {
      127, 128, 129, 255, 256, 257, 4095, 4096, 4097, 32768, 65535, 65536};
  static const size_t offsets[] = {0, 1, 7, 15};

  for (size_t oi = 0; oi < sizeof offsets / sizeof offsets[0]; ++oi) {
    size_t offset = offsets[oi];
    for (size_t i = 0; i < sizeof long_left; ++i) {
      long_left[i] = (unsigned char)(i * 73u + i / 11u + offset * 19u);
      long_right[i] = long_left[i];
    }
    for (size_t li = 0; li < sizeof lengths / sizeof lengths[0]; ++li) {
      size_t size = lengths[li];
      const unsigned char *a = long_left + offset;
      unsigned char *b = long_right + offset;
      if (memcmp(a, b, size) != 0) return fail("long memcmp equal", offset, size);

      size_t positions[] = {0, 1, 15, 16, 31, 32, 63, 64, 127, 128,
                            255, 256, 4095, 4096, size / 2, size - 1};
      for (size_t pi = 0; pi < sizeof positions / sizeof positions[0]; ++pi) {
        size_t position = positions[pi];
        if (position >= size) continue;
        b[position] = a[position] ^ 0x80u;
        int actual = memcmp(a, b, size);
        int expected = ref_memcmp(a, b, size);
        b[position] = a[position];
        if ((actual > 0) != (expected > 0) || (actual < 0) != (expected < 0)) {
          return fail("long memcmp mismatch", offset + position, size);
        }
      }
    }
  }
  return 0;
}

static int long_memscan_checks(void) {
  static const size_t lengths[] = {127, 128, 129, 255, 256, 257, 2047,
                                   2048, 2049, 4095, 4096, 4097, 32768,
                                   65535, 65536};
  static const size_t offsets[] = {0, 1, 7, 15};

  for (size_t oi = 0; oi < sizeof offsets / sizeof offsets[0]; ++oi) {
    size_t offset = offsets[oi];
    for (size_t li = 0; li < sizeof lengths / sizeof lengths[0]; ++li) {
      size_t size = lengths[li];
      unsigned char *bytes = long_left + offset;
      for (size_t i = 0; i < size; ++i) bytes[i] = 0x11;
      if (memchr(bytes, 0x22, size) != ref_memchr(bytes, 0x22, size) ||
          memrchr(bytes, 0x22, size) != ref_memrchr(bytes, 0x22, size)) {
        return fail("long memscan absent", offset, size);
      }

      size_t positions[] = {0, 1, 15, 16, 31, 32, 63, 64, 127, 128,
                            255, 256, 2047, 2048, 4095, 4096, size / 2,
                            size - 1};
      for (size_t pi = 0; pi < sizeof positions / sizeof positions[0]; ++pi) {
        size_t position = positions[pi];
        if (position >= size) continue;
        bytes[position] = 0xE7;
        if (memchr(bytes, 0xE7, size) != bytes + position ||
            memchr(bytes, 0xE7, size) != ref_memchr(bytes, 0xE7, size)) {
          return fail("long memchr", offset + position, size);
        }
        if (memrchr(bytes, 0xE7, size) != bytes + position ||
            memrchr(bytes, 0xE7, size) != ref_memrchr(bytes, 0xE7, size)) {
          return fail("long memrchr", offset + position, size);
        }
        bytes[position] = 0x11;
      }

      bytes[1] = 0xE7;
      bytes[size - 1] = 0xE7;
      if (memchr(bytes, 0xE7, size) != bytes + 1 ||
          memrchr(bytes, 0xE7, size) != bytes + size - 1) {
        return fail("long memscan order", offset, size);
      }
    }
  }
  return 0;
}

static int long_memmove_checks(void) {
  static const size_t lengths[] = {2047, 2048, 2049, 2559, 2560, 2561,
                                   4095, 4096, 4097, 32768, 65535, 65536};
  for (size_t li = 0; li < sizeof lengths / sizeof lengths[0]; ++li) {
    size_t size = lengths[li];
    for (size_t mode = 0; mode < 3; ++mode) {
      size_t src_offset = mode == 1 ? 8 : 0;
      size_t dst_offset = mode == 0 ? 8 : mode == 1 ? 0 : 32768;
      if (mode == 2 && size > 32768) continue;
      for (size_t i = 0; i < sizeof long_left; ++i) {
        long_left[i] = long_right[i] = (unsigned char)(i * 73u + i / 11u + li * 19u);
      }
      unsigned char *dst = long_left + dst_offset;
      unsigned char *expected = long_right + dst_offset;
      const unsigned char *src = long_left + src_offset;
      const unsigned char *expected_src = long_right + src_offset;
      if (memmove(dst, src, size) != dst) return fail("long memmove return", mode, size);
      ref_memmove(expected, expected_src, size);
      for (size_t i = 0; i < sizeof long_left; ++i) {
        if (long_left[i] != long_right[i]) return fail("long memmove bytes", mode, size);
      }
    }
  }
  return 0;
}

static void *guarded_page(size_t page_size, unsigned char **middle);

static int guard_page_memmove_overlap_checks(void) {
  static const size_t lengths[] = {2048, 2049, 4096};
  long page_result = sysconf(_SC_PAGESIZE);
  if (page_result <= 0) return fail("overlap guard page size", 0, 0);
  size_t page_size = (size_t)page_result;
  unsigned char *mapping = mmap(NULL, page_size * 4, PROT_NONE,
                                MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
  if (mapping == MAP_FAILED) return fail("overlap guard page setup", 0, 0);
  unsigned char *page = mapping + page_size;
  size_t accessible_size = page_size * 2;
  if (mprotect(page, accessible_size, PROT_READ | PROT_WRITE) != 0) {
    munmap(mapping, page_size * 4);
    return fail("overlap guard page protection", 0, 0);
  }

  for (size_t li = 0; li < sizeof lengths / sizeof lengths[0]; ++li) {
    size_t size = lengths[li];
    for (size_t direction = 0; direction < 2; ++direction) {
      for (size_t i = 0; i < accessible_size; ++i)
        page[i] = (unsigned char)(i * 73u + i / 11u + li * 19u + direction * 31u);
      unsigned char expected[4097];
      size_t window_offset = accessible_size - size - 1;
      size_t window_size = size + 1;
      for (size_t i = 0; i < window_size; ++i)
        expected[i] = page[window_offset + i];

      unsigned char *src = direction == 0 ? page + accessible_size - size - 1
                                           : page + accessible_size - size;
      unsigned char *dst = direction == 0 ? src + 1 : src - 1;
      size_t src_offset = (size_t)(src - page) - window_offset;
      size_t dst_offset = (size_t)(dst - page) - window_offset;
      if (memmove(dst, src, size) != dst) {
        munmap(mapping, page_size * 4);
        return fail("overlap guard memmove return", direction, size);
      }
      ref_memmove(expected + dst_offset, expected + src_offset, size);
      for (size_t i = 0; i < window_size; ++i) {
        if (page[window_offset + i] != expected[i]) {
          munmap(mapping, page_size * 4);
          return fail("overlap guard memmove bytes", direction, size);
        }
      }
    }
  }

  munmap(mapping, page_size * 4);
  return 0;
}

static void *guarded_page(size_t page_size, unsigned char **middle) {
  unsigned char *mapping = mmap(NULL, page_size * 3, PROT_NONE,
                                MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
  if (mapping == MAP_FAILED) return MAP_FAILED;
  *middle = mapping + page_size;
  if (mprotect(*middle, page_size, PROT_READ | PROT_WRITE) != 0) {
    munmap(mapping, page_size * 3);
    return MAP_FAILED;
  }
  return mapping;
}

static int guard_page_explicit_bzero_checks(size_t page_size) {
  unsigned char *page;
  void *mapping = guarded_page(page_size, &page);
  if (mapping == MAP_FAILED) return fail("explicit_bzero guard page setup", 0, 0);

  for (size_t edge = 0; edge < 2; ++edge) {
    for (size_t size = 0; size <= 512; ++size) {
      unsigned char *start = edge == 0 ? page : page + page_size - size;
      for (size_t i = 0; i < size; ++i) start[i] = 0xA5;
      explicit_bzero(start, size);
      for (size_t i = 0; i < size; ++i) {
        if (start[i] != 0) {
          munmap(mapping, page_size * 3);
          return fail("explicit_bzero guard page bytes", edge, size);
        }
      }
    }
  }

  munmap(mapping, page_size * 3);
  return 0;
}

static int guard_page_checks(void) {
  long page_result = sysconf(_SC_PAGESIZE);
  if (page_result <= 0) return fail("guard page size", 0, 0);
  size_t page_size = (size_t)page_result;
  if (guard_page_explicit_bzero_checks(page_size) != 0) return 1;
  unsigned char *source_page;
  unsigned char *destination_page;
  void *source_mapping = guarded_page(page_size, &source_page);
  void *destination_mapping = guarded_page(page_size, &destination_page);
  if (source_mapping == MAP_FAILED || destination_mapping == MAP_FAILED) {
    if (source_mapping != MAP_FAILED) munmap(source_mapping, page_size * 3);
    if (destination_mapping != MAP_FAILED) munmap(destination_mapping, page_size * 3);
    return fail("guard page setup", 0, 0);
  }

  unsigned char compare[512];
  unsigned char needle[] = {0xF1, 0xF2, 0xF3};
  size_t fail_edge = 0;
  size_t fail_size = 0;
  for (size_t edge = 0; edge < 2; ++edge) {
    for (size_t size = 1; size <= sizeof compare; ++size) {
      fail_edge = edge;
      fail_size = size;
      unsigned char *src = edge == 0 ? source_page : source_page + page_size - size;
      unsigned char *dst = edge == 0 ? destination_page : destination_page + page_size - size;
      for (size_t i = 0; i < size; ++i) src[i] = 0x11;
      src[0] = 0xEF;
      if (size > 1) src[size - 1] = 0xEE;

      if (memchr(src, size == 1 ? 0xEF : 0xEE, size) != src + size - 1) goto fail_memchr;
      if (memrchr(src, 0xEF, size) != src) goto fail_memrchr;
      if (memchr(src, 0x22, size) != NULL) goto fail_memchr_absent;
      if (memrchr(src, 0x22, size) != NULL) goto fail_memrchr_absent;

      for (size_t i = 0; i < size; ++i) compare[i] = src[i];
      if (memcmp(src, compare, size) != 0) goto fail_memcmp_equal;
      compare[size - 1] ^= 1;
      if ((memcmp(src, compare, size) > 0) != (ref_memcmp(src, compare, size) > 0) ||
          (memcmp(src, compare, size) < 0) != (ref_memcmp(src, compare, size) < 0)) {
        goto fail_memcmp_mismatch;
      }

      for (size_t i = 0; i < size; ++i) dst[i] = 0xA5;
      if (memcpy(dst, src, size) != dst) goto fail_memcpy_return;
      for (size_t i = 0; i < size; ++i) if (dst[i] != src[i]) goto fail_memcpy_bytes;
      for (size_t i = 0; i < size; ++i) dst[i] = 0xA5;
      if (mempcpy(dst, src, size) != dst + size) goto fail_mempcpy_return;
      for (size_t i = 0; i < size; ++i) if (dst[i] != src[i]) goto fail_mempcpy_bytes;
      for (size_t i = 0; i < size; ++i) dst[i] = 0xA5;
      if (memmove(dst, src, size) != dst) goto fail_memmove_return;
      for (size_t i = 0; i < size; ++i) if (dst[i] != src[i]) goto fail_memmove_bytes;
      for (size_t i = 0; i < size; ++i) dst[i] = 0xA5;
      if (memset(dst, 0xD3, size) != dst) goto fail_memset_return;
      for (size_t i = 0; i < size; ++i) if (dst[i] != 0xD3) goto fail_memset_bytes;

      for (size_t i = 0; i < size; ++i) dst[i] = 0xA5;
      void *copied = memccpy(dst, src, size == 1 ? 0xEF : 0xEE, size);
      if (copied != dst + size) goto fail_memccpy_return;
      for (size_t i = 0; i < size; ++i) if (dst[i] != src[i]) goto fail_memccpy_bytes;

      if (memmem(src, size, needle, sizeof needle) != NULL) goto fail_memmem_absent;
      if (size >= sizeof needle) {
        src[size - 3] = needle[0];
        src[size - 2] = needle[1];
        src[size - 1] = needle[2];
        if (memmem(src, size, needle, sizeof needle) != src + size - 3) goto fail_memmem_found;
      }
    }
  }

  /* Exercise the TinyCC 64-byte SSE2 memcpy entry at every alignment. */
  for (size_t source_offset = 0; source_offset < 16; ++source_offset) {
    for (size_t destination_offset = 0; destination_offset < 16;
         ++destination_offset) {
      unsigned char expected[64];
      unsigned char *src = source_page + page_size - sizeof expected - source_offset;
      unsigned char *dst = destination_page + destination_offset;
      fail_edge = source_offset;
      fail_size = destination_offset;
      for (size_t i = 0; i < sizeof expected; ++i) {
        expected[i] = (unsigned char)(i * 37u + source_offset * 11u + destination_offset);
        src[i] = expected[i];
      }
      if (memcpy(dst, src, sizeof expected) != dst) goto fail_memcpy_return;
      for (size_t i = 0; i < sizeof expected; ++i) {
        if (dst[i] != expected[i]) goto fail_memcpy_bytes;
      }

      src = source_page + source_offset;
      dst = destination_page + page_size - sizeof expected - destination_offset;
      for (size_t i = 0; i < sizeof expected; ++i) {
        expected[i] = (unsigned char)(i * 53u + source_offset + destination_offset * 7u);
        src[i] = expected[i];
      }
      if (memcpy(dst, src, sizeof expected) != dst) goto fail_memcpy_return;
      for (size_t i = 0; i < sizeof expected; ++i) {
        if (dst[i] != expected[i]) goto fail_memcpy_bytes;
      }
    }
  }

  munmap(source_mapping, page_size * 3);
  munmap(destination_mapping, page_size * 3);
  return 0;

fail_memchr: return fail("guard memchr", fail_edge, fail_size);
fail_memrchr: return fail("guard memrchr", fail_edge, fail_size);
fail_memchr_absent: return fail("guard memchr absent", fail_edge, fail_size);
fail_memrchr_absent: return fail("guard memrchr absent", fail_edge, fail_size);
fail_memcmp_equal: return fail("guard memcmp equal", fail_edge, fail_size);
fail_memcmp_mismatch: return fail("guard memcmp mismatch", fail_edge, fail_size);
fail_memcpy_return: return fail("guard memcpy return", fail_edge, fail_size);
fail_memcpy_bytes: return fail("guard memcpy bytes", fail_edge, fail_size);
fail_mempcpy_return: return fail("guard mempcpy return", fail_edge, fail_size);
fail_mempcpy_bytes: return fail("guard mempcpy bytes", fail_edge, fail_size);
fail_memmove_return: return fail("guard memmove return", fail_edge, fail_size);
fail_memmove_bytes: return fail("guard memmove bytes", fail_edge, fail_size);
fail_memset_return: return fail("guard memset return", fail_edge, fail_size);
fail_memset_bytes: return fail("guard memset bytes", fail_edge, fail_size);
fail_memccpy_return: return fail("guard memccpy return", fail_edge, fail_size);
fail_memccpy_bytes: return fail("guard memccpy bytes", fail_edge, fail_size);
fail_memmem_absent: return fail("guard memmem absent", fail_edge, fail_size);
fail_memmem_found: return fail("guard memmem found", fail_edge, fail_size);
}

int main(void) {
  for (size_t i = 0; i < sizeof source; ++i) source[i] = (unsigned char)(i * 73u + i / 3u + 19u);

  for (size_t offset = 0; offset < 16; ++offset) {
    for (size_t size = 0; size <= 512; ++size) {
      const unsigned char *src = source + offset;
      for (size_t i = 0; i < sizeof left; ++i) {
        left[i] = 0xA5;
        right[i] = 0xA5;
      }

      if (memcmp(src, src, size) != 0) return fail("memcmp equal", offset, size);
      for (size_t position = 0; position < size; ++position) {
        unsigned char changed = src[position] ^ 0x81;
        for (size_t i = 0; i < size; ++i) right[i] = src[i];
        right[position] = changed;
        if ((memcmp(src, right, size) > 0) != (ref_memcmp(src, right, size) > 0) ||
            (memcmp(src, right, size) < 0) != (ref_memcmp(src, right, size) < 0)) {
          return fail("memcmp mismatch", offset, size);
        }
      }

      for (int target = 0; target < 256; ++target) {
        if (memchr(src, target, size) != ref_memchr(src, target, size)) return fail("memchr", offset, size);
        if (memrchr(src, target, size) != ref_memrchr(src, target, size)) return fail("memrchr", offset, size);
      }
      for (size_t i = 0; i < sizeof boundary_values / sizeof boundary_values[0]; ++i) {
        int target = boundary_values[i];
        if (memchr(src, target, size) != ref_memchr(src, target, size)) return fail("memchr int conversion", offset, size);
        if (memrchr(src, target, size) != ref_memrchr(src, target, size)) return fail("memrchr int conversion", offset, size);
      }

      for (size_t i = 0; i < sizeof left; ++i) left[i] = right[i] = 0xA5;
      for (size_t i = 0; i < size; ++i) right[16 + i] = src[i];
      if (memcpy(left + 16, src, size) != left + 16) return fail("memcpy return", offset, size);
      for (size_t i = 0; i < sizeof left; ++i) if (left[i] != right[i]) return fail("memcpy bytes", offset, size);
      for (size_t i = 0; i < sizeof left; ++i) left[i] = right[i] = 0xA5;
      for (size_t i = 0; i < size; ++i) right[16 + i] = src[i];
      if (mempcpy(left + 16, src, size) != left + 16 + size) return fail("mempcpy return", offset, size);
      for (size_t i = 0; i < sizeof left; ++i) if (left[i] != right[i]) return fail("mempcpy bytes", offset, size);

      memset(left, 0xA5, sizeof left);
      for (size_t i = 0; i < sizeof right; ++i) right[i] = 0xA5;
      if (memset(left + 16, (int)(offset + size), size) != left + 16) {
        return fail("memset return", offset, size);
      }
      for (size_t i = 0; i < size; ++i) right[16 + i] = (unsigned char)(offset + size);
      for (size_t i = 0; i < sizeof left; ++i) if (left[i] != right[i]) return fail("memset bytes", offset, size);

      for (size_t direction = 0; direction < 2; ++direction) {
        for (size_t i = 0; i < sizeof left; ++i) {
          left[i] = (unsigned char)(i * 29u + 7u);
          right[i] = left[i];
        }
        size_t dst_offset = direction == 0 ? offset + 2 : offset;
        size_t src_offset = direction == 0 ? offset : offset + 2;
        if (memmove(left + dst_offset, left + src_offset, size) != left + dst_offset) {
          return fail("memmove return", offset, size);
        }
        ref_memmove(right + dst_offset, right + src_offset, size);
        for (size_t i = 0; i < sizeof left; ++i) if (left[i] != right[i]) return fail("memmove bytes", offset, size);
      }

      for (int target = 0; target < 256; ++target) {
        for (size_t i = 0; i < sizeof left; ++i) left[i] = right[i] = 0xA5;
        void *actual = memccpy(left + 16, src, target, size);
        void *expected = ref_memccpy(right + 16, src, target, size);
        if ((actual == NULL) != (expected == NULL) ||
            (actual != NULL && (size_t)((unsigned char *)actual - (left + 16)) !=
                                   (size_t)((unsigned char *)expected - (right + 16)))) {
          return fail("memccpy return", offset, size);
        }
        for (size_t i = 0; i < sizeof left; ++i) if (left[i] != right[i]) return fail("memccpy bytes", offset, size);
      }
      for (size_t target_index = 0; target_index < sizeof boundary_values / sizeof boundary_values[0]; ++target_index) {
        int target = boundary_values[target_index];
        for (size_t i = 0; i < sizeof left; ++i) left[i] = right[i] = 0xA5;
        void *actual = memccpy(left + 16, src, target, size);
        void *expected = ref_memccpy(right + 16, src, target, size);
        if ((actual == NULL) != (expected == NULL) ||
            (actual != NULL && (size_t)((unsigned char *)actual - (left + 16)) !=
                                   (size_t)((unsigned char *)expected - (right + 16)))) {
          return fail("memccpy int conversion", offset, size);
        }
        for (size_t i = 0; i < sizeof left; ++i) if (left[i] != right[i]) return fail("memccpy int conversion bytes", offset, size);
      }

      for (size_t needle_size = 0; needle_size <= 32; ++needle_size) {
        const unsigned char *needle = src + (size > needle_size ? (size - needle_size) / 2 : 0);
        if (memmem(src, size, needle, needle_size) != ref_memmem(src, size, needle, needle_size)) {
          return fail("memmem found", offset, size);
        }
        unsigned char absent[33];
        for (size_t i = 0; i < sizeof absent; ++i) absent[i] = 0xFF;
        if (memmem(src, size, absent, needle_size) != ref_memmem(src, size, absent, needle_size)) {
          return fail("memmem absent", offset, size);
        }
      }
    }
  }

  if (explicit_bzero_checks() != 0) return 1;
  if (guard_page_checks() != 0) return 1;
  if (long_memcmp_checks() != 0) return 1;
  if (long_memscan_checks() != 0) return 1;
  if (long_memmove_checks() != 0) return 1;
  if (guard_page_memmove_overlap_checks() != 0) return 1;
  if (long_memmem_checks() != 0) return 1;
  if (randomized_memmem_checks() != 0) return 1;
  puts("Nelua memory API matches bytewise references, explicit_bzero bounds and guard pages, guarded overlap memmove, long memory operations, and 256 randomized memmem cases");
  return 0;
}
