#define _POSIX_C_SOURCE 200809L
#include "nelibc_alloc.h"
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#ifdef NELIBC_BENCH_GLIBC
/* Stable glibc entry points let the benchmark reach the host allocator even
   when the executable exports Nelua's public malloc/free/realloc symbols. */
extern void *__libc_malloc(size_t size);
extern void *__libc_calloc(size_t count, size_t size);
extern void *__libc_realloc(void *pointer, size_t size);
extern void __libc_free(void *pointer);
extern void *__libc_memalign(size_t alignment, size_t size);
#endif

#ifndef BENCH_COMPILER
#define BENCH_COMPILER "unspecified"
#endif
#ifndef BENCH_LIBC
#define BENCH_LIBC "unspecified"
#endif

#define SAMPLES 7
#define ITERATIONS 2000u

typedef void *(*allocation_fn)(size_t);
typedef void *(*reallocation_fn)(void *, size_t);
typedef void (*deallocation_fn)(void *);
typedef void *(*zero_allocation_fn)(size_t, size_t);
typedef void *(*aligned_allocation_fn)(size_t, size_t);

static volatile uintptr_t sink;

static double now_seconds(void) {
  struct timespec ts;
  clock_gettime(CLOCK_MONOTONIC, &ts);
  return (double)ts.tv_sec + (double)ts.tv_nsec / 1000000000.0;
}

static void sort(double values[SAMPLES]) {
  for (size_t i = 1; i < SAMPLES; ++i) {
    double value = values[i];
    size_t j = i;
    while (j != 0 && values[j - 1] > value) {
      values[j] = values[j - 1];
      --j;
    }
    values[j] = value;
  }
}

static double bench_allocate(allocation_fn allocate, deallocation_fn deallocate,
                             size_t size) {
  double samples[SAMPLES];
  for (size_t sample = 0; sample < SAMPLES; ++sample) {
    uintptr_t local = 0;
    double start = now_seconds();
    for (size_t i = 0; i < ITERATIONS; ++i) {
      unsigned char *ptr = allocate(size);
      if (ptr == NULL) return -1.0;
      ptr[0] = (unsigned char)i;
      local += ptr[0];
      deallocate(ptr);
    }
    samples[sample] = (now_seconds() - start) * 1e9 / ITERATIONS;
    sink ^= local;
  }
  sort(samples);
  return samples[SAMPLES / 2];
}

static double bench_calloc(zero_allocation_fn allocate,
                           deallocation_fn deallocate, size_t size) {
  double samples[SAMPLES];
  for (size_t sample = 0; sample < SAMPLES; ++sample) {
    uintptr_t local = 0;
    double start = now_seconds();
    for (size_t i = 0; i < ITERATIONS; ++i) {
      unsigned char *ptr = allocate(1, size);
      if (ptr == NULL) return -1.0;
      local += ptr[size - 1];
      deallocate(ptr);
    }
    samples[sample] = (now_seconds() - start) * 1e9 / ITERATIONS;
    sink ^= local;
  }
  sort(samples);
  return samples[SAMPLES / 2];
}

static double bench_reallocate(allocation_fn allocate,
                               reallocation_fn reallocate,
                               deallocation_fn deallocate, size_t size,
                               int double_size) {
  double samples[SAMPLES];
  for (size_t sample = 0; sample < SAMPLES; ++sample) {
    uintptr_t local = 0;
    double start = now_seconds();
    for (size_t i = 0; i < ITERATIONS; ++i) {
      unsigned char *ptr = allocate(size);
      if (ptr == NULL) return -1.0;
      ptr[0] = (unsigned char)i;
      ptr = reallocate(ptr, double_size ? size * 2 : size + 16);
      if (ptr == NULL) return -1.0;
      local += ptr[0];
      deallocate(ptr);
    }
    samples[sample] = (now_seconds() - start) * 1e9 / ITERATIONS;
    sink ^= local;
  }
  sort(samples);
  return samples[SAMPLES / 2];
}

static double bench_aligned(aligned_allocation_fn allocate,
                            deallocation_fn deallocate, size_t size) {
  double samples[SAMPLES];
  for (size_t sample = 0; sample < SAMPLES; ++sample) {
    uintptr_t local = 0;
    double start = now_seconds();
    for (size_t i = 0; i < ITERATIONS; ++i) {
      unsigned char *ptr = allocate(64, size);
      if (ptr == NULL || (uintptr_t)ptr % 64 != 0) return -1.0;
      ptr[0] = (unsigned char)i;
      local += ptr[0];
      deallocate(ptr);
    }
    samples[sample] = (now_seconds() - start) * 1e9 / ITERATIONS;
    sink ^= local;
  }
  sort(samples);
  return samples[SAMPLES / 2];
}

static double bench_ownership(void *pointer, int expected) {
  double samples[SAMPLES];
  for (size_t sample = 0; sample < SAMPLES; ++sample) {
    uintptr_t local = 0;
    double start = now_seconds();
    for (size_t i = 0; i < ITERATIONS; ++i) {
      int owned = nelibc_allocator_owns(pointer);
      if (owned != expected) return -1.0;
      local += (uintptr_t)owned;
    }
    samples[sample] = (now_seconds() - start) * 1e9 / ITERATIONS;
    sink ^= local;
  }
  sort(samples);
  return samples[SAMPLES / 2];
}

static double bench_hybrid_free(allocation_fn allocate, size_t size) {
  double samples[SAMPLES];
  for (size_t sample = 0; sample < SAMPLES; ++sample) {
    uintptr_t local = 0;
    double start = now_seconds();
    for (size_t i = 0; i < ITERATIONS; ++i) {
      unsigned char *ptr = allocate(size);
      if (ptr == NULL) return -1.0;
      ptr[0] = (unsigned char)i;
      local += ptr[0];
      nelibc_hybrid_free(ptr);
    }
    samples[sample] = (now_seconds() - start) * 1e9 / ITERATIONS;
    sink ^= local;
  }
  sort(samples);
  return samples[SAMPLES / 2];
}

static double bench_hybrid_realloc(allocation_fn allocate, size_t size) {
  double samples[SAMPLES];
  for (size_t sample = 0; sample < SAMPLES; ++sample) {
    uintptr_t local = 0;
    double start = now_seconds();
    for (size_t i = 0; i < ITERATIONS; ++i) {
      unsigned char *ptr = allocate(size);
      if (ptr == NULL) return -1.0;
      ptr[0] = (unsigned char)i;
      ptr = nelibc_hybrid_realloc(ptr, size + 16);
      if (ptr == NULL) return -1.0;
      local += ptr[0];
      nelibc_hybrid_free(ptr);
    }
    samples[sample] = (now_seconds() - start) * 1e9 / ITERATIONS;
    sink ^= local;
  }
  sort(samples);
  return samples[SAMPLES / 2];
}

static int report(const char *implementation, const char *function,
                  size_t size, double median) {
  if (median < 0.0) return 1;
  printf("%s,%s,%s,%s,%zu,%u,%.2f\n", BENCH_COMPILER, BENCH_LIBC,
         implementation, function, size, ITERATIONS, median);
  return 0;
}

static int run(const char *implementation, allocation_fn allocate,
               zero_allocation_fn zero_allocate,
               reallocation_fn reallocate,
               aligned_allocation_fn aligned_allocate,
               deallocation_fn deallocate) {
  static const size_t sizes[] = {64, 128, 4096};
  for (size_t i = 0; i < sizeof sizes / sizeof sizes[0]; ++i) {
    size_t size = sizes[i];
    if (report(implementation, "malloc-free", size,
               bench_allocate(allocate, deallocate, size)) ||
        report(implementation, "calloc-free", size,
               bench_calloc(zero_allocate, deallocate, size)) ||
        report(implementation, "realloc-double-free", size,
               bench_reallocate(allocate, reallocate, deallocate, size, 1)) ||
        report(implementation, "realloc-small-grow-free", size,
               bench_reallocate(allocate, reallocate, deallocate, size, 0)) ||
        report(implementation, "aligned_alloc-free", size,
               bench_aligned(aligned_allocate, deallocate, size))) {
      return 1;
    }
  }
  return 0;
}

static int run_hybrid_bridge(const char *implementation,
                             allocation_fn allocate) {
  static const size_t sizes[] = {64, 4096, 32768};
  for (size_t i = 0; i < sizeof sizes / sizeof sizes[0]; ++i) {
    size_t size = sizes[i];
    if (report(implementation, "hybrid-free", size,
               bench_hybrid_free(allocate, size)) ||
        report(implementation, "hybrid-realloc-free", size,
               bench_hybrid_realloc(allocate, size))) {
      return 1;
    }
  }
  return 0;
}

int main(void) {
  puts("compiler,libc,implementation,function,size_bytes,iterations,median_ns_per_call");
#ifdef NELIBC_BENCH_GLIBC
  allocation_fn host_allocate = __libc_malloc;
  zero_allocation_fn host_zero_allocate = __libc_calloc;
  reallocation_fn host_reallocate = __libc_realloc;
  aligned_allocation_fn host_aligned_allocate = __libc_memalign;
  deallocation_fn host_deallocate = __libc_free;
#else
  allocation_fn host_allocate = malloc;
  zero_allocation_fn host_zero_allocate = calloc;
  reallocation_fn host_reallocate = realloc;
  aligned_allocation_fn host_aligned_allocate = aligned_alloc;
  deallocation_fn host_deallocate = free;
#endif
  if (run("host-libc", host_allocate, host_zero_allocate, host_reallocate,
          host_aligned_allocate, host_deallocate) != 0)
    return 1;
  if (run("nelibc", nelibc_malloc, nelibc_calloc, nelibc_realloc,
          nelibc_aligned_alloc, nelibc_free) != 0)
    return 2;
#ifdef NELIBC_BENCH_GLIBC
  if (run("hybrid-public", malloc, calloc, realloc, aligned_alloc, free) != 0)
    return 5;
#endif
  void *host_pointer = host_allocate(64);
  void *nelibc_pointer = nelibc_malloc(64);
  void *host_direct_pointer = host_allocate(32768);
  void *nelibc_direct_pointer = nelibc_malloc(32768);
  if (host_pointer == NULL || nelibc_pointer == NULL ||
      host_direct_pointer == NULL || nelibc_direct_pointer == NULL ||
      report("host-pointer", "ownership-check", 64,
             bench_ownership(host_pointer, 0)) ||
      report("nelibc-pointer", "ownership-check", 64,
             bench_ownership(nelibc_pointer, 1)) ||
      report("host-direct-pointer", "ownership-check", 32768,
             bench_ownership(host_direct_pointer, 0)) ||
      report("nelibc-direct-pointer", "ownership-check", 32768,
             bench_ownership(nelibc_direct_pointer, 1)))
    return 3;
  host_deallocate(host_pointer);
  nelibc_free(nelibc_pointer);
  host_deallocate(host_direct_pointer);
  nelibc_free(nelibc_direct_pointer);
#ifdef NELIBC_BENCH_GLIBC
  if (run_hybrid_bridge("hybrid-host-pointer", host_allocate) != 0 ||
#else
  if (run_hybrid_bridge("hybrid-host-pointer", host_allocate) != 0 ||
#endif
      run_hybrid_bridge("hybrid-nelibc-pointer", nelibc_malloc) != 0)
    return 4;
  return sink == SIZE_MAX;
}
