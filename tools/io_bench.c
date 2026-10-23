#define _GNU_SOURCE
#define _POSIX_C_SOURCE 200809L
#include <errno.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/uio.h>
#include <time.h>
#include <unistd.h>

#ifndef BENCH_COMPILER
#define BENCH_COMPILER "unspecified"
#endif
#ifndef BENCH_LIBC
#define BENCH_LIBC "unspecified"
#endif
#ifndef BENCH_IMPLEMENTATION
#define BENCH_IMPLEMENTATION "host-libc"
#endif

#define SAMPLES 7
#define ITERATIONS 200000u

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

int main(void) {
  int input = open("/dev/zero", O_RDONLY);
  int output = open("/dev/null", O_WRONLY);
  if (input < 0 || output < 0) return 1;
  char byte = 'x';
  ssize_t (*volatile read_call)(int, void *, size_t) = read;
  ssize_t (*volatile write_call)(int, const void *, size_t) = write;
  ssize_t (*volatile readv_call)(int, const struct iovec *, int) = readv;
  ssize_t (*volatile writev_call)(int, const struct iovec *, int) = writev;
  ssize_t (*volatile pread_call)(int, void *, size_t, off_t) = pread;
  ssize_t (*volatile pwrite_call)(int, const void *, size_t, off_t) = pwrite;
  ssize_t (*volatile preadv_call)(int, const struct iovec *, int, off_t) = preadv;
  ssize_t (*volatile pwritev_call)(int, const struct iovec *, int, off_t) = pwritev;
  ssize_t (*volatile preadv2_call)(int, const struct iovec *, int, off_t, int) = preadv2;
  ssize_t (*volatile pwritev2_call)(int, const struct iovec *, int, off_t, int) = pwritev2;
  double samples[SAMPLES];

  for (size_t sample = 0; sample < SAMPLES; ++sample) {
    double start = now_seconds();
    for (size_t iteration = 0; iteration < ITERATIONS; ++iteration)
      if (read_call(input, &byte, 1) != 1 || byte != 0) return 2;
    samples[sample] = (now_seconds() - start) * 1e9 / ITERATIONS;
  }
  sort(samples);
  printf("%s,%s,%s,read-1-byte-dev-zero,%u,%.2f\n", BENCH_COMPILER,
         BENCH_LIBC, BENCH_IMPLEMENTATION, ITERATIONS, samples[SAMPLES / 2]);

  for (size_t sample = 0; sample < SAMPLES; ++sample) {
    double start = now_seconds();
    for (size_t iteration = 0; iteration < ITERATIONS; ++iteration)
      if (write_call(output, &byte, 1) != 1) return 3;
    samples[sample] = (now_seconds() - start) * 1e9 / ITERATIONS;
  }
  sort(samples);
  printf("%s,%s,%s,write-1-byte-dev-null,%u,%.2f\n", BENCH_COMPILER,
         BENCH_LIBC, BENCH_IMPLEMENTATION, ITERATIONS, samples[SAMPLES / 2]);

  char vector_bytes[4] = {'a', 'b', 'c', 'd'};
  struct iovec vectors[4];
  for (size_t i = 0; i < 4; ++i) {
    vectors[i].iov_base = &vector_bytes[i];
    vectors[i].iov_len = 1;
  }
  for (int vector_count = 1; vector_count <= 4; vector_count *= 2) {
    for (size_t sample = 0; sample < SAMPLES; ++sample) {
      double start = now_seconds();
      for (size_t iteration = 0; iteration < ITERATIONS; ++iteration) {
        if (readv_call(input, vectors, vector_count) != vector_count) return 4;
        for (int i = 0; i < vector_count; ++i)
          if (vector_bytes[i] != 0) return 5;
      }
      samples[sample] = (now_seconds() - start) * 1e9 / ITERATIONS;
    }
    sort(samples);
    printf("%s,%s,%s,readv-%d-byte-dev-zero,%u,%.2f\n", BENCH_COMPILER,
           BENCH_LIBC, BENCH_IMPLEMENTATION, vector_count, ITERATIONS,
           samples[SAMPLES / 2]);

    for (size_t sample = 0; sample < SAMPLES; ++sample) {
      double start = now_seconds();
      for (size_t iteration = 0; iteration < ITERATIONS; ++iteration)
        if (writev_call(output, vectors, vector_count) != vector_count) return 6;
      samples[sample] = (now_seconds() - start) * 1e9 / ITERATIONS;
    }
    sort(samples);
    printf("%s,%s,%s,writev-%d-byte-dev-null,%u,%.2f\n", BENCH_COMPILER,
           BENCH_LIBC, BENCH_IMPLEMENTATION, vector_count, ITERATIONS,
           samples[SAMPLES / 2]);
  }

  char path[] = "/tmp/nelibc-preadv-XXXXXX";
  int positioned = mkstemp(path);
  if (positioned < 0 || unlink(path) != 0) return 7;
  char positioned_bytes[4] = {'e', 'f', 'g', 'h'};
  char received_bytes[4] = {0};
  struct iovec positioned_vectors[4];
  struct iovec received_vectors[4];
  for (size_t i = 0; i < 4; ++i) {
    positioned_vectors[i].iov_base = &positioned_bytes[i];
    positioned_vectors[i].iov_len = 1;
    received_vectors[i].iov_base = &received_bytes[i];
    received_vectors[i].iov_len = 1;
  }
  if (pwritev_call(positioned, positioned_vectors, 4, 0) != 4) return 8;
  char positioned_byte = 0;
  if (pread_call(positioned, &positioned_byte, 1, 0) != 1 ||
      positioned_byte != positioned_bytes[0])
    return 9;
  for (size_t sample = 0; sample < SAMPLES; ++sample) {
    double start = now_seconds();
    for (size_t iteration = 0; iteration < ITERATIONS; ++iteration)
      if (pread_call(positioned, &positioned_byte, 1, 0) != 1) return 10;
    samples[sample] = (now_seconds() - start) * 1e9 / ITERATIONS;
  }
  sort(samples);
  printf("%s,%s,%s,pread-1-byte-tempfile,%u,%.2f\n", BENCH_COMPILER,
         BENCH_LIBC, BENCH_IMPLEMENTATION, ITERATIONS, samples[SAMPLES / 2]);

  for (size_t sample = 0; sample < SAMPLES; ++sample) {
    double start = now_seconds();
    for (size_t iteration = 0; iteration < ITERATIONS; ++iteration)
      if (pwrite_call(positioned, &positioned_byte, 1, 0) != 1) return 11;
    samples[sample] = (now_seconds() - start) * 1e9 / ITERATIONS;
  }
  sort(samples);
  printf("%s,%s,%s,pwrite-1-byte-tempfile,%u,%.2f\n", BENCH_COMPILER,
         BENCH_LIBC, BENCH_IMPLEMENTATION, ITERATIONS, samples[SAMPLES / 2]);

  for (int vector_count = 1; vector_count <= 4; vector_count *= 2) {
    if (pwritev_call(positioned, positioned_vectors, vector_count, 0) != vector_count)
      return 12;
    if (preadv_call(positioned, received_vectors, vector_count, 0) != vector_count)
      return 13;
    for (int i = 0; i < vector_count; ++i)
      if (received_bytes[i] != positioned_bytes[i]) return 14;
    for (size_t sample = 0; sample < SAMPLES; ++sample) {
      double start = now_seconds();
      for (size_t iteration = 0; iteration < ITERATIONS; ++iteration)
        if (preadv_call(positioned, received_vectors, vector_count, 0) != vector_count)
          return 15;
      samples[sample] = (now_seconds() - start) * 1e9 / ITERATIONS;
    }
    sort(samples);
    printf("%s,%s,%s,preadv-%d-byte-tempfile,%u,%.2f\n", BENCH_COMPILER,
           BENCH_LIBC, BENCH_IMPLEMENTATION, vector_count, ITERATIONS,
           samples[SAMPLES / 2]);

    for (size_t sample = 0; sample < SAMPLES; ++sample) {
      double start = now_seconds();
      for (size_t iteration = 0; iteration < ITERATIONS; ++iteration)
        if (pwritev_call(positioned, positioned_vectors, vector_count, 0) != vector_count)
          return 16;
      samples[sample] = (now_seconds() - start) * 1e9 / ITERATIONS;
    }
    sort(samples);
    printf("%s,%s,%s,pwritev-%d-byte-tempfile,%u,%.2f\n", BENCH_COMPILER,
           BENCH_LIBC, BENCH_IMPLEMENTATION, vector_count, ITERATIONS,
           samples[SAMPLES / 2]);

    if (pwritev2_call(positioned, positioned_vectors, vector_count, 0, 0) != vector_count)
      return 17;
    if (preadv2_call(positioned, received_vectors, vector_count, 0, 0) != vector_count)
      return 18;
    for (int i = 0; i < vector_count; ++i)
      if (received_bytes[i] != positioned_bytes[i]) return 19;
    for (size_t sample = 0; sample < SAMPLES; ++sample) {
      double start = now_seconds();
      for (size_t iteration = 0; iteration < ITERATIONS; ++iteration)
        if (preadv2_call(positioned, received_vectors, vector_count, 0, 0) != vector_count)
          return 20;
      samples[sample] = (now_seconds() - start) * 1e9 / ITERATIONS;
    }
    sort(samples);
    printf("%s,%s,%s,preadv2-flags0-%d-byte-tempfile,%u,%.2f\n", BENCH_COMPILER,
           BENCH_LIBC, BENCH_IMPLEMENTATION, vector_count, ITERATIONS,
           samples[SAMPLES / 2]);

    for (size_t sample = 0; sample < SAMPLES; ++sample) {
      double start = now_seconds();
      for (size_t iteration = 0; iteration < ITERATIONS; ++iteration)
        if (pwritev2_call(positioned, positioned_vectors, vector_count, 0, 0) != vector_count)
          return 21;
      samples[sample] = (now_seconds() - start) * 1e9 / ITERATIONS;
    }
    sort(samples);
    printf("%s,%s,%s,pwritev2-flags0-%d-byte-tempfile,%u,%.2f\n", BENCH_COMPILER,
           BENCH_LIBC, BENCH_IMPLEMENTATION, vector_count, ITERATIONS,
           samples[SAMPLES / 2]);
  }
  close(positioned);
  close(input);
  close(output);
  return 0;
}
