#include <errno.h>
#include <stdio.h>
#include <string.h>

static void bytes(const char *label, const unsigned char *p, size_t n) {
  printf("%s:", label);
  for (size_t i = 0; i < n; ++i) printf("%02x", p[i]);
  putchar('\n');
}

static void mode_case(const char *mode) {
  unsigned char b[8] = "abc";
  errno = 0;
  FILE *f = fmemopen(b, sizeof b, mode);
  printf("mode-%s:%d:%d\n", mode, f != NULL, errno);
  if (f) fclose(f);
  bytes("mode-buffer", b, sizeof b);
}

int main(void) {
  mode_case("r"); mode_case("r+"); mode_case("w"); mode_case("w+");
  mode_case("a"); mode_case("a+"); mode_case("rb"); mode_case("r+b");
  mode_case("r++"); mode_case("rx"); mode_case("q");

  unsigned char b[8] = "abc";
  FILE *f = fmemopen(b, sizeof b, "r+");
  if (!f) return 1;
  int r0 = fgetc(f), r1 = fgetc(f), r2 = fgetc(f);
  printf("read:%d:%d:%d\n", r0, r1, r2);
  printf("eof:%d\n", fgetc(f));
  errno = 0; int seek = fseek(f, 6, SEEK_SET);
  printf("seek-hole:%d:%d\n", seek, errno);
  errno = 0; int put = fputc('Z', f);
  printf("put-hole:%d:%d\n", put, errno);
  if (fflush(f)) return 2;
  bytes("hole-buffer", b, sizeof b);
  fclose(f);

  memset(b, 'x', sizeof b);
  f = fmemopen(b, sizeof b, "w+");
  if (!f) return 3;
  bytes("wplus-open", b, sizeof b);
  printf("write-full:%zu\n", fwrite("12345678", 1, 8, f));
  if (fflush(f)) return 4;
  bytes("wplus-full", b, sizeof b);
  errno = 0; seek = fseek(f, 0, SEEK_SET);
  printf("wplus-seek:%d:%d\n", seek, errno);
  fclose(f);

  memcpy(b, "abc", 4);
  f = fmemopen(b, sizeof b, "a+");
  if (!f) return 5;
  errno = 0; seek = fseek(f, 0, SEEK_SET);
  int first = fgetc(f);
  int appended = fputc('Q', f);
  if (fflush(f)) return 6;
  printf("append-seek:%d:%d:%d\n", seek, first, appended);
  bytes("append-buffer", b, sizeof b);
  fclose(f);

  errno = 0; f = fmemopen(b, 0, "w+");
  printf("zero-size:%d:%d\n", f != NULL, errno);
  if (f) fclose(f);
  return 0;
}
