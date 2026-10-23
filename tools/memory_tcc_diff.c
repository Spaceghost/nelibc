#define _GNU_SOURCE
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>

void *nelibc_tcc_sse_memchr(const void *, int, size_t);
void *nelibc_tcc_sse_memrchr(const void *, int, size_t);
int nelibc_tcc_sse_memcmp(const void *, const void *, size_t);
void *nelibc_tcc_sse_memmove(void *, const void *, size_t);
void *nelibc_tcc_sse_memccpy(void *, const void *, int, size_t);

static unsigned char source[640];
static unsigned char other[640];
static unsigned char actual[704];
static unsigned char expected[704];

static int signum(int value) {
  return (value > 0) - (value < 0);
}

static int fail(const char *name, size_t offset, size_t size) {
  fprintf(stderr, "%s mismatch at offset %zu, size %zu\n", name, offset, size);
  return 1;
}

int main(void) {
  for (size_t i = 0; i < sizeof source; ++i) {
    source[i] = (unsigned char)(i * 73u + i / 3u + 19u);
    other[i] = source[i];
  }

  for (size_t offset = 0; offset < 16; ++offset) {
    for (size_t size = 0; size <= 512; ++size) {
      const unsigned char *src = source + offset;
      const unsigned char *cmp = other + offset;
      if (signum(nelibc_tcc_sse_memcmp(src, cmp, size)) !=
          signum(memcmp(src, cmp, size))) {
        return fail("memcmp equal", offset, size);
      }
      for (size_t position = 0; position < size; ++position) {
        other[offset + position] ^= 0x81;
        if (signum(nelibc_tcc_sse_memcmp(src, cmp, size)) !=
            signum(memcmp(src, cmp, size))) {
          return fail("memcmp mismatch", offset, size);
        }
        other[offset + position] ^= 0x81;
      }
      for (int target = 0; target < 256; ++target) {
        if (nelibc_tcc_sse_memchr(src, target, size) !=
            memchr(src, target, size)) {
          return fail("memchr", offset, size);
        }
        if (nelibc_tcc_sse_memrchr(src, target, size) !=
            memrchr(src, target, size)) {
          return fail("memrchr", offset, size);
        }
      }

      for (size_t direction = 0; direction < 2; ++direction) {
        memcpy(actual, source, sizeof source);
        memcpy(expected, source, sizeof source);
        size_t dst_offset = direction == 0 ? offset + 2 : offset;
        size_t src_offset = direction == 0 ? offset : offset + 2;
        if (nelibc_tcc_sse_memmove(actual + dst_offset, actual + src_offset, size) !=
            actual + dst_offset) {
          return fail("memmove return", offset, size);
        }
        memmove(expected + dst_offset, expected + src_offset, size);
        if (memcmp(actual, expected, sizeof source) != 0) {
          return fail("memmove bytes", offset, size);
        }
      }

      for (int target = 0; target < 256; ++target) {
        memset(actual, 0xA5, sizeof actual);
        memset(expected, 0xA5, sizeof expected);
        void *got = nelibc_tcc_sse_memccpy(actual, src, target, size);
        void *want = memccpy(expected, src, target, size);
        if ((got == NULL) != (want == NULL) ||
            (got != NULL && (size_t)((unsigned char *)got - actual) !=
                                (size_t)((unsigned char *)want - expected)) ||
            memcmp(actual, expected, sizeof actual) != 0) {
          return fail("memccpy", offset, size);
        }
      }
    }
  }

  puts("TCC SSE2 memory helpers match host libc (offsets 0-15, sizes 0-512)");
  return 0;
}
