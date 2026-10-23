#define _POSIX_C_SOURCE 200809L
#include "nelibc_alloc.h"
#include <pthread.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#ifndef BENCH_COMPILER
#define BENCH_COMPILER "unspecified"
#endif
#ifndef BENCH_LIBC
#define BENCH_LIBC "unspecified"
#endif

#define SAMPLES 7
#define ITERATIONS 30000u
#define MAX_THREADS 8

typedef void *(*allocation_fn)(size_t);
typedef void (*deallocation_fn)(void *);

typedef struct {
  pthread_mutex_t mutex;
  pthread_cond_t condition;
  size_t ready;
  size_t done;
  int go;
  size_t size;
  allocation_fn allocate;
  deallocation_fn deallocate;
} ThreadGate;

typedef struct {
  ThreadGate *gate;
  uintptr_t digest;
  int failed;
} ThreadProbe;

static volatile uintptr_t sink;

static double now_seconds(void) {
  struct timespec ts;
  if (clock_gettime(CLOCK_MONOTONIC, &ts) != 0) return -1.0;
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

static void *allocation_worker(void *argument) {
  ThreadProbe *probe = argument;
  ThreadGate *gate = probe->gate;
  pthread_mutex_lock(&gate->mutex);
  ++gate->ready;
  pthread_cond_broadcast(&gate->condition);
  while (!gate->go)
    pthread_cond_wait(&gate->condition, &gate->mutex);
  pthread_mutex_unlock(&gate->mutex);

  uintptr_t digest = 0;
  for (size_t iteration = 0; iteration < ITERATIONS; ++iteration) {
    unsigned char *memory = gate->allocate(gate->size);
    if (memory == NULL) {
      probe->failed = 1;
      break;
    }
    unsigned char value = (unsigned char)(iteration + (uintptr_t)probe);
    memory[0] = value;
    if (gate->size > 1) memory[gate->size - 1] = (unsigned char)~value;
    digest += memory[0];
    if (gate->size > 1) digest += memory[gate->size - 1];
    gate->deallocate(memory);
  }
  probe->digest = digest;

  pthread_mutex_lock(&gate->mutex);
  ++gate->done;
  pthread_cond_broadcast(&gate->condition);
  pthread_mutex_unlock(&gate->mutex);
  return NULL;
}

static double threaded_sample(allocation_fn allocate, deallocation_fn deallocate,
                              size_t size, size_t thread_count) {
  ThreadGate gate;
  ThreadProbe probes[MAX_THREADS] = {{0}};
  pthread_t threads[MAX_THREADS];
  if (pthread_mutex_init(&gate.mutex, NULL) != 0) return -1.0;
  if (pthread_cond_init(&gate.condition, NULL) != 0) {
    pthread_mutex_destroy(&gate.mutex);
    return -1.0;
  }
  gate.ready = 0;
  gate.done = 0;
  gate.go = 0;
  gate.size = size;
  gate.allocate = allocate;
  gate.deallocate = deallocate;

  size_t created = 0;
  for (; created < thread_count; ++created) {
    probes[created].gate = &gate;
    if (pthread_create(&threads[created], NULL, allocation_worker,
                       &probes[created]) != 0)
      break;
  }
  if (created != thread_count) {
    pthread_mutex_lock(&gate.mutex);
    gate.go = 1;
    pthread_cond_broadcast(&gate.condition);
    pthread_mutex_unlock(&gate.mutex);
    for (size_t i = 0; i < created; ++i) pthread_join(threads[i], NULL);
    pthread_cond_destroy(&gate.condition);
    pthread_mutex_destroy(&gate.mutex);
    return -1.0;
  }

  pthread_mutex_lock(&gate.mutex);
  while (gate.ready != thread_count)
    pthread_cond_wait(&gate.condition, &gate.mutex);
  double start = now_seconds();
  gate.go = 1;
  pthread_cond_broadcast(&gate.condition);
  while (gate.done != thread_count)
    pthread_cond_wait(&gate.condition, &gate.mutex);
  double finish = now_seconds();
  pthread_mutex_unlock(&gate.mutex);

  int failed = start < 0.0 || finish < start;
  for (size_t i = 0; i < thread_count; ++i) {
    if (pthread_join(threads[i], NULL) != 0 || probes[i].failed)
      failed = 1;
    sink ^= probes[i].digest;
  }
  pthread_cond_destroy(&gate.condition);
  pthread_mutex_destroy(&gate.mutex);
  if (failed) return -1.0;
  return (finish - start) * 1e9 /
      ((double)thread_count * (double)ITERATIONS);
}

static double threaded_median(allocation_fn allocate, deallocation_fn deallocate,
                              size_t size, size_t thread_count) {
  double samples[SAMPLES];
  for (size_t i = 0; i < SAMPLES; ++i) {
    samples[i] = threaded_sample(allocate, deallocate, size, thread_count);
    if (samples[i] < 0.0) return -1.0;
  }
  sort(samples);
  return samples[SAMPLES / 2];
}

static int run(const char *implementation, allocation_fn allocate,
               deallocation_fn deallocate) {
  static const size_t sizes[] = {64, 128, 4096};
  static const size_t thread_counts[] = {1, 2, 4, 8};
  for (size_t i = 0; i < sizeof sizes / sizeof sizes[0]; ++i) {
    for (size_t j = 0; j < sizeof thread_counts / sizeof thread_counts[0]; ++j) {
      size_t count = thread_counts[j];
      double median = threaded_median(allocate, deallocate, sizes[i], count);
      if (median < 0.0) return 1;
      printf("%s,%s,%s,malloc-free,%zu,%zu,%u,%.2f\n", BENCH_COMPILER,
          BENCH_LIBC, implementation, sizes[i], count, ITERATIONS, median);
    }
  }
  return 0;
}

int main(void) {
  puts("compiler,libc,implementation,function,size_bytes,threads,iterations_per_thread,median_ns_per_pair");
  if (run("host-libc", malloc, free) != 0) return 1;
  if (run("nelibc", nelibc_malloc, nelibc_free) != 0) return 2;
  return sink == SIZE_MAX;
}
