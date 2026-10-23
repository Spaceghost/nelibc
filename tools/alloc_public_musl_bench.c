#define _GNU_SOURCE
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#define SAMPLES 7
#define ITERATIONS 2000u

#ifdef BENCH_NELIBC_PUBLIC
#define IMPLEMENTATION "nelibc-public"
#else
#define IMPLEMENTATION "musl-host"
#endif

static volatile uintptr_t allocation_sink;

static double now_ns(void) {
  struct timespec value;
  if (clock_gettime(CLOCK_MONOTONIC, &value) != 0) return 0.0;
  return (double)value.tv_sec * 1e9 + (double)value.tv_nsec;
}

static int operation(const char *name, size_t size, double *elapsed) {
  size_t aligned_size = (size + 63u) & ~(size_t)63u;
  if (aligned_size == 0) aligned_size = 64;
  double start = now_ns();
  for (size_t i = 0; i < ITERATIONS; ++i) {
    void *pointer = NULL;
    if (name[0] == 'm') {
      pointer = malloc(size);
      if (pointer == NULL) return 1;
      allocation_sink ^= (uintptr_t)pointer;
      free(pointer);
    } else if (name[0] == 'c') {
      pointer = calloc(size, 1);
      if (pointer == NULL) return 1;
      allocation_sink ^= ((volatile unsigned char *)pointer)[0];
      free(pointer);
    } else if (name[0] == 'a') {
      pointer = aligned_alloc(64, aligned_size);
      if (pointer == NULL) return 1;
      allocation_sink ^= (uintptr_t)pointer;
      free(pointer);
    } else {
      pointer = malloc(size);
      if (pointer == NULL) return 1;
      ((unsigned char *)pointer)[0] = 0x5a;
      size_t new_size = name[8] == 's' ? size - 1 : size * 2 + 1;
      void *resized = realloc(pointer, new_size);
      if (resized == NULL || ((unsigned char *)resized)[0] != 0x5a) {
        free(resized != NULL ? resized : pointer);
        return 1;
      }
      allocation_sink ^= (uintptr_t)resized;
      free(resized);
    }
  }
  *elapsed = (now_ns() - start) / ITERATIONS;
  return 0;
}

int main(void) {
  static const char *operations[] = {
    "malloc-free", "calloc-free", "realloc-same-free",
    "realloc-grow-free", "aligned-free"
  };
  static const size_t sizes[] = {64, 128, 4096, 32768};
  puts("implementation,function,size_bytes,iterations,median_ns_per_call");
  for (size_t s = 0; s < sizeof sizes / sizeof sizes[0]; ++s) {
    for (size_t op = 0; op < sizeof operations / sizeof operations[0]; ++op) {
      double samples[SAMPLES];
      for (size_t n = 0; n < SAMPLES; ++n) {
        if (operation(operations[op], sizes[s], &samples[n]) != 0) return 1;
      }
      for (size_t i = 1; i < SAMPLES; ++i) {
        double value = samples[i];
        size_t j = i;
        while (j > 0 && samples[j - 1] > value) {
          samples[j] = samples[j - 1];
          --j;
        }
        samples[j] = value;
      }
      printf("%s,%s,%zu,%u,%.2f\n", IMPLEMENTATION, operations[op],
             sizes[s], ITERATIONS, samples[SAMPLES / 2]);
    }
  }
  return allocation_sink == UINTPTR_MAX ? 2 : 0;
}
