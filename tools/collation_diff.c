#define _GNU_SOURCE 1
#include <locale.h>
#include <stdio.h>
#include <string.h>

/* include/string.h intentionally exposes Nelibc's portable API surface;
   these GNU/POSIX locale-reference functions come from the host libc. */
extern int strcoll_l(const char *left, const char *right, locale_t locale);
extern size_t strxfrm_l(char *restrict dest, const char *restrict src,
                        size_t size, locale_t locale);

static const char *const samples[] = {
  "", "A", "a", "aa", "ab", "ae", "b", "ss", "z", "_", "10", "2",
  "\xc3\xa4", "\xc3\x9f", "\xc3\xa5", "\xc3\xb6"
};

static int signum(int value) { return (value > 0) - (value < 0); }

int main(int argc, char **argv) {
  int global_mode = argc > 1 && strcmp(argv[1], "--global") == 0;
  const char *locale_name = global_mode
    ? (argc > 2 ? argv[2] : "C")
    : (argc > 1 ? argv[1] : "C");
  locale_t collate = newlocale(LC_COLLATE_MASK, locale_name, (locale_t)0);
  if (collate == (locale_t)0) {
    fprintf(stderr, "cannot create LC_COLLATE locale '%s'\n", locale_name);
    return 77;
  }
  locale_t previous_locale = (locale_t)0;
  if (global_mode) {
    if (setlocale(LC_COLLATE, locale_name) == (char *)0) {
      fprintf(stderr, "cannot select global LC_COLLATE locale '%s'\n", locale_name);
      freelocale(collate);
      return 77;
    }
  } else {
    previous_locale = uselocale(collate);
    if (previous_locale == (locale_t)0) {
      fprintf(stderr, "cannot select LC_COLLATE locale '%s' for this thread\n", locale_name);
      freelocale(collate);
      return 77;
    }
  }

  unsigned comparisons = 0;
  unsigned mismatches = 0;
  for (unsigned i = 0; i < sizeof samples / sizeof samples[0]; ++i) {
    char actual_key[256];
    char reference_key[256];
    size_t actual_length = strxfrm(actual_key, samples[i], sizeof actual_key);
    size_t reference_length = strxfrm_l(reference_key, samples[i], sizeof reference_key, collate);
    if (actual_length >= sizeof actual_key || reference_length >= sizeof reference_key) {
      fprintf(stderr, "collation key too large for sample %u\n", i);
      ++mismatches;
      continue;
    }
    if (actual_key[actual_length] != '\0' || reference_key[reference_length] != '\0') {
      fprintf(stderr, "invalid collation key termination for sample %u\n", i);
      ++mismatches;
      continue;
    }
    if ((locale_name[0] == 'C' && (locale_name[1] == '\0' || locale_name[1] == '.')) ||
        (locale_name[0] == 'P' && locale_name[1] == 'O')) {
      if (actual_length != strlen(samples[i]) || strcmp(actual_key, samples[i]) != 0) {
        fprintf(stderr, "C-locale strxfrm mismatch for sample %u\n", i);
        ++mismatches;
      }
    }
  }

  for (unsigned i = 0; i < sizeof samples / sizeof samples[0]; ++i) {
    for (unsigned j = 0; j < sizeof samples / sizeof samples[0]; ++j) {
      char left_key[256];
      char right_key[256];
      char reference_left_key[256];
      char reference_right_key[256];
      int actual = signum(strcoll(samples[i], samples[j]));
      int expected = signum(strcoll_l(samples[i], samples[j], collate));
      size_t left_length = strxfrm(left_key, samples[i], sizeof left_key);
      size_t right_length = strxfrm(right_key, samples[j], sizeof right_key);
      size_t reference_left_length = strxfrm_l(reference_left_key, samples[i], sizeof reference_left_key, collate);
      size_t reference_right_length = strxfrm_l(reference_right_key, samples[j], sizeof reference_right_key, collate);
      if (actual != expected) {
        if (mismatches < 8)
          fprintf(stderr, "strcoll(%s, %s): Nelibc=%d host[%s]=%d\n",
                  samples[i], samples[j], actual, locale_name, expected);
        ++mismatches;
      }
      if (left_length >= sizeof left_key || right_length >= sizeof right_key ||
          left_key[left_length] != '\0' || right_key[right_length] != '\0' ||
          signum(strcmp(left_key, right_key)) != actual) {
        if (mismatches < 8)
          fprintf(stderr, "strxfrm ordering mismatch for sample pair %u,%u\n", i, j);
        ++mismatches;
      }
      if (reference_left_length >= sizeof reference_left_key ||
          reference_right_length >= sizeof reference_right_key ||
          signum(strcmp(reference_left_key, reference_right_key)) != expected) {
        if (mismatches < 8)
          fprintf(stderr, "host strxfrm_l ordering mismatch for sample pair %u,%u\n", i, j);
        ++mismatches;
      }
      ++comparisons;
    }
  }

  if (!global_mode) uselocale(previous_locale);
  freelocale(collate);
  if (mismatches != 0) {
    fprintf(stderr, "%u collation mismatches across %u pairs (locale %s)\n",
            mismatches, comparisons, locale_name);
    return 1;
  }
  printf("%u collation pairs passed (locale %s)\n", comparisons, locale_name);
  return 0;
}
