#define _GNU_SOURCE
#include <errno.h>
#include <fcntl.h>
#include <stdio.h>
#include <time.h>
#include <unistd.h>

#ifndef _PC_LINK_MAX
#define _PC_LINK_MAX 0
#define _PC_MAX_CANON 1
#define _PC_MAX_INPUT 2
#define _PC_NAME_MAX 3
#define _PC_PATH_MAX 4
#define _PC_PIPE_BUF 5
#define _PC_CHOWN_RESTRICTED 6
#define _PC_NO_TRUNC 7
#define _PC_VDISABLE 8
#define _PC_SYNC_IO 9
#define _PC_ASYNC_IO 10
#define _PC_PRIO_IO 11
#define _PC_SOCK_MAXBUF 12
#define _PC_FILESIZEBITS 13
#define _PC_REC_INCR_XFER_SIZE 14
#define _PC_REC_MAX_XFER_SIZE 15
#define _PC_REC_MIN_XFER_SIZE 16
#define _PC_REC_XFER_ALIGN 17
#define _PC_ALLOC_SIZE_MIN 18
#define _PC_SYMLINK_MAX 19
#define _PC_2_SYMLINKS 20
#endif
#ifndef BENCH_COMPILER
#define BENCH_COMPILER "unknown"
#endif
#ifndef BENCH_LIBC
#define BENCH_LIBC "unknown"
#endif
#ifndef BENCH_IMPLEMENTATION
#define BENCH_IMPLEMENTATION "unknown"
#endif

enum { SAMPLES = 7, ITERATIONS = 100000 };
static long (*volatile call_pathconf)(const char *, int) = pathconf;
static long (*volatile call_fpathconf)(int, int) = fpathconf;
static volatile long result_sink;
static const int selectors[] = {_PC_LINK_MAX, _PC_MAX_CANON, _PC_MAX_INPUT,
  _PC_NAME_MAX, _PC_PATH_MAX, _PC_PIPE_BUF, _PC_CHOWN_RESTRICTED,
  _PC_NO_TRUNC, _PC_VDISABLE, _PC_SYNC_IO, _PC_ASYNC_IO, _PC_PRIO_IO,
  _PC_SOCK_MAXBUF, _PC_FILESIZEBITS, _PC_REC_INCR_XFER_SIZE,
  _PC_REC_MAX_XFER_SIZE, _PC_REC_MIN_XFER_SIZE, _PC_REC_XFER_ALIGN,
  _PC_ALLOC_SIZE_MIN, _PC_SYMLINK_MAX, _PC_2_SYMLINKS};
static const char *const names[] = {"LINK_MAX", "MAX_CANON", "MAX_INPUT",
  "NAME_MAX", "PATH_MAX", "PIPE_BUF", "CHOWN_RESTRICTED", "NO_TRUNC",
  "VDISABLE", "SYNC_IO", "ASYNC_IO", "PRIO_IO", "SOCK_MAXBUF",
  "FILESIZEBITS", "REC_INCR_XFER_SIZE", "REC_MAX_XFER_SIZE",
  "REC_MIN_XFER_SIZE", "REC_XFER_ALIGN", "ALLOC_SIZE_MIN", "SYMLINK_MAX",
  "2_SYMLINKS"};

static double now_ns(void) {
  struct timespec t;
  clock_gettime(CLOCK_MONOTONIC, &t);
  return (double)t.tv_sec * 1e9 + (double)t.tv_nsec;
}

static double measure_path(const char *target_path, int selector) {
  double samples[SAMPLES];
  for (int s = 0; s < SAMPLES; ++s) {
    double start = now_ns();
    for (int i = 0; i < ITERATIONS; ++i) result_sink = call_pathconf(target_path, selector);
    samples[s] = (now_ns() - start) / ITERATIONS;
  }
  for (int i = 0; i < SAMPLES; ++i)
    for (int j = i + 1; j < SAMPLES; ++j)
      if (samples[j] < samples[i]) { double x = samples[i]; samples[i] = samples[j]; samples[j] = x; }
  return samples[SAMPLES / 2];
}

static double measure_fd(int fd, int selector) {
  double samples[SAMPLES];
  for (int s = 0; s < SAMPLES; ++s) {
    double start = now_ns();
    for (int i = 0; i < ITERATIONS; ++i) result_sink = call_fpathconf(fd, selector);
    samples[s] = (now_ns() - start) / ITERATIONS;
  }
  for (int i = 0; i < SAMPLES; ++i)
    for (int j = i + 1; j < SAMPLES; ++j)
      if (samples[j] < samples[i]) { double x = samples[i]; samples[i] = samples[j]; samples[j] = x; }
  return samples[SAMPLES / 2];
}

int main(int argc, char **argv) {
  const char *target_path = argc > 1 ? argv[1] : ".";
  int fd = open(target_path, O_RDONLY);
  if (fd < 0) { perror("open"); return 1; }
  int regular_fd = open("/proc/self/exe", O_RDONLY);
  if (regular_fd < 0) { perror("open /proc/self/exe"); close(fd); return 1; }
  for (unsigned i = 0; i < sizeof selectors / sizeof selectors[0]; ++i) {
    errno = 0;
    long p = call_pathconf(target_path, selectors[i]);
    int path_errno = errno;
    errno = 0;
    long f = call_fpathconf(fd, selectors[i]);
    int fd_errno = errno;
    if (p != f || path_errno != fd_errno || (p < 0 && path_errno != 0)) {
      fprintf(stderr, "%s mismatch: path=%ld errno=%d fd=%ld errno=%d\n",
        names[i], p, path_errno, f, fd_errno);
      close(fd);
      return 1;
    }
  }
  errno = 0;
  if (call_pathconf(target_path, -1) != -1 || errno != EINVAL) {
    fprintf(stderr, "pathconf invalid selector did not set EINVAL\n");
    close(fd);
    return 1;
  }
  errno = 0;
  long invalid_fd_result = call_fpathconf(-1, _PC_NAME_MAX);
#ifdef BENCH_NELIBC
  if (invalid_fd_result != -1 || errno != EBADF) {
    fprintf(stderr, "Nelibc fpathconf invalid fd returned %ld, errno=%d\n", invalid_fd_result, errno);
    close(fd);
    return 1;
  }
#endif
  int invalid_fd_errno = errno;
  errno = 0;
  long regular_path_async = call_pathconf("/proc/self/exe", _PC_ASYNC_IO);
  int regular_path_errno = errno;
  errno = 0;
  long regular_fd_async = call_fpathconf(regular_fd, _PC_ASYNC_IO);
  int regular_fd_errno = errno;
  if (regular_path_async != regular_fd_async || regular_path_errno != regular_fd_errno ||
      (regular_path_async < 0 && regular_path_errno != 0)) {
    fprintf(stderr, "regular-file ASYNC_IO mismatch: path=%ld errno=%d fd=%ld errno=%d\n",
      regular_path_async, regular_path_errno, regular_fd_async, regular_fd_errno);
    close(regular_fd);
    close(fd);
    return 1;
  }
#ifdef BENCH_NELIBC
  if (regular_path_async != 1) {
    fprintf(stderr, "Nelibc regular-file ASYNC_IO returned %ld\n", regular_path_async);
    close(regular_fd);
    close(fd);
    return 1;
  }
#endif
  puts("compiler,libc,implementation,target_path,function,selector,value,iterations,median_ns_per_call,probe,errno");
  for (unsigned i = 0; i < sizeof selectors / sizeof selectors[0]; ++i) {
    long value = call_pathconf(target_path, selectors[i]);
    printf("%s,%s,%s,%s,pathconf,%s,%ld,%d,%.2f,value,0\n", BENCH_COMPILER, BENCH_LIBC,
      BENCH_IMPLEMENTATION, target_path, names[i], value, ITERATIONS, measure_path(target_path, selectors[i]));
    printf("%s,%s,%s,%s,fpathconf,%s,%ld,%d,%.2f,value,0\n", BENCH_COMPILER, BENCH_LIBC,
      BENCH_IMPLEMENTATION, target_path, names[i], value, ITERATIONS, measure_fd(fd, selectors[i]));
  }
  printf("%s,%s,%s,%s,fpathconf,INVALID_FD,%ld,0,0.00,invalid_fd,%d\n", BENCH_COMPILER,
    BENCH_LIBC, BENCH_IMPLEMENTATION, target_path, invalid_fd_result, invalid_fd_errno);
  printf("%s,%s,%s,%s,pathconf,ASYNC_IO_REGULAR,%ld,0,0.00,regular_file,%d\n",
    BENCH_COMPILER, BENCH_LIBC, BENCH_IMPLEMENTATION, target_path, regular_path_async, regular_path_errno);
  printf("%s,%s,%s,%s,fpathconf,ASYNC_IO_REGULAR,%ld,0,0.00,regular_file,%d\n",
    BENCH_COMPILER, BENCH_LIBC, BENCH_IMPLEMENTATION, target_path, regular_fd_async, regular_fd_errno);
  close(regular_fd);
  close(fd);
  return 0;
}
