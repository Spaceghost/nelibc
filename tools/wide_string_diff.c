#define _GNU_SOURCE
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <wchar.h>

#define CASES 256
#define WIDTH 40
static uint64_t state = UINT64_C(0x243f6a8885a308d3);

static uint64_t random_u64(void) {
  state ^= state << 13;
  state ^= state >> 7;
  state ^= state << 17;
  return state;
}

static int signum(int value) { return (value > 0) - (value < 0); }

static ptrdiff_t pointer_offset(const wchar_t *base, const wchar_t *value) {
  return value == NULL ? -1 : value - base;
}

static void print_wide(const wchar_t *text) {
  while (*text != 0) {
    printf("%x,", (unsigned)*text);
    ++text;
  }
  putchar(';');
}

static void make_string(wchar_t *text, size_t *length) {
  static const wchar_t alphabet[] = {L'A', L'B', L'C', L'D', 0x100, 0x101};
  *length = (size_t)(random_u64() % 15);
  for (size_t i = 0; i < *length; ++i)
    text[i] = alphabet[random_u64() % (sizeof alphabet / sizeof alphabet[0])];
  text[*length] = 0;
}

int main(void) {
  static const wchar_t accept[] = {L'A', L'C', 0x100, 0};
  static const wchar_t reject[] = {L'B', L'D', 0x101, 0};
  static const wchar_t delimiters[] = {L' ', 0x100, 0};
  for (size_t test = 0; test < CASES; ++test) {
    wchar_t left[WIDTH], right[WIDTH], needle[WIDTH];
    size_t left_length, right_length;
    make_string(left, &left_length);
    make_string(right, &right_length);
    size_t needle_length = right_length == 0 ? 0 : (size_t)(random_u64() % (right_length + 1));
    for (size_t i = 0; i < needle_length; ++i) needle[i] = right[i];
    needle[needle_length] = 0;
    wchar_t probe = (random_u64() & 1) ? 0x100 : L'B';
    size_t bound = (size_t)(random_u64() % (left_length + 3));

    printf("%zu:", test);
    print_wide(left);
    print_wide(right);
    print_wide(needle);
    printf("%zu,%zu,%d,%d,%zu,%zu,%td,%td,%td,%td,",
        wcslen(left), wcsnlen(left, bound), signum(wcscmp(left, right)),
        signum(wcsncmp(left, right, bound)), wcsspn(left, accept), wcscspn(left, reject),
        pointer_offset(left, wcschr(left, probe)), pointer_offset(left, wcsrchr(left, probe)),
        pointer_offset(left, wcspbrk(left, reject)), pointer_offset(left, wcsstr(left, needle)));

    printf("%d,%td,", signum(wmemcmp(left, right, left_length < right_length ? left_length + 1 : right_length + 1)),
        pointer_offset(left, wmemchr(left, probe, left_length + 1)));

    wchar_t moved[WIDTH] = {0};
    wmemcpy(moved, left, left_length + 1);
    if (left_length != 0) wmemmove(moved + 1, moved, left_length);
    for (size_t i = 0; i <= left_length; ++i) printf("%x,", (unsigned)moved[i]);
    putchar(';');
    wmemset(moved, probe, 3);
    printf("%x,%x,%x;", (unsigned)moved[0], (unsigned)moved[1], (unsigned)moved[2]);

    wchar_t copied[WIDTH] = {0};
    wcscpy(copied, left);
    print_wide(copied);
    wchar_t padded[WIDTH];
    for (size_t i = 0; i < WIDTH; ++i) padded[i] = 0x777;
    size_t copy_limit = (size_t)(random_u64() % 18);
    wcsncpy(padded, right, copy_limit);
    for (size_t i = 0; i < 18; ++i) printf("%x,", (unsigned)padded[i]);
    putchar(';');

    wchar_t concatenated[WIDTH] = {0};
    wcscpy(concatenated, left);
    wcscat(concatenated, right);
    print_wide(concatenated);
    wchar_t bounded_concat[WIDTH] = {0};
    wcscpy(bounded_concat, left);
    size_t concat_limit = (size_t)(random_u64() % 18);
    wcsncat(bounded_concat, right, concat_limit);
    print_wide(bounded_concat);

    wchar_t duplicate_source[WIDTH];
    wcscpy(duplicate_source, left);
    wchar_t *duplicate = wcsdup(duplicate_source);
    if (duplicate == NULL) return 2;
    print_wide(duplicate);
    free(duplicate);

    wchar_t token_text[WIDTH];
    size_t token_length = 0;
    token_text[token_length++] = L' ';
    token_text[token_length++] = L' ';
    for (size_t i = 0; i < left_length; ++i) token_text[token_length++] = left[i];
    token_text[token_length++] = 0x100;
    token_text[token_length++] = 0x100;
    for (size_t i = 0; i < right_length; ++i) token_text[token_length++] = right[i];
    token_text[token_length++] = L' ';
    token_text[token_length] = 0;
    wchar_t *saveptr = NULL;
    wchar_t *token = wcstok(token_text, delimiters, &saveptr);
    while (token != NULL) {
      print_wide(token);
      token = wcstok(NULL, delimiters, &saveptr);
    }
    putchar('\n');
  }
  printf("%d wide string and memory cases emitted\n", CASES);
  return 0;
}
