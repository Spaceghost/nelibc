#define _GNU_SOURCE
#include "nelibc_stdio.h"
#include "nelibc_alloc.h"

#include <errno.h>
#include <locale.h>
#include <stdarg.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <unistd.h>

void *nelibc_hybrid_realloc(void *pointer, size_t size);
void nelibc_hybrid_free(void *pointer);

static int smoke_vsnprintf(char *buffer, size_t size, const char *format, ...) {
  va_list arguments;
  va_start(arguments, format);
  int result = vsnprintf(buffer, size, format, arguments);
  va_end(arguments);
  return result;
}

static int smoke_vfprintf(FILE *stream, const char *format, ...) {
  va_list arguments;
  va_start(arguments, format);
  int result = vfprintf(stream, format, arguments);
  va_end(arguments);
  return result;
}

static int smoke_vsprintf(char *buffer, const char *format, ...) {
  va_list arguments;
  va_start(arguments, format);
  int result = vsprintf(buffer, format, arguments);
  va_end(arguments);
  return result;
}

static int smoke_vprintf(const char *format, ...) {
  va_list arguments;
  va_start(arguments, format);
  int result = vprintf(format, arguments);
  va_end(arguments);
  return result;
}

static int smoke_vdprintf(int fd, const char *format, ...) {
  va_list arguments;
  va_start(arguments, format);
  int result = vdprintf(fd, format, arguments);
  va_end(arguments);
  return result;
}

static int smoke_vasprintf(char **buffer, const char *format, ...) {
  va_list arguments;
  va_start(arguments, format);
  int result = vasprintf(buffer, format, arguments);
  va_end(arguments);
  return result;
}

int main(void) {
  const char *active_numeric = setlocale(LC_NUMERIC, "C");
  if (active_numeric == NULL || strcmp(active_numeric, "C") != 0) return 45;
  struct lconv *locale_data = localeconv();
  if (locale_data == NULL || locale_data->decimal_point == NULL ||
      strcmp(locale_data->decimal_point, ".") != 0) return 44;

  FILE *opened = fopen("/dev/null", "w");
  if (opened == NULL || fclose(opened) != 0) return 29;

  char *allocated_format = NULL;
  if (asprintf(&allocated_format, "%s:%d", "asprintf", 9) != 10 ||
      strcmp(allocated_format, "asprintf:9") != 0) return 27;
  free(allocated_format);
  allocated_format = NULL;
  if (smoke_vasprintf(&allocated_format, "%s:%.1f", "vasprintf", 2.5) != 13 ||
      strcmp(allocated_format, "vasprintf:2.5") != 0) return 28;
  free(allocated_format);

  int fd_pipe[2];
  if (pipe(fd_pipe) != 0) return 24;
  if (dprintf(fd_pipe[1], "fd:%d", 42) != 5 ||
      smoke_vdprintf(fd_pipe[1], ":%.1f", 3.5) != 4 ||
      close(fd_pipe[1]) != 0) return 25;
  char fd_output[9];
  if (read(fd_pipe[0], fd_output, sizeof fd_output) != (ssize_t)sizeof fd_output ||
      memcmp(fd_output, "fd:42:3.5", sizeof fd_output) != 0 ||
      close(fd_pipe[0]) != 0) return 26;

  char formatted[64];
  if (sprintf(formatted, "%s:%d", "sprintf", 7) != 9 ||
      strcmp(formatted, "sprintf:7") != 0 ||
      smoke_vsprintf(formatted, "%s:%d", "vsprintf", 8) != 10 ||
      strcmp(formatted, "vsprintf:8") != 0) return 23;
  int formatted_length = snprintf(formatted, sizeof formatted, "%.40g",
                                  1.2345678901234567);
  if (formatted_length != 41 ||
      strcmp(formatted, "1.234567890123456690432135474111419171095") != 0)
    return 19;
  formatted_length = smoke_vsnprintf(formatted, sizeof formatted, "%#010.2f:%d",
                                     12.5, 7);
  if (formatted_length != 12 || strcmp(formatted, "0000012.50:7") != 0)
    return 20;

  int output_pipe[2];
  if (pipe(output_pipe) != 0) return 1;
  int saved_stdout = dup(STDOUT_FILENO);
  if (saved_stdout < 0 || fflush(stdout) != 0 ||
      dup2(output_pipe[1], STDOUT_FILENO) != STDOUT_FILENO) return 2;
  close(output_pipe[1]);
  if (putchar('P') != 'P' || printf("NUM%d", 7) != 4 ||
      smoke_vprintf("V%d", 8) != 2 ||
      puts("TCC") < 0 || fflush(stdout) != 0) return 3;
  if (dup2(saved_stdout, STDOUT_FILENO) != STDOUT_FILENO || close(saved_stdout) != 0)
    return 4;
  char output[11];
  if (read(output_pipe[0], output, sizeof output) != (ssize_t)sizeof output ||
      memcmp(output, "PNUM7V8TCC\n", sizeof output) != 0 || close(output_pipe[0]) != 0)
    return 5;

  int input_pipe[2];
  if (pipe(input_pipe) != 0 || write(input_pipe[1], "I", 1) != 1 ||
      close(input_pipe[1]) != 0) return 6;
  int saved_stdin = dup(STDIN_FILENO);
  if (saved_stdin < 0 || dup2(input_pipe[0], STDIN_FILENO) != STDIN_FILENO ||
      close(input_pipe[0]) != 0) return 7;
  if (getchar() != 'I' || dup2(saved_stdin, STDIN_FILENO) != STDIN_FILENO ||
      close(saved_stdin) != 0) return 8;

  int fdopen_pipe[2];
  if (pipe(fdopen_pipe) != 0 || write(fdopen_pipe[1], "D", 1) != 1 ||
      close(fdopen_pipe[1]) != 0) return 31;
  FILE *fdopen_stream = fdopen(fdopen_pipe[0], "r");
  if (fdopen_stream == NULL || fgetc(fdopen_stream) != 'D' ||
      fgetc(fdopen_stream) != EOF || !feof(fdopen_stream) ||
      ferror(fdopen_stream) || fileno(fdopen_stream) != fdopen_pipe[0]) return 32;
  clearerr(fdopen_stream);
  if (feof(fdopen_stream) || fclose(fdopen_stream) != 0) return 32;

  FILE *full_device = fopen("/dev/full", "w");
  if (full_device == NULL) return 40;
  int full_put = fputc('E', full_device);
  if (full_put == EOF && !ferror(full_device)) return 41;
  if (!ferror(full_device) && (fflush(full_device) != EOF || !ferror(full_device)))
    return 42;
  clearerr(full_device);
  if (ferror(full_device)) return 43;
  (void)fclose(full_device);

  FILE *stream = tmpfile();
  if (stream == NULL) return 9;
  if (setvbuf(stream, NULL, _IONBF, 0) != 0) return 35;
  flockfile(stream);
  if (ftrylockfile(stream) != 0) return 33;
  funlockfile(stream);
  funlockfile(stream);
  if (fputs("alpha", stream) < 0 || fputc('!', stream) != '!' ||
      fwrite("bc", 1, 2, stream) != 2 || fflush(stream) != 0 ||
      fseek(stream, 0, SEEK_SET) != 0 || ftell(stream) != 0) return 9;
  off_t wide_position = (off_t)0x100000000LL;
  if (fseeko(stream, wide_position, SEEK_SET) != 0 ||
      ftello(stream) != wide_position || fseeko(stream, 0, SEEK_SET) != 0)
    return 44;
  fpos_t saved_position;
  if (fgetpos(stream, &saved_position) != 0 || fgetc(stream) != 'a' ||
      fsetpos(stream, &saved_position) != 0) return 34;
  if (fgetc(stream) != 'a' || getc(stream) != 'l' || ungetc('L', stream) != 'L' ||
      fgetc(stream) != 'L') return 10;
  char contents[7] = {0};
  if (fread(contents, 1, 6, stream) != 6 || strcmp(contents, "pha!bc") != 0)
    return 11;
  if (fgetc(stream) != EOF || !feof(stream)) return 29;
  rewind(stream);
  if (feof(stream) || fgetc(stream) != 'a') return 30;
  FILE *identity = stream;
  if (freopen("/dev/null", "w", stream) != identity ||
      fputs("reopened", stream) < 0 || fclose(stream) != 0) return 12;

  FILE *setvbuf_stream = tmpfile();
  char setvbuf_storage[BUFSIZ];
  if (setvbuf_stream == NULL ||
      setvbuf(setvbuf_stream, setvbuf_storage, _IOFBF, sizeof setvbuf_storage) != 0 ||
      fputs("setvbuf", setvbuf_stream) < 0 || fclose(setvbuf_stream) != 0)
    return 45;

  FILE *setbuf_stream = tmpfile();
  char setbuf_storage[BUFSIZ];
  if (setbuf_stream == NULL) return 36;
  setbuf(setbuf_stream, setbuf_storage);
  if (fputs("buffered", setbuf_stream) < 0 || fclose(setbuf_stream) != 0)
    return 36;

  FILE *setbuffer_stream = tmpfile();
  char setbuffer_storage[BUFSIZ];
  if (setbuffer_stream == NULL) return 37;
  setbuffer(setbuffer_stream, setbuffer_storage, sizeof setbuffer_storage);
  if (fputs("setbuffer", setbuffer_stream) < 0 ||
      fclose(setbuffer_stream) != 0) return 37;

  FILE *setlinebuf_stream = tmpfile();
  if (setlinebuf_stream == NULL) return 38;
  setlinebuf(setlinebuf_stream);
  if (fputs("line buffered\n", setlinebuf_stream) < 0 ||
      fclose(setlinebuf_stream) != 0) return 38;

  FILE *unlocked_stream = tmpfile();
  char unlocked_line[32];
  if (unlocked_stream == NULL) return 39;
  if (fputs_unlocked("unlocked line\n", unlocked_stream) < 0 ||
      fflush(unlocked_stream) != 0 ||
      fseeko(unlocked_stream, 0, SEEK_SET) != 0 ||
      fgets_unlocked(unlocked_line, sizeof unlocked_line, unlocked_stream) == NULL ||
      strcmp(unlocked_line, "unlocked line\n") != 0 ||
      fclose(unlocked_stream) != 0) return 39;

  FILE *lines = tmpfile();
  if (lines == NULL || fputs("host buffer\nnelibc buffer\n", lines) < 0 ||
      fseek(lines, 0, SEEK_SET) != 0) return 13;
  char *host_line = NULL;
  size_t host_capacity = 0;
  ssize_t host_length = getline(&host_line, &host_capacity, lines);
  if (host_length != 12 || strcmp(host_line, "host buffer\n") != 0) return 14;
  free(host_line);
  if (fclose(lines) != 0) return 17;

  FILE *formatted_stream = tmpfile();
  if (formatted_stream == NULL ||
      fprintf(formatted_stream, "host-FILE:%d:", 42) != 13 ||
      smoke_vfprintf(formatted_stream, "%.1f", 3.5) != 3 ||
      fflush(formatted_stream) != 0 || fseek(formatted_stream, 0, SEEK_SET) != 0)
    return 21;
  char formatted_stream_output[17] = {0};
  if (fread(formatted_stream_output, 1, 16, formatted_stream) != 16 ||
      strcmp(formatted_stream_output, "host-FILE:42:3.5") != 0 ||
      fclose(formatted_stream) != 0) return 22;

  char *memstream_output = NULL;
  size_t memstream_size = 0;
  FILE *memstream = open_memstream(&memstream_output, &memstream_size);
  if (memstream == NULL || fwrite("alpha", 1, 5, memstream) != 5 ||
      fflush(NULL) != 0 || memstream_size != 5 ||
      strcmp(memstream_output, "alpha") != 0) return 39;
  if (fseek(memstream, 2, SEEK_SET) != 0 ||
      fwrite("XY", 1, 2, memstream) != 2 || fflush(memstream) != 0 ||
      memstream_size != 5 || strcmp(memstream_output, "alXYa") != 0 ||
      fseek(memstream, 7, SEEK_SET) != 0 ||
      fwrite("Z", 1, 1, memstream) != 1 || fflush(memstream) != 0 ||
      memstream_size != 8 ||
      memcmp(memstream_output, "alXYa\0\0Z\0", 9) != 0 ||
      fclose(memstream) != 0) return 40;
  free(memstream_output);

  char fmem_buffer[16] = "hello";
  FILE *fmem = fmemopen(fmem_buffer, sizeof fmem_buffer, "r+");
  if (fmem == NULL || fgetc(fmem) != 'h' || fseek(fmem, 1, SEEK_SET) != 0 ||
      fputc('A', fmem) != 'A' || fflush(fmem) != 0 ||
      strcmp(fmem_buffer, "hAllo") != 0 || fclose(fmem) != 0) return 41;
  FILE *owned_fmem = fmemopen(NULL, 8, "w+");
  char fmem_contents[4] = {0};
  if (owned_fmem == NULL || fputs("mem", owned_fmem) < 0 ||
      fseek(owned_fmem, 0, SEEK_SET) != 0 ||
      fread(fmem_contents, 1, 3, owned_fmem) != 3 ||
      strcmp(fmem_contents, "mem") != 0 || fclose(owned_fmem) != 0)
    return 42;

  void *nelibc_pointer = nelibc_malloc(1);
  if (nelibc_pointer == NULL || !nelibc_allocator_owns(nelibc_pointer)) return 16;
  ((unsigned char *)nelibc_pointer)[0] = 0xa5;
  void *nelibc_grown = nelibc_hybrid_realloc(nelibc_pointer, 64);
  if (nelibc_grown == NULL) {
    nelibc_hybrid_free(nelibc_pointer);
    return 16;
  }
  nelibc_pointer = nelibc_grown;
  if (((unsigned char *)nelibc_pointer)[0] != 0xa5 ||
      !nelibc_allocator_owns(nelibc_pointer)) return 16;
  nelibc_hybrid_free(nelibc_pointer);

  errno = ENOENT;
  perror("nelibc-stdio");
  if (errno != ENOENT) return 18;
  return 0;
}
