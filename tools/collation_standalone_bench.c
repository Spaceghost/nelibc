#define _GNU_SOURCE 1
#include <locale.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <time.h>

#ifndef BENCH_COMPILER
#define BENCH_COMPILER "standalone-unknown"
#endif

/* BEGIN GENERATED GERMAN COMBINING CLASS BENCHMARK */
typedef struct { unsigned int combining_class; const char *utf8; } combining_class_mark;
static const combining_class_mark combining_class_marks[] = {
  {1, "\xcc\xb4"},
  {6, "\xf0\x96\xbf\xb0"},
  {7, "\xe0\xa4\xbc"},
  {8, "\xe3\x82\x99"},
  {9, "\xe0\xa5\x8d"},
  {10, "\xd6\xb0"},
  {11, "\xd6\xb1"},
  {12, "\xd6\xb2"},
  {13, "\xd6\xb3"},
  {14, "\xd6\xb4"},
  {15, "\xd6\xb5"},
  {16, "\xd6\xb6"},
  {17, "\xd6\xb7"},
  {18, "\xd6\xb8"},
  {19, "\xd6\xb9"},
  {20, "\xd6\xbb"},
  {21, "\xd6\xbc"},
  {22, "\xd6\xbd"},
  {23, "\xd6\xbf"},
  {24, "\xd7\x81"},
  {25, "\xd7\x82"},
  {26, "\xef\xac\x9e"},
  {27, "\xd9\x8b"},
  {28, "\xd9\x8c"},
  {29, "\xd9\x8d"},
  {30, "\xd8\x98"},
  {31, "\xd8\x99"},
  {32, "\xd8\x9a"},
  {33, "\xd9\x91"},
  {34, "\xd9\x92"},
  {35, "\xd9\xb0"},
  {36, "\xdc\x91"},
  {84, "\xe0\xb1\x95"},
  {91, "\xe0\xb1\x96"},
  {103, "\xe0\xb8\xb8"},
  {107, "\xe0\xb9\x88"},
  {118, "\xe0\xba\xb8"},
  {122, "\xe0\xbb\x88"},
  {129, "\xe0\xbd\xb1"},
  {130, "\xe0\xbd\xb2"},
  {132, "\xe0\xbd\xb4"},
  {202, "\xcc\xa1"},
  {214, "\xe1\xb7\x8e"},
  {216, "\xcc\x9b"},
  {218, "\xe1\xb7\xba"},
  {220, "\xcc\x96"},
  {222, "\xd6\x9a"},
  {224, "\xe3\x80\xae"},
  {226, "\xf0\x9d\x85\xad"},
  {228, "\xd6\xae"},
  {230, "\xcc\x80"},
  {232, "\xcc\x95"},
  {233, "\xcd\x9c"},
  {234, "\xcd\x9d"},
  {240, "\xcd\x85"},
};
/* END GENERATED GERMAN COMBINING CLASS BENCHMARK */

#define KEY_SIZE 65536
#define MAX_REPETITIONS 16
#define SAMPLES 5

static char descending[KEY_SIZE];
static char ascending[KEY_SIZE];
static char transformed_a[KEY_SIZE];
static char transformed_b[KEY_SIZE];
static volatile uint64_t sink;

static int build_inputs(size_t repetitions) {
  size_t count = sizeof combining_class_marks / sizeof combining_class_marks[0];
  if (count != 55 || repetitions > MAX_REPETITIONS) return 1;
  size_t d = 0, a = 0;
  descending[d++] = ascending[a++] = 'a';
  for (size_t repeat = 0; repeat < repetitions; ++repeat) {
    for (size_t i = count; i-- > 0;) {
      size_t n = strlen(combining_class_marks[i].utf8);
      memcpy(descending + d, combining_class_marks[i].utf8, n);
      d += n;
    }
    for (size_t i = 0; i < count; ++i) {
      size_t n = strlen(combining_class_marks[i].utf8);
      memcpy(ascending + a, combining_class_marks[i].utf8, n);
      a += n;
    }
  }
  descending[d] = ascending[a] = '\0';
  return 0;
}

static double elapsed_ns(struct timespec start, struct timespec end) {
  return (end.tv_sec - start.tv_sec) * 1e9 + (end.tv_nsec - start.tv_nsec);
}

static double bench_collate(size_t iterations) {
  double samples[SAMPLES];
  for (size_t sample = 0; sample < SAMPLES; ++sample) {
    struct timespec start, end;
    uint64_t local = 0;
    clock_gettime(CLOCK_MONOTONIC, &start);
    for (size_t i = 0; i < iterations; ++i)
      local += (uint64_t)(strcoll(descending, ascending) + 256);
    clock_gettime(CLOCK_MONOTONIC, &end);
    samples[sample] = elapsed_ns(start, end) / iterations;
    sink ^= local;
  }
  for (size_t i = 1; i < SAMPLES; ++i) {
    double x = samples[i]; size_t j = i;
    while (j && samples[j - 1] > x) { samples[j] = samples[j - 1]; --j; }
    samples[j] = x;
  }
  return samples[SAMPLES / 2];
}

static double bench_transform(size_t iterations) {
  double samples[SAMPLES];
  for (size_t sample = 0; sample < SAMPLES; ++sample) {
    struct timespec start, end;
    uint64_t local = 0;
    clock_gettime(CLOCK_MONOTONIC, &start);
    for (size_t i = 0; i < iterations; ++i)
      local += strxfrm(transformed_a, descending, sizeof transformed_a);
    clock_gettime(CLOCK_MONOTONIC, &end);
    samples[sample] = elapsed_ns(start, end) / iterations;
    sink ^= local;
  }
  for (size_t i = 1; i < SAMPLES; ++i) {
    double x = samples[i]; size_t j = i;
    while (j && samples[j - 1] > x) { samples[j] = samples[j - 1]; --j; }
    samples[j] = x;
  }
  return samples[SAMPLES / 2];
}

int main(void) {
  if (setlocale(LC_CTYPE, "C.UTF-8") == NULL ||
      setlocale(LC_COLLATE, "de_DE.UTF-8") == NULL) {
    fputs("cannot select standalone German locale\n", stderr);
    return 77;
  }
  puts("profile,function,marks,iterations,median_ns_per_call");
  for (size_t repetitions = 1; repetitions <= MAX_REPETITIONS; repetitions *= 4) {
    size_t marks = repetitions * 55;
    size_t iterations = marks == 55 ? 100 : marks == 220 ? 25 : 6;
    if (build_inputs(repetitions) != 0 || strcoll(descending, ascending) != 0)
      return 1;
    size_t a = strxfrm(transformed_a, descending, sizeof transformed_a);
    size_t b = strxfrm(transformed_b, ascending, sizeof transformed_b);
    if (a != b || a >= sizeof transformed_a || strcmp(transformed_a, transformed_b) != 0)
      return 1;
    printf("%s,strcoll,%zu,%zu,%.2f\n", BENCH_COMPILER, marks, iterations,
           bench_collate(iterations));
    printf("%s,strxfrm,%zu,%zu,%.2f\n", BENCH_COMPILER, marks, iterations,
           bench_transform(iterations));
  }
  (void)sink;
  return 0;
}
