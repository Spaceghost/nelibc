#ifndef NELIBC_STDIO_STANDALONE_H
#define NELIBC_STDIO_STANDALONE_H

#include <stddef.h>
#include <stdarg.h>
#include <wchar.h>

#ifndef SEEK_SET
#define SEEK_SET 0
#define SEEK_CUR 1
#define SEEK_END 2
#endif

typedef struct nelibc_FILE {
  int fd;
  unsigned int flags;
  int pushback_value;
  int has_pushback;
  size_t read_position;
  size_t read_length;
  size_t write_length;
  int lock_word;
  int lock_owner;
  int lock_depth;
  size_t buffer_size;
  unsigned char *user_buffer;
  struct nelibc_FILE *next;
  unsigned char buffer[4096];
  unsigned char **memstream_buffer;
  size_t *memstream_size;
  int orientation;
  unsigned long wide_read_state[16];
  unsigned long wide_write_state[16];
  int has_wide_pushback;
  int wide_pushback_value;
} FILE;

typedef long fpos_t;

extern FILE nelibc_stdin_FILE;
extern FILE nelibc_stdout_FILE;
extern FILE nelibc_stderr_FILE;

#define stdin (&nelibc_stdin_FILE)
#define stdout (&nelibc_stdout_FILE)
#define stderr (&nelibc_stderr_FILE)
#define EOF (-1)
#define NELIBC_FILE_EOF 1u
#define NELIBC_FILE_ERROR 2u
#define NELIBC_FILE_READABLE 4u
#define NELIBC_FILE_WRITABLE 8u
#define NELIBC_FILE_LINE_BUFFERED 16u
#define NELIBC_FILE_UNBUFFERED 32u
#define _IOFBF 0
#define _IOLBF 1
#define _IONBF 2
#define BUFSIZ 4096

int getchar(void);
int putchar(int character);
int puts(const char *string);
int printf(const char *restrict format, ...);
int vprintf(const char *restrict format, va_list arguments);
int snprintf(char *restrict buffer, size_t size, const char *restrict format, ...);
int vsnprintf(char *restrict buffer, size_t size, const char *restrict format, va_list arguments);
int asprintf(char **restrict buffer, const char *restrict format, ...);
int vasprintf(char **restrict buffer, const char *restrict format, va_list arguments);
int sprintf(char *restrict buffer, const char *restrict format, ...);
int vsprintf(char *restrict buffer, const char *restrict format, va_list arguments);
int dprintf(int fd, const char *restrict format, ...);
int vdprintf(int fd, const char *restrict format, va_list arguments);
int fgetc(FILE *stream);
int getc(FILE *stream);
int getchar_unlocked(void);
int putchar_unlocked(int character);
long getdelim(char **restrict lineptr, size_t *restrict capacity, int delimiter, FILE *restrict stream);
long getline(char **restrict lineptr, size_t *restrict capacity, FILE *restrict stream);
char *fgets(char *restrict buffer, int count, FILE *restrict stream);
char *fgets_unlocked(char *restrict buffer, int count, FILE *restrict stream);
int ungetc(int character, FILE *stream);
int fputc(int character, FILE *stream);
int fputs(const char *restrict string, FILE *restrict stream);
int fputs_unlocked(const char *restrict string, FILE *restrict stream);
int fprintf(FILE *restrict stream, const char *restrict format, ...);
int vfprintf(FILE *restrict stream, const char *restrict format, va_list arguments);
size_t fread(void *restrict buffer, size_t size, size_t count, FILE *restrict stream);
size_t fwrite(const void *restrict buffer, size_t size, size_t count, FILE *restrict stream);
FILE *fopen(const char *restrict path, const char *restrict mode);
FILE *freopen(const char *restrict path, const char *restrict mode, FILE *restrict stream);
FILE *fmemopen(void *restrict buffer, size_t size, const char *restrict mode);
FILE *open_memstream(char **restrict buffer, size_t *restrict size);
FILE *fdopen(int fd, const char *restrict mode);
int fileno(FILE *stream);
FILE *tmpfile(void);
int remove(const char *path);
int rename(const char *old_path, const char *new_path);
int fclose(FILE *stream);
int fflush(FILE *stream);
int setvbuf(FILE *restrict stream, char *restrict buffer, int mode, size_t size);
void setbuf(FILE *restrict stream, char *restrict buffer);
void setbuffer(FILE *restrict stream, char *restrict buffer, size_t size);
void setlinebuf(FILE *stream);
int fseek(FILE *stream, long offset, int whence);
int fseeko(FILE *stream, long offset, int whence);
long ftell(FILE *stream);
long ftello(FILE *stream);
int fgetpos(FILE *restrict stream, fpos_t *restrict position);
int fsetpos(FILE *stream, const fpos_t *position);
void rewind(FILE *stream);
void flockfile(FILE *stream);
int ftrylockfile(FILE *stream);
void funlockfile(FILE *stream);
int feof(FILE *stream);
int ferror(FILE *stream);
void clearerr(FILE *stream);
void perror(const char *string);

#endif
