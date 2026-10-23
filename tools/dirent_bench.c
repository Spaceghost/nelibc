#define _GNU_SOURCE
#include <dirent.h>
#include <errno.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
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
#define ITERATIONS 1000u
#define ENTRY_COUNT 64

static char directory_path[] = "/tmp/nelibc-dirent-XXXXXX";
static double samples[SAMPLES];

static double now_seconds(void) {
  struct timespec ts;
  if (clock_gettime(CLOCK_MONOTONIC, &ts) != 0) return 0.0;
  return (double)ts.tv_sec + (double)ts.tv_nsec / 1000000000.0;
}

static void sort_samples(void) {
  for (size_t i = 1; i < SAMPLES; ++i) {
    double value = samples[i];
    size_t j = i;
    while (j != 0 && samples[j - 1] > value) {
      samples[j] = samples[j - 1];
      --j;
    }
    samples[j] = value;
  }
}

static int prepare_directory(void) {
  if (mkdtemp(directory_path) == NULL) return -1;
  for (int i = 0; i < ENTRY_COUNT; ++i) {
    char path[sizeof directory_path + 16];
    int length = snprintf(path, sizeof path, "%s/entry-%03d", directory_path, i);
    if (length <= 0 || (size_t)length >= sizeof path) return -1;
    int fd = open(path, O_WRONLY | O_CREAT | O_EXCL, 0600);
    if (fd < 0 || close(fd) != 0) return -1;
  }
  return 0;
}

static int cleanup_directory(void) {
  for (int i = 0; i < ENTRY_COUNT; ++i) {
    char path[sizeof directory_path + 16];
    int length = snprintf(path, sizeof path, "%s/entry-%03d", directory_path, i);
    if (length <= 0 || (size_t)length >= sizeof path || unlink(path) != 0) return -1;
  }
  return rmdir(directory_path);
}

static int operation_open_close(void) {
  int (*volatile open_call)(const char *, int, ...) = open;
  int (*volatile close_call)(int) = close;
  int fd = open_call(directory_path, O_RDONLY | O_DIRECTORY);
  return fd >= 0 && close_call(fd) == 0 ? 0 : -1;
}

static int operation_opendir_close(void) {
  DIR *(*volatile open_call)(const char *) = opendir;
  int (*volatile close_call)(DIR *) = closedir;
  DIR *directory = open_call(directory_path);
  return directory != NULL && close_call(directory) == 0 ? 0 : -1;
}

static int scan_entries(DIR *directory) {
  struct dirent *(*volatile read_call)(DIR *) = readdir;
  unsigned int count = 0;
  struct dirent *entry;
  while ((entry = read_call(directory)) != NULL) {
    if (entry->d_name[0] != '.') ++count;
  }
  return count == ENTRY_COUNT ? 0 : -1;
}

static int operation_full_scan(void) {
  DIR *(*volatile open_call)(const char *) = opendir;
  int (*volatile close_call)(DIR *) = closedir;
  DIR *directory = open_call(directory_path);
  if (directory == NULL) return -1;
  int scanned = scan_entries(directory);
  int result = close_call(directory);
  return result == 0 && scanned == 0 ? 0 : -1;
}

static DIR *reused_directory;

static int operation_rewind_scan(void) {
  void (*volatile rewind_call)(DIR *) = rewinddir;
  rewind_call(reused_directory);
  return scan_entries(reused_directory);
}

static int report(const char *function, int (*operation)(void)) {
  for (size_t sample = 0; sample < SAMPLES; ++sample) {
    double start = now_seconds();
    for (size_t iteration = 0; iteration < ITERATIONS; ++iteration) {
      if (operation() != 0) {
        fprintf(stderr, "%s failed: errno=%d\n", function, errno);
        return -1;
      }
    }
    samples[sample] = (now_seconds() - start) * 1e9 / ITERATIONS;
  }
  sort_samples();
  printf("%s,%s,%s,%s,%u,%.2f\n", BENCH_COMPILER, BENCH_LIBC,
         BENCH_IMPLEMENTATION, function, ITERATIONS, samples[SAMPLES / 2]);
  return 0;
}

int main(void) {
  if (prepare_directory() != 0) {
    fprintf(stderr, "directory setup failed: errno=%d\n", errno);
    return 1;
  }
  reused_directory = opendir(directory_path);
  if (reused_directory == NULL || report("open-close", operation_open_close) != 0 ||
      report("opendir-closedir", operation_opendir_close) != 0 ||
      report("readdir-rewind-64", operation_rewind_scan) != 0 ||
      report("opendir-readdir-64-closedir", operation_full_scan) != 0) {
    if (reused_directory != NULL) closedir(reused_directory);
    cleanup_directory();
    return 1;
  }
  if (closedir(reused_directory) != 0) {
    cleanup_directory();
    return 1;
  }
  if (cleanup_directory() != 0) {
    fprintf(stderr, "directory cleanup failed: errno=%d\n", errno);
    return 1;
  }
  return 0;
}
