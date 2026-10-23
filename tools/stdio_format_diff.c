#include <stdarg.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>

static int call_vsprintf(char *buffer, const char *format, ...) {
  va_list arguments;
  va_start(arguments, format);
  int result = vsprintf(buffer, format, arguments);
  va_end(arguments);
  return result;
}

static int emit(const char *label, int length, const char *buffer) {
  if (printf("%s:%d:%s\n", label, length, buffer) < 0) return 1;
  return 0;
}

int main(void) {
  char buffer[256];
  int length = sprintf(buffer, "%#010x:%-8.5s:%+08d", 0x2a, "nelibc", -73);
  if (length != 28 || emit("sprintf-int", length, buffer)) return 1;

  length = call_vsprintf(buffer, "%#.6g:%+.2f:%.1e", 1234.5, -0.125, 0.03125);
  if (length < 0 || emit("vsprintf-float", length, buffer)) return 2;

  length = sprintf(buffer, "%lld:%zu:%td", -9223372036854775807LL,
                   (size_t)123456, (ptrdiff_t)-321);
  if (length < 0 || emit("sprintf-widths", length, buffer)) return 3;

  int written = -1;
  length = call_vsprintf(buffer, "prefix%n-suffix", &written);
  if (length != 13 || written != 6 || emit("vsprintf-n", length, buffer)) return 4;

  signed char written_hh = -1;
  short written_h = -1;
  long written_l = -1;
  long long written_ll = -1;
  ptrdiff_t written_z = -1, written_t = -1;
  intmax_t written_j = -1;
  length = sprintf(buffer, "abc%hhn-def%hn-gh%ln-ijk%lln-lmn%zn-op%tn-qr%jn-end",
                   &written_hh, &written_h, &written_l, &written_ll,
                   &written_z, &written_t, &written_j);
  if (length != 28 || written_hh != 3 || written_h != 7 || written_l != 10 ||
      written_ll != 14 || written_z != 18 || written_t != 21 || written_j != 24 ||
      emit("sprintf-n-lengths", length, buffer)) return 5;
  if (printf("counts:%d:%d:%ld:%lld:%td:%td:%jd\n", written_hh, written_h,
             written_l, written_ll, written_z, written_t, written_j) < 0) return 6;
  return 0;
}
