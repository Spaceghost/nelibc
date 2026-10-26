#define _POSIX_C_SOURCE 200809L
#include "string.h"
#include <stdint.h>
#include <stdio.h>
#include <time.h>

#ifndef BENCH_COMPILER
#define BENCH_COMPILER "unspecified"
#endif
#ifndef BENCH_LIBC
#define BENCH_LIBC "unspecified"
#endif
#ifndef BENCH_IMPLEMENTATION
#define BENCH_IMPLEMENTATION "libc"
#endif
#ifndef BENCH_MEMMOVE_FUNCTION
#define BENCH_MEMMOVE_FUNCTION memmove
#else
extern void *BENCH_MEMMOVE_FUNCTION(void *, const void *, size_t);
#endif

#define MAX_SIZE 65536
#define SAMPLES 7
#define WORK_BYTES (64u * 1024u * 1024u)

typedef void *(*copy_fn)(void *, const void *, size_t);
typedef void *(*stop_copy_fn)(void *, const void *, int, size_t);
typedef void *(*set_fn)(void *, int, size_t);
typedef int (*compare_fn)(const void *, const void *, size_t);
typedef void *(*find_fn)(const void *, int, size_t);
typedef void *(*search_fn)(const void *, size_t, const void *, size_t);

static unsigned char source[MAX_SIZE];
static unsigned char destination[MAX_SIZE];
static unsigned char move_area[2 * MAX_SIZE + 1];
static unsigned char haystack[MAX_SIZE];
static unsigned char needle[] = {0xF1, 0xF2, 0xF3, 0xF4};
static volatile uintptr_t sink;
static void sort(double values[SAMPLES]);

static double now_seconds(void) {
  struct timespec ts;
  if (clock_gettime(CLOCK_MONOTONIC, &ts) != 0) return 0.0;
  return (double)ts.tv_sec + (double)ts.tv_nsec / 1000000000.0;
}

static double median_copy(copy_fn fn, size_t size, size_t reps) {
  double samples[SAMPLES];
  for (size_t s = 0; s < SAMPLES; ++s) {
    uintptr_t local = 0;
    double start = now_seconds();
    for (size_t i = 0; i < reps; ++i) local += (uintptr_t)fn(destination, source, size);
    samples[s] = (now_seconds() - start) * 1e9 / (double)reps;
    sink ^= local;
  }
  sort(samples);
  return samples[SAMPLES / 2];
}

static double median_memccpy(stop_copy_fn fn, size_t size, size_t reps) {
  double samples[SAMPLES];
  for (size_t s = 0; s < SAMPLES; ++s) {
    uintptr_t local = 0;
    double start = now_seconds();
    for (size_t i = 0; i < reps; ++i) local += (uintptr_t)fn(destination, haystack, 0xEE, size);
    samples[s] = (now_seconds() - start) * 1e9 / (double)reps;
    sink ^= local;
  }
  sort(samples);
  return samples[SAMPLES / 2];
}

static double median_move(copy_fn fn, size_t size, size_t reps,
                          size_t source_offset, size_t destination_offset) {
  double samples[SAMPLES];
  for (size_t s = 0; s < SAMPLES; ++s) {
    for (size_t i = 0; i < size; ++i) move_area[source_offset + i] = source[i];
    uintptr_t local = 0;
    double start = now_seconds();
    for (size_t i = 0; i < reps; ++i) {
      local += (uintptr_t)fn(move_area + destination_offset,
                             move_area + source_offset, size);
    }
    samples[s] = (now_seconds() - start) * 1e9 / (double)reps;
    sink ^= local;
  }
  sort(samples);
  return samples[SAMPLES / 2];
}

static double median_set(set_fn fn, size_t size, size_t reps) {
  double samples[SAMPLES];
  for (size_t s = 0; s < SAMPLES; ++s) {
    uintptr_t local = 0;
    double start = now_seconds();
    for (size_t i = 0; i < reps; ++i) local += (uintptr_t)fn(destination, (int)i, size);
    samples[s] = (now_seconds() - start) * 1e9 / (double)reps;
    sink ^= local;
  }
  sort(samples);
  return samples[SAMPLES / 2];
}

static double median_compare(compare_fn fn, size_t size, size_t reps) {
  double samples[SAMPLES];
  for (size_t s = 0; s < SAMPLES; ++s) {
    uintptr_t local = 0;
    double start = now_seconds();
    for (size_t i = 0; i < reps; ++i) local += (uintptr_t)fn(source, destination, size);
    samples[s] = (now_seconds() - start) * 1e9 / (double)reps;
    sink ^= local;
  }
  sort(samples);
  return samples[SAMPLES / 2];
}

static double median_find(find_fn fn, size_t size, size_t reps, int target) {
  double samples[SAMPLES];
  for (size_t s = 0; s < SAMPLES; ++s) {
    uintptr_t local = 0;
    double start = now_seconds();
    for (size_t i = 0; i < reps; ++i) local += (uintptr_t)fn(haystack, target, size);
    samples[s] = (now_seconds() - start) * 1e9 / (double)reps;
    sink ^= local;
  }
  sort(samples);
  return samples[SAMPLES / 2];
}

static double median_search(search_fn fn, size_t size, const unsigned char *search_needle,
                            size_t needle_size, size_t reps) {
  double samples[SAMPLES];
  for (size_t s = 0; s < SAMPLES; ++s) {
    uintptr_t local = 0;
    double start = now_seconds();
    for (size_t i = 0; i < reps; ++i) local += (uintptr_t)fn(haystack, size, search_needle, needle_size);
    samples[s] = (now_seconds() - start) * 1e9 / (double)reps;
    sink ^= local;
  }
  sort(samples);
  return samples[SAMPLES / 2];
}

static void sort(double values[SAMPLES]) {
  for (size_t i = 1; i < SAMPLES; ++i) {
    double value = values[i];
    size_t j = i;
    while (j > 0 && values[j - 1] > value) {
      values[j] = values[j - 1];
      --j;
    }
    values[j] = value;
  }
}

static void report(const char *name, double ns_per_call, size_t size, size_t reps) {
  printf("%s,%s,%s,%s,%zu,%zu,%.2f\n", BENCH_COMPILER, BENCH_LIBC,
         BENCH_IMPLEMENTATION, name, size, reps, ns_per_call);
}

int main(void) {
  /* Include Alpaquita's short/medium/long string lengths (36, 132, 4100)
     alongside cache-line, page, and large-buffer cases. */
  static const size_t sizes[] = {36, 64, 132, 4096, 4100, MAX_SIZE};
  for (size_t i = 0; i < MAX_SIZE; ++i) {
    source[i] = (unsigned char)(i * 37u + 11u);
    haystack[i] = 0x11;
  }
  memcpy(haystack + MAX_SIZE / 2, needle, sizeof needle);
  haystack[0] = 0xEF;
  haystack[MAX_SIZE - 1] = 0xEE;
  puts("compiler,libc,implementation,function,size_bytes,iterations,median_ns_per_call");
#ifdef BENCH_MEMMOVE_ONLY
  static const size_t move_sizes[] = {2047, 2048, 2049, 2559, 2560, 3072,
                                      3584, 4095, 4096, 4097, MAX_SIZE};
  for (size_t s = 0; s < sizeof move_sizes / sizeof move_sizes[0]; ++s) {
    size_t size = move_sizes[s];
    size_t reps = WORK_BYTES / size;
    if (reps < 64) reps = 64;
    report("memmove-overlap-backward",
           median_move(BENCH_MEMMOVE_FUNCTION, size, reps, 0, 1), size, reps);
    report("memmove-overlap-forward",
           median_move(BENCH_MEMMOVE_FUNCTION, size, reps, 1, 0), size, reps);
    report("memmove-disjoint-dst-after",
           median_move(BENCH_MEMMOVE_FUNCTION, size, reps, 0, size + 1), size, reps);
    report("memmove-disjoint-dst-before",
           median_move(BENCH_MEMMOVE_FUNCTION, size, reps, size + 1, 0), size, reps);
  }
  return sink == UINTPTR_MAX;
#endif
  for (size_t s = 0; s < sizeof sizes / sizeof sizes[0]; ++s) {
    size_t size = sizes[s];
    size_t reps = WORK_BYTES / size;
    if (reps < 64) reps = 64;
    for (size_t i = 0; i < size; ++i) haystack[i] = 0x11;
    memcpy(haystack + size / 2, needle, sizeof needle);
    haystack[0] = 0xEF;
    haystack[size - 1] = 0xEE;
    memset(destination, 0, size);
    report("memcpy", median_copy(memcpy, size, reps), size, reps);
    report("mempcpy", median_copy(mempcpy, size, reps), size, reps);
    report("memmove-overlap-backward",
           median_move(memmove, size, reps, 0, 1), size, reps);
    report("memmove-overlap-forward",
           median_move(memmove, size, reps, 1, 0), size, reps);
    report("memmove-disjoint-dst-after",
           median_move(memmove, size, reps, 0, size + 1), size, reps);
    report("memmove-disjoint-dst-before",
           median_move(memmove, size, reps, size + 1, 0), size, reps);
    report("memccpy", median_memccpy(memccpy, size, reps), size, reps);
    report("memset", median_set(memset, size, reps), size, reps);
    memcpy(destination, source, size);
    report("memcmp", median_compare(memcmp, size, reps), size, reps);
    const size_t mismatch_positions[] = {0, size / 2, size - 1};
    const char *mismatch_names[] = {
        "memcmp-first-diff", "memcmp-middle-diff", "memcmp-last-diff"};
    for (size_t position = 0; position < 3; ++position) {
      destination[mismatch_positions[position]] ^= 1;
      report(mismatch_names[position], median_compare(memcmp, size, reps), size, reps);
      destination[mismatch_positions[position]] ^= 1;
    }
    report("memchr", median_find(memchr, size, reps, 0xEE), size, reps);
    report("memrchr", median_find(memrchr, size, reps, 0xEF), size, reps);
    report("memmem-rare4", median_search(memmem, size, needle, sizeof needle, reps), size, reps);
  }

  /* Long needles and a periodic near-match expose substring-search worst cases. */
  static unsigned char long_needle[256];
  const size_t long_sizes[] = {4, 8, 12, 16, 24, 32, 64, 128, 256};
  const size_t size = MAX_SIZE;
  for (size_t k = 0; k < sizeof long_sizes / sizeof long_sizes[0]; ++k) {
    size_t needle_size = long_sizes[k];
    size_t reps = 256;
    char needle_tail_name[40];
    char periodic_absent_name[40];
    snprintf(needle_tail_name, sizeof needle_tail_name,
             "memmem-needle-tail-%u", (unsigned)needle_size);
    snprintf(periodic_absent_name, sizeof periodic_absent_name,
             "memmem-periodic-absent-%u", (unsigned)needle_size);
    for (size_t i = 0; i < needle_size; ++i) long_needle[i] = (unsigned char)(0x80u + (i * 29u) % 0x7Fu);
    memset(haystack, 0x11, size);
    memcpy(haystack + size - needle_size, long_needle, needle_size);
    report(needle_tail_name, median_search(memmem, size, long_needle, needle_size, reps), size, reps);

    memset(haystack, 'a', size);
    memset(long_needle, 'a', needle_size);
    long_needle[needle_size - 1] = 'b';
    report(periodic_absent_name, median_search(memmem, size, long_needle, needle_size, reps), size, reps);
  }
  return sink == UINTPTR_MAX;
}
