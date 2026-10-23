#define _GNU_SOURCE
#include <stdio.h>
#include <sys/types.h>

#if defined(__GLIBC__)
extern int _IO_fclose(FILE *stream);
extern FILE *_IO_fdopen(int descriptor, const char *mode);
extern void _IO_flockfile(FILE *stream);
extern int _IO_ftrylockfile(FILE *stream);
extern void _IO_funlockfile(FILE *stream);
extern int _IO_fgetpos(FILE *stream, fpos_t *position);
extern int _IO_fsetpos(FILE *stream, const fpos_t *position);
extern int _IO_setvbuf(FILE *stream, char *buffer, int mode, size_t size);

int nelibc_host_fclose(FILE *stream) {
  return _IO_fclose(stream);
}

FILE *nelibc_host_fdopen(int descriptor, const char *mode) {
  return _IO_fdopen(descriptor, mode);
}

void nelibc_host_flockfile(FILE *stream) { _IO_flockfile(stream); }
int nelibc_host_ftrylockfile(FILE *stream) { return _IO_ftrylockfile(stream); }
void nelibc_host_funlockfile(FILE *stream) { _IO_funlockfile(stream); }
int nelibc_host_fgetpos(FILE *stream, fpos_t *position) {
  return _IO_fgetpos(stream, position);
}
int nelibc_host_fsetpos(FILE *stream, const fpos_t *position) {
  return _IO_fsetpos(stream, position);
}
int nelibc_host_setvbuf(FILE *stream, char *buffer, int mode, size_t size) {
  return _IO_setvbuf(stream, buffer, mode, size);
}
#else
extern FILE *__fdopen(int descriptor, const char *mode);

FILE *nelibc_host_fdopen(int descriptor, const char *mode) {
  return __fdopen(descriptor, mode);
}
#endif

extern ssize_t nelibc_open_memstream_write(void *cookie, const void *buffer,
                                           size_t size);
extern int nelibc_open_memstream_seek(void *cookie, off_t *offset,
                                      int whence);
extern int nelibc_open_memstream_close(void *cookie);
extern ssize_t nelibc_fmemopen_read(void *cookie, void *buffer, size_t size);
extern ssize_t nelibc_fmemopen_write(void *cookie, const void *buffer,
                                     size_t size);
extern int nelibc_fmemopen_seek(void *cookie, off_t *offset, int whence);
extern int nelibc_fmemopen_close(void *cookie);

static ssize_t nelibc_open_memstream_write_adapter(void *cookie,
                                                    const char *buffer,
                                                    size_t size) {
  return nelibc_open_memstream_write(cookie, buffer, size);
}

static int nelibc_open_memstream_seek_adapter(void *cookie, off_t *offset,
                                               int whence) {
  return nelibc_open_memstream_seek(cookie, (long *)offset, whence);
}

static int nelibc_open_memstream_close_adapter(void *cookie) {
  return nelibc_open_memstream_close(cookie);
}

FILE *nelibc_host_open_memstream(void *cookie) {
  cookie_io_functions_t functions = {0};
  functions.write = nelibc_open_memstream_write_adapter;
  functions.seek = nelibc_open_memstream_seek_adapter;
  functions.close = nelibc_open_memstream_close_adapter;
  return fopencookie(cookie, "w", functions);
}

static ssize_t nelibc_fmemopen_read_adapter(void *cookie, char *buffer,
                                             size_t size) {
  return nelibc_fmemopen_read(cookie, buffer, size);
}

static ssize_t nelibc_fmemopen_write_adapter(void *cookie, const char *buffer,
                                              size_t size) {
  return nelibc_fmemopen_write(cookie, buffer, size);
}

static int nelibc_fmemopen_seek_adapter(void *cookie, off_t *offset,
                                         int whence) {
  return nelibc_fmemopen_seek(cookie, offset, whence);
}

static int nelibc_fmemopen_close_adapter(void *cookie) {
  return nelibc_fmemopen_close(cookie);
}

FILE *nelibc_host_fmemopen_cookie(void *cookie, const char *mode) {
  cookie_io_functions_t functions = {0};
  functions.read = nelibc_fmemopen_read_adapter;
  functions.write = nelibc_fmemopen_write_adapter;
  functions.seek = nelibc_fmemopen_seek_adapter;
  functions.close = nelibc_fmemopen_close_adapter;
  return fopencookie(cookie, mode, functions);
}

int nelibc_host_fmemopen_glibc_termination(void) {
#if defined(__GLIBC__)
  return 1;
#else
  return 0;
#endif
}

#if defined(__GLIBC__)
extern char __libc_single_threaded __attribute__((weak));
#endif

static int nelibc_host_stdio_needs_lock(void) {
#if defined(__GLIBC__)
  if (&__libc_single_threaded != NULL)
    return !*(volatile unsigned char *)&__libc_single_threaded;
#endif
  return 1;
}

int nelibc_host_stream_lock(FILE *stream) {
  if (!nelibc_host_stdio_needs_lock()) return 0;
  flockfile(stream);
  return 1;
}

void nelibc_host_stream_unlock(FILE *stream, int lock_token) {
  if (lock_token) funlockfile(stream);
}

FILE *nelibc_host_stdout(void) { return stdout; }
FILE *nelibc_host_stdin(void) { return stdin; }
FILE *nelibc_host_stderr(void) { return stderr; }
size_t nelibc_host_bufsiz(void) { return BUFSIZ; }

#if defined(__GLIBC__)
/* Glibc exposes a separate entry point, so this bypasses our interposed
 * freopen symbol. Musl aliases freopen64 to freopen and must not export the
 * Nelua wrapper from its hybrid target. */
FILE *nelibc_host_freopen(const char *pathname, const char *mode, FILE *stream) {
  return freopen64(pathname, mode, stream);
}
#endif
