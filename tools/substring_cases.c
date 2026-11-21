#define _GNU_SOURCE
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/mman.h>
#include <time.h>
#include <unistd.h>

static char *(*volatile strings)(const char *, const char *) = strstr;
static void *(*volatile bytes)(const void *, size_t, const void *, size_t) = memmem;

static unsigned char *guarded(size_t length) {
  size_t page = sysconf(_SC_PAGESIZE);
  size_t accessible = (length + page - 1) / page * page;
  unsigned char *mapping = mmap(NULL, accessible + page, PROT_READ | PROT_WRITE,
                                MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
  if (mapping == MAP_FAILED || mprotect(mapping + accessible, page, PROT_NONE)) abort();
  return mapping + accessible - length;
}

static uint64_t now(void) {
  struct timespec t;
  if (clock_gettime(CLOCK_MONOTONIC, &t)) abort();
  return (uint64_t)t.tv_sec * 1000000000 + t.tv_nsec;
}

static unsigned char *first_match(unsigned char *h, size_t length,
                                  unsigned char *n, size_t width) {
  if (width <= length) {
    for (size_t i = 0; i <= length - width; ++i) {
      size_t j = 0;
      while (j < width && h[i + j] == n[j]) ++j;
      if (j == width) return h + i;
    }
  }
  return NULL;
}

static void check_short_ranges(void) {
  unsigned char *hend = guarded(257) + 257;
  unsigned char *nend = guarded(97) + 97;
  for (int binary = 0; binary <= 1; ++binary) {
    for (size_t length = 0; length <= 130; ++length) {
      unsigned char *h = hend - length - !binary;
      for (size_t width = 0; width <= 70; ++width) {
        unsigned char *n = nend - width - !binary;
        if (!binary) h[length] = n[width] = 0;
        for (unsigned variant = 0; variant < 5; ++variant) {
          memset(h, 'a', length);
          memset(n, 'a', width);
          if (width) n[(width * variant / 5)] = binary ? 0 : 'b';
          if (variant == 1) {
            for (size_t k = 1; k < length; k += 2) h[k] = 'b';
          }
          if (width <= length && variant >= 2) {
            memcpy(h + length - width, n, width);
            if (variant == 4) memcpy(h + (length - width) / 3, n, width);
          }
          void *found = binary ? bytes(h, length, n, width) : (void *)strings((char *)h, (char *)n);
          if (found != first_match(h, length, n, width)) {
            fprintf(stderr, "wrong offset binary=%d length=%zu width=%zu variant=%u\n",
                    binary, length, width, variant);
            abort();
          }
        }
      }
    }
  }
}

int main(void) {
  check_short_ranges();
  const size_t length = 65536;
  const char *names[] = {"alternating", "runs63", "interior", "text", "random", "binary"};
  const char text[] = "the compiler reads a source file and writes an object. ";
  for (int binary = 0; binary <= 1; ++binary) {
    unsigned char *h = guarded(length + !binary);
    for (int kind = 0; kind < 5 + binary; ++kind) {
      size_t width = kind == 0 ? 2 : kind == 1 ? 64 : kind == 2 ? 128 : 8;
      unsigned char *n = guarded(width + !binary);
      memset(n, 'a', width);
      if (kind == 2) n[width / 2] = 'b';
      if (kind == 3) memcpy(n, "an input", width);
      if (kind == 4) memcpy(n, "q93X!yre", width);
      if (kind == 5) memcpy(n, "aaa\0Xaaa", width);
      if (!binary) n[width] = 0;
      for (int hit = 0; hit < 3; ++hit) {
        uint32_t state = 83;
        for (size_t i = 0; i < length; ++i) {
          state = state * 1664525u + 1013904223u;
          h[i] = kind == 0 ? "ab"[i % 2] : kind == 1 && i % 64 == 63 ? 'b' :
                 kind == 3 ? text[i % (sizeof text - 1)] :
                 kind == 4 ? 1 + ((state >> 16) % 255) : 'a';
        }
        if (!binary) h[length] = 0;
        if (hit) memcpy(h + length - width, n, width);
        if (hit == 1) memcpy(h + 8, n, width);
        unsigned char *expected = first_match(h, length, n, width);
        uint64_t start = now();
        for (unsigned rep = 0; rep < 256; ++rep) {
          void *found = binary ? bytes(h, length, n, width) : (void *)strings((char *)h, (char *)n);
          if (found != expected) abort();
        }
        double ns = (now() - start) / 256.0;
        printf("%s,%s,%s,%td,%.2f\n", binary ? "memmem" : "strstr", names[kind],
               hit == 0 ? "miss" : hit == 1 ? "early" : "late", expected ? expected - h : -1, ns);
      }
    }
  }
}
