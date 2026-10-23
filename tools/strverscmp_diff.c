#define _GNU_SOURCE
#include "string.h"
#include <stdint.h>
#include <stdio.h>

enum { CASES = 16384, MAX_TEXT = 64 };
static uint64_t random_state = UINT64_C(0x5354525645525343);
static const char prefix_alphabet[] = "abcxyz.-_";
static const char suffix_alphabet[] = "abcxyz.-_0123456789";

static uint32_t next_random(void) {
  random_state ^= random_state << 13;
  random_state ^= random_state >> 7;
  random_state ^= random_state << 17;
  return (uint32_t)(random_state >> 16);
}

static void print_hex(const char *text) {
  while (*text != '\0') printf("%02x", (unsigned char)*text++);
}

static unsigned random_digits(char *text, unsigned max_length, int first_zero) {
  unsigned length = 1 + next_random() % max_length;
  text[0] = first_zero ? '0' : (char)('1' + next_random() % 9);
  for (unsigned i = 1; i < length; ++i)
    text[i] = (char)('0' + next_random() % 10);
  return length;
}

static void make_pair(unsigned number, char left[MAX_TEXT + 1],
                      char right[MAX_TEXT + 1]) {
  unsigned prefix_length = next_random() % 13;
  unsigned left_length = 0;
  unsigned right_length = 0;
  for (unsigned i = 0; i < prefix_length; ++i) {
    char value = prefix_alphabet[next_random() % (sizeof(prefix_alphabet) - 1)];
    left[left_length++] = value;
    right[right_length++] = value;
  }

  unsigned mode = number & 3;
  unsigned left_digits = random_digits(left + left_length, 18, mode == 1 || mode == 2);
  unsigned right_digits = random_digits(right + right_length, 18, mode == 1 || mode == 3);
  left_length += left_digits;
  right_length += right_digits;

  unsigned left_suffix = next_random() % 9;
  unsigned right_suffix = next_random() % 9;
  for (unsigned i = 0; i < left_suffix; ++i)
    left[left_length++] = suffix_alphabet[next_random() % (sizeof(suffix_alphabet) - 1)];
  for (unsigned i = 0; i < right_suffix; ++i)
    right[right_length++] = suffix_alphabet[next_random() % (sizeof(suffix_alphabet) - 1)];
  left[left_length] = '\0';
  right[right_length] = '\0';
  if (number % 16 == 0) {
    right_length = left_length;
    for (unsigned i = 0; i <= left_length; ++i) right[i] = left[i];
  }
}

int main(void) {
  for (unsigned number = 0; number < CASES; ++number) {
    char left[MAX_TEXT + 1];
    char right[MAX_TEXT + 1];
    make_pair(number, left, right);
    int result = strverscmp(left, right);
    int sign = (result > 0) - (result < 0);
    printf("%u ", number);
    print_hex(left);
    putchar(':');
    print_hex(right);
    printf(":%d\n", sign);
  }
  return 0;
}
