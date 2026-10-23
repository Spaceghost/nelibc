#include "string.h"
#include "nelibc_stdlib.h"
#include <ctype.h>
#include <errno.h>
#include <stdlib.h>
#include <math.h>

extern long nelibc_legacy_strtol(const char *, char **, int) __asm__("strtol");
extern long long nelibc_legacy_strtoll(const char *, char **, int) __asm__("strtoll");

static int compare_ints(const void *left, const void *right) {
  int a = *(const int *)left;
  int b = *(const int *)right;
  return (a > b) - (a < b);
}

int main(void) {
  if (strverscmp("file2", "file10") >= 0 || strverscmp("file1", "file01") <= 0 ||
      strverscmp("x0", "x00") <= 0 || strverscmp("a2b3", "a2b12") >= 0 ||
      strverscmp("same9", "same9") != 0) return 152;
  if (strcasecmp("NeLiBc", "nElIbC") != 0 || strcasecmp("A", "b") >= 0) return 146;
  if (strncasecmp("NELIBC-tail", "nelibc-head", 6) != 0 ||
      strncasecmp("A", "b", 1) >= 0 || strncasecmp("A", "a", 0) != 0) return 147;
  if (strcasecmp("@", "`") >= 0 || strcasecmp("[", "{") >= 0) return 148;
  const char high_case_left[] = {(char)0xE0, 0};
  const char high_case_right[] = {(char)0xC0, 0};
  if (strcasecmp(high_case_left, high_case_right) != 32 ||
      strncasecmp(high_case_left, high_case_right, 1) != 32) return 149;
  char *number_end = 0;
  errno = 0;
  if (strtol("  -0x7f-tail", &number_end, 0) != -127 || number_end == 0 || strcmp(number_end, "-tail") != 0) return 100;
  if (strtol("077xyz", &number_end, 0) != 63 || strcmp(number_end, "xyz") != 0) return 101;
  if (nelibc_legacy_strtol("0b101tail", &number_end, 0) != 0 || strcmp(number_end, "b101tail") != 0 ||
      nelibc_legacy_strtoll("0B101", &number_end, 2) != 0 || strcmp(number_end, "B101") != 0 ||
      __isoc23_strtol("0b101tail", &number_end, 0) != 5 || strcmp(number_end, "tail") != 0 ||
      __isoc23_strtoll("0B101", &number_end, 2) != 5 || *number_end != 0) return 110;
  if (__isoc23_strtol("0b110tail", &number_end, 0) != 6 || strcmp(number_end, "tail") != 0 ||
      __isoc23_strtoll("0B111", &number_end, 0) != 7 || *number_end != 0 ||
      __isoc23_strtoul("0b1000", &number_end, 0) != 8 || *number_end != 0 ||
      __isoc23_strtoull("0B1001", &number_end, 0) != 9 || *number_end != 0) return 111;
  if (strtol("0x", &number_end, 0) != 0 || number_end == 0 || *number_end != 'x') return 102;
  if (strtol("  +no digits", &number_end, 10) != 0 || number_end == 0 || *number_end != ' ') return 103;
  number_end = 0;
  long invalid_base_result = strtol("12", &number_end, 1);
  if (invalid_base_result != 0) return 104;
  if (errno != EINVAL) return 108;
  if (number_end == 0 || *number_end != '1') return 109;
  errno = 0;
  if (strtol("9223372036854775808!", &number_end, 10) != 0x7FFFFFFFFFFFFFFFLL ||
      errno != ERANGE || strcmp(number_end, "!") != 0) return 105;
  errno = 0;
  if (strtoll("-9223372036854775809", &number_end, 10) != (-0x7FFFFFFFFFFFFFFFLL - 1) || errno != ERANGE) return 106;
  errno = 0;
  if (strtoul("-1", &number_end, 10) != ~0UL || errno != 0 || *number_end != 0) return 111;
  if (strtoul("-0x10tail", &number_end, 0) != ~0UL - 15UL || strcmp(number_end, "tail") != 0) return 112;
  if (strtoull("18446744073709551615!", &number_end, 10) != ~0ULL || strcmp(number_end, "!") != 0) return 113;
  errno = 0;
  if (strtoull("18446744073709551616!", &number_end, 10) != ~0ULL || errno != ERANGE ||
      strcmp(number_end, "!") != 0) return 114;
  if (strtod(" -123.5e2 rest", &number_end) != -12350.0 || strcmp(number_end, " rest") != 0) return 115;
  if (atof(" -12.5e1tail") != -125.0 || atof("0x1.8p2") != 6.0) return 122;
  if (setenv("NELIBC_ABI_SETENV_PROBE", "first", 1) != 0 ||
      strcmp(getenv("NELIBC_ABI_SETENV_PROBE"), "first") != 0 ||
      setenv("NELIBC_ABI_SETENV_PROBE", "ignored", 0) != 0 ||
      strcmp(getenv("NELIBC_ABI_SETENV_PROBE"), "first") != 0 ||
      setenv("NELIBC_ABI_SETENV_PROBE", "second", 1) != 0 ||
      strcmp(getenv("NELIBC_ABI_SETENV_PROBE"), "second") != 0) return 123;
  errno = 0;
  if (setenv("", "value", 1) != -1 || errno != EINVAL) return 124;
  errno = 0;
  if (setenv("BAD=NAME", "value", 1) != -1 || errno != EINVAL) return 125;
  if (setenv("NELIBC_ABI_SETENV_EMPTY", "", 1) != 0 ||
      strcmp(getenv("NELIBC_ABI_SETENV_EMPTY"), "") != 0) return 126;
  if (unsetenv("NELIBC_ABI_SETENV_PROBE") != 0 ||
      getenv("NELIBC_ABI_SETENV_PROBE") != NULL ||
      unsetenv("NELIBC_ABI_SETENV_PROBE") != 0) return 127;
  errno = 0;
  if (unsetenv("") != -1 || errno != EINVAL) return 128;
  errno = 0;
  if (unsetenv("BAD=NAME") != -1 || errno != EINVAL) return 129;
  if (strtod("0x1.8p+2!", &number_end) != 6.0 || strcmp(number_end, "!") != 0 ||
      strtod(".5", &number_end) != 0.5) return 116;
  if (strtod("1e+", &number_end) != 1.0 || *number_end != 'e' ||
      strtod(" x", &number_end) != 0.0 || number_end[0] != ' ') return 117;
  if (!isinf(strtod("-INFINITY!", &number_end)) || strcmp(number_end, "!") != 0 ||
      !isnan(strtod("nan(payload)", &number_end)) || *number_end != 0) return 118;
  if (strtod("1.00000000000000011102230246251565404236316680908203125", &number_end) != 1.0 ||
      strtod("1.0000000000000002", &number_end) <= 1.0 ||
      strtod("4.9406564584124654e-324", &number_end) == 0.0) return 121;
  if (strtof(" -12.5e1tail", &number_end) != -125.0f || strcmp(number_end, "tail") != 0 ||
      strtof("0x1.8p2", &number_end) != 6.0f || *number_end != 0) return 124;
  if (strtof("1.000000059604644775390625", &number_end) != 1.0f) return 127;
  if (strtof("1.000000059604644775390626", &number_end) <= 1.0f) return 128;
  if (strtof("0x1.000001p0", &number_end) != 1.0f ||
      strtof("0x1.000001000001p0", &number_end) <= 1.0f) return 132;
  if (strtof("0x1p-150", &number_end) != 0.0f ||
      strtof("0x1.0000000000001p-150", &number_end) == 0.0f) return 133;
  if (strtod("0x1.00000000000008p0", &number_end) != 1.0 ||
      strtod("0x1.0000000000000800000000000001p0", &number_end) <= 1.0) return 134;
  if (strtof("7.00649232162408535461864791644958065640130970938257885878534141944895541342930300743319094181060791015625e-46", &number_end) != 0.0f) return 129;
  if (strtof("7.00649232162408535461864791644958065640130970938257885878534141944895541342930300743319094181060791015626e-46", &number_end) == 0.0f) return 130;
  errno = 0;
  if (!isinf(strtof("1e100", &number_end)) || errno != ERANGE) return 125;
  errno = 0;
  if (!isinf(strtof("inf", &number_end)) || errno != 0) return 131;
  errno = 0;
  if (strtof("1e-100", &number_end) != 0.0f || errno != ERANGE) return 126;
  if (strtold("1.0000000000000000001", &number_end) <= 1.0L || *number_end != 0 ||
      strtold("1e400", &number_end) < 1.0e300L ||
      strtold("0x1p+16380", &number_end) < 1.0e4000L) return 135;
  if (strtold("0x1.0000000000000001p0", &number_end) != 1.0L) return 138;
  if (strtold("0x1.0000000000000001000001p0", &number_end) <= 1.0L) return 140;
  if (strtold("0x1p-16446", &number_end) != 0.0L) return 141;
  if (strtold("0x1.0000000000000001p-16446", &number_end) == 0.0L) return 142;
  if (strtold("0x1p-16445", &number_end) == 0.0L) return 143;
  if (strtold("1.0000000000000000000542101086242752217003726400434970855712890625", &number_end) != 1.0L ||
      strtold("1.0000000000000000000542101086242752217003726400434970855712890626", &number_end) <= 1.0L) return 139;
  if (!signbit(strtod("-0", &number_end)) || !signbit(strtof("-0", &number_end)) ||
      !signbit(strtold("-0", &number_end))) return 144;
  errno = 0;
  if (!isinf(strtold("1e5000", &number_end)) || errno != ERANGE) return 136;
  errno = 0;
  if (strtold("1e-5000", &number_end) != 0.0L || errno != ERANGE) return 137;
  errno = 0;
  if (!isinf(strtod("1e9999", &number_end)) || errno != ERANGE) return 119;
  errno = 0;
  if (strtod("1e-9999", &number_end) != 0.0 || errno != ERANGE) return 120;
  int sortable[] = {7, -2, 9, 0, 4, 4};
  div_t int_quotient = div(-17, 5);
  ldiv_t long_quotient = ldiv(29L, -6L);
  lldiv_t long_long_quotient = lldiv(-9223372036854775000LL, 7LL);
  if (int_quotient.quot != -3 || int_quotient.rem != -2 ||
      long_quotient.quot != -4 || long_quotient.rem != 5 ||
      long_long_quotient.quot != -1317624576693539285LL ||
      long_long_quotient.rem != -5) return 151;
  qsort(sortable, sizeof(sortable) / sizeof(sortable[0]), sizeof(sortable[0]), compare_ints);
  const int sorted[] = {-2, 0, 4, 4, 7, 9};
  for (size_t i = 0; i < sizeof(sorted) / sizeof(sorted[0]); ++i)
    if (sortable[i] != sorted[i]) return 122;
  int wanted = 7;
  int *found = bsearch(&wanted, sortable, sizeof(sortable) / sizeof(sortable[0]),
                       sizeof(sortable[0]), compare_ints);
  if (found == 0 || *found != wanted ||
      bsearch(&(int){3}, sortable, sizeof(sortable) / sizeof(sortable[0]),
              sizeof(sortable[0]), compare_ints) != 0) return 123;
  srand(0x12345678U);
  int random_first = rand();
  int random_second = rand();
  srand(0x12345678U);
  if (random_first < 0 || random_first > RAND_MAX ||
      random_second < 0 || random_second > RAND_MAX ||
      rand() != random_first || rand() != random_second) return 145;
  if (atoi(" -42tail") != -42 || atol("123") != 123 || atoll("-456") != -456 ||
      abs(-7) != 7 || labs(-9L) != 9L || llabs(-11LL) != 11LL) return 107;
  if (!isalnum('A') || !isalnum('7') || isalnum('!')) return 90;
  if (!isalpha('z') || isalpha('4') || !isdigit('4') || isdigit('x')) return 91;
  if (!isblank(' ') || !isblank('\t') || isblank('\n') || !isspace('\n') || isspace('x')) return 92;
  if (!iscntrl(0) || !iscntrl(127) || iscntrl(' ') || !isprint(' ') || isprint(127)) return 93;
  if (!isgraph('!') || isgraph(' ') || !ispunct('!') || ispunct('A')) return 94;
  if (!isupper('Z') || isupper('z') || !islower('z') || islower('Z')) return 95;
  if (!isxdigit('f') || !isxdigit('F') || !isxdigit('9') || isxdigit('g')) return 96;
  if (tolower('Q') != 'q' || tolower('q') != 'q' || toupper('q') != 'Q' || toupper('Q') != 'Q') return 97;
  if (!isascii(127) || isascii(128) || toascii(0xFF) != 127) return 98;
  if (isalpha(-1) || isdigit(256) || tolower(-1) != -1 || toupper(256) != 256) return 99;
  char src[] = "nelua";
  char dst[sizeof src] = {0};
  char overlap[] = "abcd";
  unsigned char binary[] = {0x10, 0xFF, 0x20, 0xFF};
  unsigned char binary_needle[] = {0x20, 0xFF};
  unsigned char absent_needle[] = {0x20, 0xFF, 0xA0};
  unsigned char stopped_copy[6] = {0xA5, 0xA5, 0xA5, 0xA5, 0xA5, 0xA5};
  unsigned char stop_source[17];
  unsigned char stop_dest[18];
  unsigned char scan_left[65];
  unsigned char scan_right[65];
  char overlap_search[] = "ababa";
  char long_search[] = "12345678needle";
  char copied[8] = {0};
  unsigned char unaligned_copy[19] = {0};
  unsigned char move_bytes[40];
  char bounded_copy[4] = {'x', 'x', 'x', 'x'};
  char one_byte_copy[1] = {'x'};
  char bounded_cat[5] = "ab";
  char full_cat[8] = "ab";
  char unterminated_cat[3] = {'a', 'b', 'c'};
  char end_copied[8] = {0};
  char padded[5] = {'x', 'x', 'x', 'x', 'x'};
  char padded_end[6] = {'x', 'x', 'x', 'x', 'x', 'x'};
  char exact_end[4] = {'x', 'x', 'x', 'x'};
  char joined[16] = "nel";
  const char *phrase = "nelua libc runtime";
  const char *span_text = "aaabbbc";
  char *bad_descriptor_message = strerror(9);
  char *unknown_error_message = strerror(-1);
  char *duplicate = strdup(src);
  char *bounded_duplicate = strndup("truncate", 4);
  char *short_duplicate = strndup("hi", 8);
  char *empty_duplicate = strndup("hi", 0);
  char token_text[] = ",,alpha,,,beta,";
  char unsplit[] = "whole";
  char comma_text[] = "a,b";
  char colon_text[] = "1:2";
  char *comma_state = 0;
  char *colon_state = 0;
  char field_text[] = ",alpha,,beta,";
  char *field_state = field_text;
  char *field;
  char whole_text[] = "whole";
  char *whole_state = whole_text;

  if (strlen(src) != 5) return 1;
  for (size_t i = 0; i < sizeof scan_left; ++i) scan_left[i] = scan_right[i] = (unsigned char)i;
  scan_left[8] = 0xFF;
  scan_left[16] = 0xFF;
  if (memcmp(scan_left, scan_right, 7) != 0 || memcmp(scan_left, scan_right, 17) <= 0) return 82;
  scan_right[8] = 0xFF;
  if (memcmp(scan_left, scan_right, 16) != 0 || memcmp(scan_left, scan_right, 17) <= 0) return 85;
  scan_left[8] = 0;
  if (memcmp(scan_left, scan_right, 17) >= 0) return 86;
  scan_left[8] = 0xFF;
  if (memcpy(unaligned_copy + 1, scan_left + 1, 15) != unaligned_copy + 1 || memcmp(unaligned_copy + 1, scan_left + 1, 15) != 0) return 87;
  for (size_t i = 0; i < sizeof scan_left; ++i) scan_left[i] = scan_right[i] = (unsigned char)i;
  scan_left[31] = 0xFF;
  if (memcmp(scan_left, scan_right, 31) != 0 || memcmp(scan_left, scan_right, 32) <= 0) return 88;
  scan_left[31] = scan_right[31];
  scan_right[32] = 0xFF;
  if (memcmp(scan_left, scan_right, 33) >= 0) return 89;
  scan_right[32] = scan_left[32];
  scan_left[63] = 0xFF;
  if (memcmp(scan_left, scan_right, 64) <= 0) return 90;
  scan_left[63] = scan_right[63];
  scan_left[8] = scan_left[16] = 0xFF;
  if (memchr(scan_left, 0xFF, 17) != scan_left + 8 || memrchr(scan_left, 0xFF, 17) != scan_left + 16) return 83;
  if (memmem(long_search, sizeof long_search - 1, "needle", 6) != long_search + 8) return 84;
  if (duplicate == 0 || duplicate == src || strcmp(duplicate, src) != 0) return 58;
  free(duplicate);
  if (bounded_duplicate == 0 || strcmp(bounded_duplicate, "trun") != 0) return 59;
  if (short_duplicate == 0 || strcmp(short_duplicate, "hi") != 0) return 60;
  if (empty_duplicate == 0 || empty_duplicate[0] != 0) return 61;
  free(bounded_duplicate);
  free(short_duplicate);
  free(empty_duplicate);
  if (strnlen(src, 0) != 0 || strnlen(src, 3) != 3 || strnlen(src, 8) != 5) return 16;
  if (memcpy(dst, src, sizeof src) != dst) return 2;
  if (memcmp(dst, src, sizeof src) != 0) return 3;
  if (mempcpy(dst, src, sizeof src) != dst + sizeof src || memcmp(dst, src, sizeof src) != 0) return 95;
  if (mempcpy(dst, src, 0) != dst) return 96;
  if (memccpy(stopped_copy, binary, 0x1FF, sizeof binary) != stopped_copy + 2) return 53;
  if (stopped_copy[0] != 0x10 || stopped_copy[1] != 0xFF || stopped_copy[2] != 0xA5) return 54;
  if (memccpy(stopped_copy, binary, 0xA0, sizeof binary) != 0) return 55;
  if (memcmp(stopped_copy, binary, sizeof binary) != 0) return 56;
  if (memccpy(stopped_copy, binary, 0x10, 0) != 0) return 57;
  for (size_t i = 0; i < sizeof stop_source; ++i) stop_source[i] = 0x11;
  for (size_t i = 0; i < sizeof stop_dest; ++i) stop_dest[i] = 0xA5;
  stop_source[8] = 0xFE;
  if (memccpy(stop_dest, stop_source, 0x1FE, sizeof stop_source) != stop_dest + 9) return 92;
  if (memcmp(stop_dest, stop_source, 9) != 0 || stop_dest[9] != 0xA5) return 93;
  stop_source[8] = 0x11;
  stop_source[16] = 0xFE;
  if (memccpy(stop_dest, stop_source, 0xFE, sizeof stop_source) != stop_dest + 17) return 94;
  if (memchr(binary, 0x1FF, sizeof binary) != binary + 1) return 42;
  if (memchr(binary, 0x20, 2) != 0 || memchr(binary, 0, 0) != 0) return 43;
  if (memchr(binary, 0x10, sizeof binary) != binary) return 44;
  if (memrchr(binary, 0x1FF, sizeof binary) != binary + 3) return 50;
  if (memrchr(binary, 0x10, sizeof binary) != binary || memrchr(binary, 0x20, 1) != 0) return 51;
  if (memrchr(binary, 0x10, 0) != 0) return 52;
  if (memmem(binary, sizeof binary, binary_needle, sizeof binary_needle) != binary + 2) return 45;
  if (memmem(overlap_search, 5, "aba", 3) != overlap_search) return 46;
  if (memmem(binary, sizeof binary, absent_needle, sizeof absent_needle) != 0) return 47;
  if (memmem(binary, 1, binary_needle, sizeof binary_needle) != 0) return 48;
  if (memmem(binary, 0, binary, 0) != binary) return 49;
  if (memmove(overlap + 1, overlap, 3) != overlap + 1) return 4;
  if (overlap[0] != 'a' || overlap[1] != 'a' || overlap[2] != 'b' || overlap[3] != 'c') return 5;
  for (size_t i = 0; i < sizeof move_bytes; ++i) move_bytes[i] = (unsigned char)i;
  if (memmove(move_bytes + 3, move_bytes, 20) != move_bytes + 3) return 88;
  for (size_t i = 0; i < 20; ++i) if (move_bytes[i + 3] != (unsigned char)i) return 89;
  if (memmove(move_bytes, move_bytes + 3, 20) != move_bytes) return 90;
  for (size_t i = 0; i < 20; ++i) if (move_bytes[i] != (unsigned char)i) return 91;
  if (memset(dst, 0xA5, sizeof dst) != dst) return 6;
  for (size_t i = 0; i < sizeof dst; ++i) {
    if ((unsigned char)dst[i] != 0xA5) return 7;
  }
  if (strcmp("alpha", "alpha") != 0 || strcmp("alpha", "beta") >= 0 || strcmp("beta", "alpha") <= 0) return 8;
  if (strncmp("same-prefix-a", "same-prefix-b", 11) != 0) return 9;
  if (strncmp("same-prefix-a", "same-prefix-b", 13) >= 0) return 10;
  if (strcpy(copied, "copy") != copied || strcmp(copied, "copy") != 0) return 11;
  if (strlcpy(bounded_copy, "abcdef", sizeof bounded_copy) != 6 || strcmp(bounded_copy, "abc") != 0) return 74;
  if (strlcpy(bounded_copy, "ab", sizeof bounded_copy) != 2 || strcmp(bounded_copy, "ab") != 0) return 75;
  if (strlcpy(one_byte_copy, "x", sizeof one_byte_copy) != 1 || one_byte_copy[0] != 0) return 76;
  if (strlcpy(bounded_copy, "ignored", 0) != 7 || strcmp(bounded_copy, "ab") != 0) return 77;
  char collation_key[5] = {'x', 'x', 'x', 'x', 'x'};
  if (strxfrm(collation_key, "abc", sizeof collation_key) != 3 || strcmp(collation_key, "abc") != 0) return 148;
  for (size_t i = 0; i < sizeof collation_key; ++i) collation_key[i] = 'x';
  if (strxfrm(collation_key, "abc", 3) != 3 || collation_key[3] != 'x') return 149;
  collation_key[2] = 'x';
  if (strxfrm(collation_key, "abc", 2) != 3 || collation_key[2] != 'x') return 151;
  collation_key[0] = 'x';
  if (strxfrm(collation_key, "abc", 0) != 3 || collation_key[0] != 'x') return 150;
  if (strlcat(bounded_cat, "cdef", sizeof bounded_cat) != 6 || strcmp(bounded_cat, "abcd") != 0) return 78;
  if (strlcat(full_cat, "cd", sizeof full_cat) != 4 || strcmp(full_cat, "abcd") != 0) return 79;
  if (strlcat(bounded_cat, "!", 0) != 1 || strcmp(bounded_cat, "abcd") != 0) return 80;
  if (strlcat(unterminated_cat, "xy", sizeof unterminated_cat) != 5 || unterminated_cat[0] != 'a' || unterminated_cat[2] != 'c') return 81;
  if (stpcpy(end_copied, "end") != end_copied + 3 || strcmp(end_copied, "end") != 0) return 62;
  if (stpcpy(end_copied, "") != end_copied || end_copied[0] != 0) return 63;
  if (strncpy(padded, "xy", sizeof padded) != padded) return 12;
  if (padded[0] != 'x' || padded[1] != 'y' || padded[2] != 0 || padded[4] != 0) return 13;
  if (stpncpy(padded_end, "xy", 5) != padded_end + 2) return 64;
  if (padded_end[0] != 'x' || padded_end[1] != 'y' || padded_end[2] != 0 || padded_end[4] != 0 || padded_end[5] != 'x') return 65;
  if (stpncpy(exact_end, "abc", 3) != exact_end + 3 || exact_end[2] != 'c') return 66;
  if (stpncpy(exact_end, "abc", 0) != exact_end || exact_end[0] != 'a') return 67;
  if (strchr(src, 'u') != src + 3 || strchr(src, 'z') != 0) return 14;
  if (strrchr(src, 'e') != src + 1 || strrchr(src, 0) != src + 5) return 15;
  if (strcat(joined, "ua") != joined || strcmp(joined, "nelua") != 0) return 17;
  if (strncat(joined, "xyz", 2) != joined || strcmp(joined, "neluaxy") != 0) return 18;
  if (strncat(joined, "ignored", 0) != joined || strcmp(joined, "neluaxy") != 0) return 19;
  if (strcoll("alpha", "beta") >= 0 || strcoll("same", "same") != 0 ||
      strcoll("z", "a") <= 0) return 147;
  if (strstr(phrase, "") != phrase || strstr(phrase, "libc") != phrase + 6) return 20;
  if (strstr(phrase, "runtime") != phrase + 11 || strstr(phrase, "missing") != 0) return 21;
  if (strspn(span_text, "ab") != 6 || strspn(span_text, "") != 0) return 22;
  if (strcspn(span_text, "bc") != 3 || strcspn(span_text, "") != 7) return 23;
  if (strpbrk(span_text, "bc") != span_text + 3 || strpbrk(span_text, "") != 0 ||
      strpbrk("abc", "z") != 0) return 146;
  char *token = strtok(token_text, ",");
  if (token != token_text + 2 || strcmp(token, "alpha") != 0) return 24;
  token = strtok(0, ",");
  if (token == 0 || strcmp(token, "beta") != 0) return 25;
  if (strtok(0, ",") != 0) return 26;
  if (strtok(unsplit, "") != unsplit || strcmp(unsplit, "whole") != 0) return 27;
  if (strtok(0, "") != 0) return 28;
  field = strsep(&field_state, ",");
  if (field != field_text || field[0] != 0 || field_state != field_text + 1) return 68;
  field = strsep(&field_state, ",");
  if (field == 0 || strcmp(field, "alpha") != 0) return 69;
  field = strsep(&field_state, ",");
  if (field == 0 || field[0] != 0) return 70;
  field = strsep(&field_state, ",");
  if (field == 0 || strcmp(field, "beta") != 0) return 71;
  field = strsep(&field_state, ",");
  if (field == 0 || field[0] != 0 || field_state != 0 || strsep(&field_state, ",") != 0) return 72;
  if (strsep(&whole_state, "") != whole_text || whole_state != 0) return 73;
  if (strcmp(strtok_r(comma_text, ",", &comma_state), "a") != 0) return 29;
  if (strcmp(strtok_r(colon_text, ":", &colon_state), "1") != 0) return 30;
  if (strcmp(strtok_r(0, ",", &comma_state), "b") != 0) return 31;
  if (strcmp(strtok_r(0, ":", &colon_state), "2") != 0) return 32;
  if (strtok_r(0, ",", &comma_state) != 0 || strtok_r(0, ":", &colon_state) != 0) return 33;
  char fresh_tokens[] = ",x,,y,";
  char *fresh_state = (char *)1;
  if (strcmp(strtok_r(fresh_tokens, ",", &fresh_state), "x") != 0) return 153;
  if (strcmp(strtok_r(0, ",", &fresh_state), "y") != 0 ||
      strtok_r(0, ",", &fresh_state) != 0) return 154;
  char empty_delim_tokens[] = "whole";
  char *empty_delim_state = 0;
  if (strtok_r(empty_delim_tokens, "", &empty_delim_state) != empty_delim_tokens ||
      strcmp(empty_delim_tokens, "whole") != 0 ||
      strtok_r(0, "", &empty_delim_state) != 0) return 155;
  if (bad_descriptor_message == 0 || strcmp(bad_descriptor_message, "Bad file descriptor") != 0) return 34;
  if (unknown_error_message == 0 || strlen(unknown_error_message) == 0) return 35;
  if (strcmp(strerror(0), "Success") != 0) return 36;
  if (strcmp(strerror(44), "Channel number out of range") != 0) return 37;
  if (strcmp(strerror(84), "Invalid or incomplete multibyte or wide character") != 0) return 38;
  if (strcmp(strerror(122), "Disk quota exceeded") != 0) return 39;
  if (strcmp(strerror(133), "Memory page has hardware error") != 0) return 40;
  if (strcmp(strerror(41), "Unknown error") != 0 || strcmp(strerror(58), "Unknown error") != 0) return 41;
  return 0;
}
