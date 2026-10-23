#define _GNU_SOURCE
#include <stdarg.h>
#include <stddef.h>
#include <stdio.h>

extern int nelibc_host_stream_lock(FILE *stream);
extern void nelibc_host_stream_unlock(FILE *stream, int lock_token);
extern int nelibc_vfprintf_host(void *stream, const char *format,
                                va_list arguments);

size_t nelibc_host_format_write(void *stream, const void *buffer, size_t count) {
  return fwrite_unlocked(buffer, 1, count, (FILE *)stream);
}

int vfprintf(FILE *stream, const char *format, va_list arguments) {
  int lock_token = nelibc_host_stream_lock(stream);
  int result = nelibc_vfprintf_host(stream, format, arguments);
  nelibc_host_stream_unlock(stream, lock_token);
  return result;
}

int fprintf(FILE *stream, const char *format, ...) {
  va_list arguments;
  va_start(arguments, format);
  int result = vfprintf(stream, format, arguments);
  va_end(arguments);
  return result;
}

int vprintf(const char *format, va_list arguments) {
  return vfprintf(stdout, format, arguments);
}

int printf(const char *format, ...) {
  va_list arguments;
  va_start(arguments, format);
  int result = vprintf(format, arguments);
  va_end(arguments);
  return result;
}
