#define _GNU_SOURCE
#include "string.h"
#include <stdint.h>
#include <stdio.h>
#include <sys/mman.h>
#include <time.h>
#include <unistd.h>

#ifndef BENCH_COMPILER
#define BENCH_COMPILER "unspecified"
#endif
#ifndef BENCH_LIBC
#define BENCH_LIBC "unspecified"
#endif
#ifndef BENCH_IMPLEMENTATION
#define BENCH_IMPLEMENTATION "libc"
#endif

#define MAX_HAYSTACK 65536
#define MAX_NEEDLE 256
#define SAMPLES 7
#define REPS 256
#define STRLEN_REPS 4096

typedef char *(*strstr_fn)(const char *, const char *);
typedef size_t (*strlen_fn)(const char *);
static char haystack[MAX_HAYSTACK + 16];
static char needle[MAX_NEEDLE + 1];
static volatile uintptr_t sink;
static uint32_t rng = 0x53545253u;

static uint32_t next_random(void) {
  uint32_t x = rng;
  x ^= x << 13;
  x ^= x >> 17;
  x ^= x << 5;
  return rng = x;
}

static char *reference_strstr(const char *h, const char *n) {
  if (!*n) return (char *)h;
  for (size_t i = 0; h[i]; ++i) {
    size_t j = 0;
    while (n[j] && h[i + j] == n[j]) ++j;
    if (!n[j]) return (char *)(h + i);
  }
  return NULL;
}

static int differential_checks(strstr_fn fn) {
  char h[4097];
  char n[MAX_NEEDLE + 1];
  for (size_t trial = 0; trial < 512; ++trial) {
    size_t hlen = 1024 + next_random() % 3073;
    size_t nlen = 1 + next_random() % MAX_NEEDLE;
    for (size_t i = 0; i < hlen; ++i) h[i] = (char)(1 + next_random() % 255);
    h[hlen] = 0;
    for (size_t i = 0; i < nlen; ++i) n[i] = (char)(1 + next_random() % 255);
    n[nlen] = 0;
    if ((trial & 1u) == 0) {
      size_t start = next_random() % (hlen - nlen + 1);
      for (size_t i = 0; i < nlen; ++i) h[start + i] = n[i];
    } else {
      for (size_t i = 0; i < hlen; ++i) h[i] = 'a';
      for (size_t i = 0; i < nlen; ++i) n[i] = 'a';
      n[nlen - 1] = 'b';
    }
    if (strlen(h) != (size_t)(strchr(h, 0) - h) || strlen(n) != (size_t)(strchr(n, 0) - n)) {
      fprintf(stderr, "strlen differential mismatch trial=%zu\n", trial);
      return 1;
    }
    if (fn(h, n) != reference_strstr(h, n)) {
      fprintf(stderr, "strstr differential mismatch trial=%zu needle=%zu\n", trial, nlen);
      return 1;
    }
  }
  return 0;
}

static int page_boundary_checks(strstr_fn fn) {
  long page_size_result = sysconf(_SC_PAGESIZE);
  if (page_size_result <= 0) return 1;
  size_t page_size = (size_t)page_size_result;
  char *mapping = mmap(NULL, page_size * 3, PROT_NONE,
                       MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
  if (mapping == MAP_FAILED) return 1;
  char *page = mapping + page_size;
  if (mprotect(page, page_size, PROT_READ | PROT_WRITE) != 0) return 1;

  for (size_t n = 0; n <= 512; ++n) {
    char *h = page + page_size - n - 1;
    for (size_t i = 0; i < n; ++i) h[i] = 'q';
    h[n] = 0;
    if (strlen(h) != n || fn(h, h) != h) {
      fprintf(stderr, "string page-boundary mismatch length=%zu\n", n);
      munmap(mapping, page_size * 3);
      return 1;
    }
  }
  munmap(mapping, page_size * 3);
  return 0;
}

static double now_seconds(void) {
  struct timespec ts;
  clock_gettime(CLOCK_MONOTONIC, &ts);
  return (double)ts.tv_sec + (double)ts.tv_nsec / 1e9;
}

static double median_strlen(strlen_fn fn, const char *s) {
  double samples[SAMPLES];
  for (size_t sample = 0; sample < SAMPLES; ++sample) {
    uintptr_t local = 0;
    double start = now_seconds();
    for (size_t i = 0; i < STRLEN_REPS; ++i) local += fn(s);
    samples[sample] = (now_seconds() - start) * 1e9 / STRLEN_REPS;
    sink ^= local;
  }
  for (size_t i = 1; i < SAMPLES; ++i) {
    double value = samples[i];
    size_t j = i;
    while (j && samples[j - 1] > value) {
      samples[j] = samples[j - 1];
      --j;
    }
    samples[j] = value;
  }
  return samples[SAMPLES / 2];
}

static double median(strstr_fn fn, size_t haystack_size, size_t needle_size) {
  double samples[SAMPLES];
  for (size_t sample = 0; sample < SAMPLES; ++sample) {
    uintptr_t local = 0;
    double start = now_seconds();
    for (size_t i = 0; i < REPS; ++i) local += (uintptr_t)fn(haystack, needle);
    samples[sample] = (now_seconds() - start) * 1e9 / REPS;
    sink ^= local;
  }
  for (size_t i = 1; i < SAMPLES; ++i) {
    double value = samples[i];
    size_t j = i;
    while (j && samples[j - 1] > value) {
      samples[j] = samples[j - 1];
      --j;
    }
    samples[j] = value;
  }
  (void)needle_size;
  return samples[SAMPLES / 2];
}

int main(void) {
  static const size_t haystack_sizes[] = {64, 4096, MAX_HAYSTACK};
  static const size_t needle_sizes[] = {4, 8, 16, 64, MAX_NEEDLE};
  if (differential_checks(strstr) != 0 || page_boundary_checks(strstr) != 0) return 1;
  puts("compiler,libc,implementation,case,haystack_bytes,needle_bytes,iterations,median_ns_per_call");
  static const size_t strlen_sizes[] = {0, 1, 4, 8, 15, 16, 31, 64, 4096, MAX_HAYSTACK};
  static const size_t strlen_offsets[] = {0, 1, 7, 15};
  for (size_t sidx = 0; sidx < sizeof strlen_sizes / sizeof strlen_sizes[0]; ++sidx) {
    size_t size = strlen_sizes[sidx];
    for (size_t aidx = 0; aidx < sizeof strlen_offsets / sizeof strlen_offsets[0]; ++aidx) {
      size_t offset = strlen_offsets[aidx];
      for (size_t i = 0; i < size; ++i) haystack[offset + i] = 's';
      haystack[offset + size] = 0;
      char name[32];
      snprintf(name, sizeof name, "strlen-align%zu", offset);
      printf("%s,%s,%s,%s,%zu,0,%u,%.2f\n", BENCH_COMPILER, BENCH_LIBC,
             BENCH_IMPLEMENTATION, name, size, STRLEN_REPS, median_strlen(strlen, haystack + offset));
    }
  }
  for (size_t hidx = 0; hidx < sizeof haystack_sizes / sizeof haystack_sizes[0]; ++hidx) {
    size_t hlen = haystack_sizes[hidx];
    for (size_t nidx = 0; nidx < sizeof needle_sizes / sizeof needle_sizes[0]; ++nidx) {
      size_t nlen = needle_sizes[nidx];
      if (nlen > hlen) continue;
      for (size_t i = 0; i < hlen; ++i) haystack[i] = 'x';
      for (size_t i = 0; i < nlen; ++i) needle[i] = (char)('A' + i % 23);
      haystack[hlen] = needle[nlen] = 0;
      for (size_t i = 0; i < nlen; ++i) haystack[hlen - nlen + i] = needle[i];
      printf("%s,%s,%s,tail-match,%zu,%zu,%u,%.2f\n", BENCH_COMPILER, BENCH_LIBC,
             BENCH_IMPLEMENTATION, hlen, nlen, REPS, median(strstr, hlen, nlen));

      for (size_t i = 0; i < hlen; ++i) haystack[i] = 'a';
      for (size_t i = 0; i < nlen; ++i) needle[i] = 'a';
      needle[nlen - 1] = 'b';
      haystack[hlen] = needle[nlen] = 0;
      printf("%s,%s,%s,periodic-absent,%zu,%zu,%u,%.2f\n", BENCH_COMPILER, BENCH_LIBC,
             BENCH_IMPLEMENTATION, hlen, nlen, REPS, median(strstr, hlen, nlen));
    }
  }
  return sink == UINTPTR_MAX;
}
