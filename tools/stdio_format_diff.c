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

  /* Hexadecimal floats: zero, negative zero, normals, a precision cut and a
     round-half-even tie.  No subnormal here: glibc prints 0x1p-1074 as
     0x0.0000000000001p-1022 while musl normalizes it to 0x1p-1074, both
     conforming, so a host diff cannot pin it.  The glibc form nelibc uses is
     checked in tools/syscall_smoke.c under NELIBC_STANDALONE instead. */
  static char big[4096];
  length = sprintf(big, "%a|%A|%a|%a|%.3a|%.0a", 0.0, -0.0, 1.0, 0x1p-1022,
                   0x1.fffp+3, 1.5);
  if (length < 0 || emit("sprintf-hexfloat", length, big)) return 7;

  /* Fixed notation at any precision: 2^-1021 has 1021 fraction digits. */
  length = sprintf(big, "%.1022f", 0x1p-1021);
  if (length != 1024 || emit("sprintf-fixed-1022", length, big)) return 8;
  length = sprintf(big, "%.300f|%.30f|%.25f", 0.8414709848078965, 1e300, -2.5);
  if (length < 0 || emit("sprintf-fixed-long", length, big)) return 9;
  /* Past 1074 fraction digits the expansion is exact and zero-filled. */
  length = sprintf(big, "%.1100f|%-1110.1080f|", 0x1p-1074, 1.0);
  if (length != 1102 + 1 + 1110 + 1 || emit("sprintf-fixed-tail", length, big)) return 10;
  length = sprintf(big, "%1200.1100f", -0x1p-1074);
  if (length != 1200 || emit("sprintf-fixed-width", length, big)) return 11;
  return 0;
}
