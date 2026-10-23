#define _GNU_SOURCE
#include "string.h"
#include <stdint.h>
#include <stdio.h>

enum { CASES = 8192, MAX_TEXT = 64, MAX_DELIM = 8 };
static const char alphabet[] = "abcXYZ012,;:| \t";
static const char *changing_delimiters[] = {",", ":", "|", ";"};
static uint64_t random_state = UINT64_C(0x4e454c4942435354);

static uint32_t next_random(void) {
  random_state ^= random_state << 13;
  random_state ^= random_state >> 7;
  random_state ^= random_state << 17;
  return (uint32_t)(random_state >> 16);
}

static void print_hex(const char *text, unsigned length) {
  for (unsigned i = 0; i < length; ++i)
    printf("%02x", (unsigned char)text[i]);
}

static void make_case(unsigned number, char text[MAX_TEXT + 1], unsigned *length,
                      char delim[MAX_DELIM + 1]) {
  *length = next_random() % (MAX_TEXT + 1);
  for (unsigned i = 0; i < *length; ++i)
    text[i] = alphabet[next_random() % (sizeof(alphabet) - 1)];
  text[*length] = '\0';

  unsigned mode = number % 3;
  unsigned delim_length = mode == 0 ? 0 : (mode == 1 ? 1 : 2 + next_random() % (MAX_DELIM - 1));
  unsigned used = 0;
  while (used < delim_length) {
    char value = alphabet[next_random() % (sizeof(alphabet) - 1)];
    unsigned i = 0;
    while (i < used && delim[i] != value) ++i;
    if (i == used) delim[used++] = value;
  }
  delim[used] = '\0';
}

static void trace_strtok(unsigned number, const char *original, unsigned length,
                         const char *delim) {
  char text[MAX_TEXT + 1];
  for (unsigned i = 0; i <= length; ++i) text[i] = original[i];
  printf("%u S d", number);
  print_hex(delim, (unsigned)__builtin_strlen(delim));
  printf(" i");
  print_hex(text, length);
  printf(" t");
  char *token = strtok(text, delim);
  unsigned calls = 0;
  while (token != NULL && calls++ <= MAX_TEXT) {
    printf(" %u:", (unsigned)(token - text));
    unsigned token_length = (unsigned)__builtin_strlen(token);
    print_hex(token, token_length);
    token = strtok(NULL, delim);
  }
  if (token != NULL) printf(" LOOP");
  printf(" m");
  print_hex(text, length + 1);
  putchar('\n');
}

static void trace_strtok_r(unsigned number, const char *original, unsigned length,
                           const char *delim) {
  char text[MAX_TEXT + 1];
  for (unsigned i = 0; i <= length; ++i) text[i] = original[i];
  /* The first call must ignore saveptr's previous contents when s is non-null. */
  char *save = number & 1 ? text + length : NULL;
  printf("%u R d", number);
  print_hex(delim, (unsigned)__builtin_strlen(delim));
  printf(" i");
  print_hex(text, length);
  printf(" t");
  char *token = strtok_r(text, delim, &save);
  unsigned calls = 0;
  while (token != NULL && calls++ <= MAX_TEXT) {
    printf(" %u:", (unsigned)(token - text));
    unsigned token_length = (unsigned)__builtin_strlen(token);
    print_hex(token, token_length);
    token = strtok_r(NULL, delim, &save);
  }
  if (token != NULL) printf(" LOOP");
  printf(" m");
  print_hex(text, length + 1);
  putchar('\n');
}

static void trace_changing_strtok(unsigned number, const char *original,
                                  unsigned length) {
  char text[MAX_TEXT + 1];
  for (unsigned i = 0; i <= length; ++i) text[i] = original[i];
  printf("%u C i", number);
  print_hex(text, length);
  printf(" t");
  unsigned calls = 0;
  const char *delim = changing_delimiters[calls % 4];
  char *token = strtok(text, delim);
  while (token != NULL && calls <= MAX_TEXT) {
    printf(" d");
    print_hex(delim, (unsigned)__builtin_strlen(delim));
    printf(" %u:", (unsigned)(token - text));
    print_hex(token, (unsigned)__builtin_strlen(token));
    ++calls;
    delim = changing_delimiters[calls % 4];
    token = strtok(NULL, delim);
  }
  if (token != NULL) printf(" LOOP");
  printf(" m");
  print_hex(text, length + 1);
  putchar('\n');
}

static void trace_changing_strtok_r(unsigned number, const char *original,
                                    unsigned length) {
  char text[MAX_TEXT + 1];
  for (unsigned i = 0; i <= length; ++i) text[i] = original[i];
  char *save = text + length;
  printf("%u Q i", number);
  print_hex(text, length);
  printf(" t");
  unsigned calls = 0;
  const char *delim = changing_delimiters[calls % 4];
  char *token = strtok_r(text, delim, &save);
  while (token != NULL && calls <= MAX_TEXT) {
    printf(" d");
    print_hex(delim, (unsigned)__builtin_strlen(delim));
    printf(" %u:", (unsigned)(token - text));
    print_hex(token, (unsigned)__builtin_strlen(token));
    ++calls;
    delim = changing_delimiters[calls % 4];
    token = strtok_r(NULL, delim, &save);
  }
  if (token != NULL) printf(" LOOP");
  printf(" m");
  print_hex(text, length + 1);
  putchar('\n');
}

int main(void) {
  for (unsigned number = 0; number < CASES; ++number) {
    char original[MAX_TEXT + 1];
    char delim[MAX_DELIM + 1];
    unsigned length;
    make_case(number, original, &length, delim);
    trace_strtok(number, original, length, delim);
    trace_strtok_r(number, original, length, delim);
    if (number % 8 == 0) {
      trace_changing_strtok(number, original, length);
      trace_changing_strtok_r(number, original, length);
    }
  }
  static const char changing_case[] = "a,b:c|d;e";
  trace_changing_strtok(CASES, changing_case, sizeof(changing_case) - 1);
  trace_changing_strtok_r(CASES, changing_case, sizeof(changing_case) - 1);
  return 0;
}
