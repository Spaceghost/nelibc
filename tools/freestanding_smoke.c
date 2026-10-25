#include "string.h"

/* memory.nelua also exports wide APIs that are not exercised by this byte and
   string smoke. Keep their C-locale link providers local to this small harness;
   the full standalone archive supplies the Unicode-backed Nelua versions. */
int towlower(int value) {
  return value >= 'A' && value <= 'Z' ? value + ('a' - 'A') : value;
}

int wcwidth(int value) {
  if (value == 0) return 0;
  if (value < 0 || value > 0x7f || value < 0x20 || value == 0x7f) return -1;
  return 1;
}

static unsigned char heap[32];

void *malloc(size_t size) {
  return size <= sizeof heap ? heap : 0;
}

int main(void) {
  if (strcasecmp("NeLiBc", "nElIbC") != 0 || strcasecmp("A", "b") >= 0) return 22;
  if (strncasecmp("NELIBC-tail", "nelibc-head", 6) != 0 ||
      strncasecmp("A", "b", 1) >= 0 || strncasecmp("A", "a", 0) != 0) return 23;
  if (strcasecmp("@", "`") >= 0 || strcasecmp("[", "{") >= 0) return 24;
  const char high_case_left[] = {(char)0xE0, 0};
  const char high_case_right[] = {(char)0xC0, 0};
  if (strcasecmp(high_case_left, high_case_right) != 32 ||
      strncasecmp(high_case_left, high_case_right, 1) != 32) return 25;
  char source[] = "abc";
  char copy[sizeof source] = {0};
  char bounded_copy[4];
  char bounded_cat[5] = "ab";
  char end_copy[8];
  char padded_end[5];
  char overlap[] = "wxyz";
  unsigned char binary[] = {0x10, 0xFF, 0x20};
  unsigned char needle[] = {0xFF, 0x20};
  unsigned char stopped[4] = {0};
  char *duplicate;
  char *bounded_duplicate;
  char fields[] = ",x,,";
  char *field_state = fields;

  duplicate = strdup("free core");
  if (duplicate == 0 || strcmp(duplicate, "free core") != 0) return 12;
  bounded_duplicate = strndup("bounded", 3);
  if (bounded_duplicate == 0 || strcmp(bounded_duplicate, "bou") != 0) return 13;
  if (strlen(source) != 3) return 1;
  if (strlcpy(bounded_copy, "abcdef", sizeof bounded_copy) != 6 || strcmp(bounded_copy, "abc") != 0) return 18;
  if (strlcat(bounded_cat, "cdef", sizeof bounded_cat) != 6 || strcmp(bounded_cat, "abcd") != 0) return 19;
  if (stpcpy(end_copy, "end") != end_copy + 3 || strcmp(end_copy, "end") != 0) return 14;
  if (stpncpy(padded_end, "xy", sizeof padded_end) != padded_end + 2 || padded_end[2] != 0 || padded_end[4] != 0) return 15;
  if (strsep(&field_state, ",") != fields || fields[0] != 0) return 16;
  if (strcmp(strsep(&field_state, ","), "x") != 0 || strsep(&field_state, ",")[0] != 0) return 17;
  if (memcpy(copy, source, sizeof source) != copy) return 2;
  if (memcmp(copy, source, sizeof source) != 0) return 3;
  if (mempcpy(copy, source, sizeof source) != copy + sizeof source) return 20;
  if (memcmp(copy, source, sizeof source) != 0) return 21;
  if (memccpy(stopped, binary, 0xFF, sizeof binary) != stopped + 2) return 11;
  if (memchr(binary, 0xFF, sizeof binary) != binary + 1) return 8;
  if (memrchr(binary, 0xFF, sizeof binary) != binary + 1) return 10;
  if (memmem(binary, sizeof binary, needle, sizeof needle) != binary + 1) return 9;
  if (memmove(overlap + 1, overlap, 3) != overlap + 1) return 4;
  if (overlap[0] != 'w' || overlap[1] != 'w' || overlap[2] != 'x' || overlap[3] != 'y') return 5;
  if (memset(copy, 0, sizeof copy) != copy) return 6;
  if (memcmp(copy, "\0\0\0\0", sizeof copy) != 0) return 7;
  return 0;
}
