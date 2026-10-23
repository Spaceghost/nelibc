#define _GNU_SOURCE
#include "string.h"
#include <locale.h>
#include <stdint.h>
#include <stdio.h>
#include <sys/mman.h>
#include <time.h>
#include <unistd.h>

#ifndef BENCH_COMPILER
#define BENCH_COMPILER "unspecified"
#endif
#ifndef BENCH_LIBC
#define BENCH_LIBC "unspecified"
#endif
#ifndef BENCH_IMPLEMENTATION
#define BENCH_IMPLEMENTATION "host-libc"
#endif

#define SAMPLES 7
#define ITERATIONS 20000

typedef struct { const char *name, *left, *right; size_t n; } input_case;
static const char long_equal_left[] = "Nelua-String-with-a-long-shared-prefix-Nelua-String-with-a-long-shared-prefix-Nelua-String-tail";
static const char long_equal_right[] = "nELUA-sTRING-WITH-A-LONG-SHARED-PREFIX-nELUA-sTRING-WITH-A-LONG-SHARED-PREFIX-nELUA-sTRING-TAIL";
static const char long_late_mismatch_right[] = "nELUA-sTRING-WITH-A-LONG-SHARED-PREFIX-nELUA-sTRING-WITH-A-LONG-SHARED-PREFIX-nELUA-sTRING-TAIx";
static const input_case cases[] = {
  {"equal-short", "Nelua-String", "nELUA-sTRING", 0},
  {"mismatch-first", "Xelua-String", "nelua-String", 0},
  {"mismatch-late", "nelua-string-with-a-long-shared-prefix", "NELUA-STRING-WITH-A-LONG-SHARED-PREfix", 0},
  {"equal-bounded", "Nelua-String-tail", "nELUA-sTRING-head", 12},
  {"mismatch-at-bound", "Nelua-String-tail", "nELUA-sTRING-head", 14},
  {"mismatch-first-bounded", "Xelua-String-with-a-long-tail", "nelua-String-with-a-long-tail", 32},
  {"equal-long-bounded", "Nelua-String-with-a-long-tail", "nELUA-sTRING-WITH-A-LONG-TAIL", 32},
  {"mismatch-late-bounded", "Nelua-String-with-a-long-tail", "nELUA-sTRING-WITH-A-LONG-XAIL", 32},
  {"equal-very-long", long_equal_left, long_equal_right, 0},
  {"mismatch-very-late", long_equal_left, long_late_mismatch_right, 0},
  {"equal-256", "", "", 0},
  {"mismatch-late-256", "", "", 0}
};
#define CASE_COUNT (sizeof cases / sizeof cases[0])
static input_case benchmark_cases[CASE_COUNT];
static char benchmark_strings[CASE_COUNT][2][512] __attribute__((aligned(4096)));
static volatile unsigned long long sink;

static void prepare_benchmark_cases(void) {
  for (size_t i = 0; i < CASE_COUNT; ++i) {
    benchmark_cases[i] = cases[i];
    char *destinations[2] = {benchmark_strings[i][0], benchmark_strings[i][1]};
    const char *sources[2] = {cases[i].left, cases[i].right};
    for (size_t side = 0; side < 2; ++side) {
      size_t j = 0;
      do { destinations[side][j] = sources[side][j]; } while (sources[side][j++] != '\0');
    }
    benchmark_cases[i].left = benchmark_strings[i][0];
    benchmark_cases[i].right = benchmark_strings[i][1];
    if (i >= CASE_COUNT - 2) {
      for (size_t j = 0; j < 256; ++j) {
        benchmark_strings[i][0][j] = (char)('A' + j % 26);
        benchmark_strings[i][1][j] = (char)('a' + j % 26);
      }
      if (i == CASE_COUNT - 1) benchmark_strings[i][1][255] = '!';
      benchmark_strings[i][0][256] = '\0';
      benchmark_strings[i][1][256] = '\0';
    }
  }
}

#ifdef BENCH_NELIBC
int nelibc_strcasecmp(const char *, const char *);
int nelibc_strncasecmp(const char *, const char *, size_t);
static int call_strcasecmp(const char *a, const char *b) { return nelibc_strcasecmp(a, b); }
static int call_strncasecmp(const char *a, const char *b, size_t n) { return nelibc_strncasecmp(a, b, n); }
static int bench_case(const input_case *item) { return item->n ? nelibc_strncasecmp(item->left, item->right, item->n) : nelibc_strcasecmp(item->left, item->right); }
#else
static int call_strcasecmp(const char *a, const char *b) { return strcasecmp(a, b); }
static int call_strncasecmp(const char *a, const char *b, size_t n) { return strncasecmp(a, b, n); }
static int bench_case(const input_case *item) { return item->n ? strncasecmp(item->left, item->right, item->n) : strcasecmp(item->left, item->right); }
#endif

static int long_case_checks(void) {
  size_t equal_95 = CASE_COUNT - 4;
  size_t mismatch_95 = CASE_COUNT - 3;
  size_t equal_256 = CASE_COUNT - 2;
  size_t mismatch_256 = CASE_COUNT - 1;
  if (call_strcasecmp(benchmark_cases[equal_95].left,
                      benchmark_cases[equal_95].right) != 0 ||
      call_strcasecmp(benchmark_cases[mismatch_95].left,
                      benchmark_cases[mismatch_95].right) == 0 ||
      call_strcasecmp(benchmark_cases[equal_256].left,
                      benchmark_cases[equal_256].right) != 0 ||
      call_strcasecmp(benchmark_cases[mismatch_256].left,
                      benchmark_cases[mismatch_256].right) == 0)
    return 1;
  return 0;
}

static int page_boundary_checks(void) {
  long page_size_result = sysconf(_SC_PAGESIZE);
  if (page_size_result <= 0) return 1;
  size_t page_size = (size_t)page_size_result;
  unsigned char *mapping = mmap(NULL, page_size * 5, PROT_NONE,
                                MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
  if (mapping == MAP_FAILED) return 1;
  unsigned char *left_page = mapping + page_size;
  unsigned char *right_page = mapping + page_size * 3;
  if (mprotect(left_page, page_size, PROT_READ | PROT_WRITE) != 0 ||
      mprotect(right_page, page_size, PROT_READ | PROT_WRITE) != 0) {
    munmap(mapping, page_size * 5);
    return 1;
  }

  for (size_t length = 0; length <= 64; ++length) {
    char *left = (char *)(left_page + page_size - length - 1);
    char *right = (char *)(right_page + page_size - length - 1);
    for (size_t i = 0; i < length; ++i) {
      left[i] = (char)('A' + i % 26);
      right[i] = (char)('a' + i % 26);
    }
    left[length] = 0;
    right[length] = 0;
    if (call_strcasecmp(left, right) != 0 ||
        call_strncasecmp(left, right, length + 1) != 0) {
      munmap(mapping, page_size * 5);
      return 2;
    }
    if (length != 0) {
      right[length - 1] = '!';
      if (call_strncasecmp(left, right, length - 1) != 0 ||
          call_strncasecmp(left, right, length) <= 0 ||
          call_strcasecmp(left, right) <= 0) {
        munmap(mapping, page_size * 5);
        return 3;
      }
    }
  }
  munmap(mapping, page_size * 5);
  return 0;
}

static int result_sign(int result) {
  return (result > 0) - (result < 0);
}

static unsigned char prefix_pattern_byte(unsigned int pattern, size_t index) {
  static const char mixed[] = "NeLiBc-AbCd-EfGh-XyZz";
  static const char punctuation[] = "@A[]{}-._:;!b?Z";
  static const char mixed_high[] = "@N\xE1\x82" "e-\xFE[A\x90]z!";
  switch (pattern) {
    case 0: return (unsigned char)('a' + index % 26);
    case 1: return (unsigned char)('A' + index % 26);
    case 2: return (unsigned char)(index & 1 ? 'B' : 'a');
    case 3: return (unsigned char)mixed[index % (sizeof mixed - 1)];
    case 4: return (unsigned char)punctuation[index % (sizeof punctuation - 1)];
    case 5: return (unsigned char)(0x80u + index % 128);
    default: return (unsigned char)mixed_high[index % (sizeof mixed_high - 1)];
  }
}

static int exhaustive_prefix_dump(void) {
  if (setlocale(LC_CTYPE, "C") == NULL) return 1;
  static const size_t positions[] = {0, 1, 7, 15, 16, 31, 63, 127};
  char left[160], right[160];
  puts("pattern,position,left_byte,right_byte,strcasecmp,before,inclusive,after");
  for (unsigned int pattern = 0; pattern < 7; ++pattern) {
    for (size_t p = 0; p < sizeof positions / sizeof positions[0]; ++p) {
      size_t position = positions[p];
      for (size_t i = 0; i < sizeof left; ++i)
        left[i] = right[i] = i == sizeof left - 1 ? 0 : prefix_pattern_byte(pattern, i);
      for (unsigned int x = 0; x <= 255; ++x) {
        for (unsigned int y = 0; y <= 255; ++y) {
          left[position] = (char)x;
          right[position] = (char)y;
          printf("%u,%zu,%u,%u,%d,%d,%d,%d\n", pattern, position, x, y,
                 result_sign(call_strcasecmp(left, right)),
                 result_sign(call_strncasecmp(left, right, position)),
                 result_sign(call_strncasecmp(left, right, position + 1)),
                 result_sign(call_strncasecmp(left, right, position + 2)));
        }
      }
    }
  }
  return 0;
}

static uint32_t random_state = 0x6D2B79F5u;

static uint32_t next_random(void) {
  random_state ^= random_state << 13;
  random_state ^= random_state >> 17;
  random_state ^= random_state << 5;
  return random_state;
}

static unsigned char random_nonzero_byte(void) {
  return (unsigned char)(1u + next_random() % 255u);
}

static unsigned char ascii_fold(unsigned char value) {
  return value >= 'A' && value <= 'Z' ? (unsigned char)(value + ('a' - 'A')) : value;
}

static unsigned char different_folded_byte(unsigned char original) {
  unsigned char replacement;
  do replacement = random_nonzero_byte();
  while (ascii_fold(replacement) == ascii_fold(original));
  return replacement;
}

static int randomized_page_edge_dump(unsigned int first_case) {
  long page_size_result = sysconf(_SC_PAGESIZE);
  if (page_size_result <= 0) return 1;
  size_t page_size = (size_t)page_size_result;
  unsigned char *mapping = mmap(NULL, page_size * 5, PROT_NONE,
                                MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
  if (mapping == MAP_FAILED) return 1;
  unsigned char *left_page = mapping + page_size;
  unsigned char *right_page = mapping + page_size * 3;
  if (mprotect(left_page, page_size, PROT_READ | PROT_WRITE) != 0 ||
      mprotect(right_page, page_size, PROT_READ | PROT_WRITE) != 0) {
    munmap(mapping, page_size * 5);
    return 1;
  }

  for (unsigned int case_index = 0; case_index < 10000; ++case_index) {
    unsigned int mode = next_random() % 4u;
    size_t left_term_backoff = next_random() % 16u;
    size_t right_term_backoff = next_random() % 16u;
    size_t left_length, right_length, mismatch_position;
    if (mode == 0) {
      left_length = right_length = next_random() % 513u;
      mismatch_position = left_length;
    } else if (mode == 1) {
      left_length = right_length = 1u + next_random() % 512u;
      mismatch_position = next_random() % left_length;
    } else {
      size_t short_length = next_random() % 512u;
      size_t long_length = short_length + 1u + next_random() % (512u - short_length);
      left_length = mode == 2 ? short_length : long_length;
      right_length = mode == 2 ? long_length : short_length;
      mismatch_position = short_length;
      if (mode == 2) left_term_backoff = 0;
      else right_term_backoff = 0;
    }

    unsigned char *left_terminator = left_page + page_size - left_term_backoff - 1;
    unsigned char *right_terminator = right_page + page_size - right_term_backoff - 1;
    char *left = (char *)(left_terminator - left_length);
    char *right = (char *)(right_terminator - right_length);
    for (size_t i = 0; i < mismatch_position; ++i)
      left[i] = right[i] = (char)random_nonzero_byte();
    if (mode == 0) {
      for (size_t i = mismatch_position; i < left_length; ++i)
        left[i] = right[i] = (char)random_nonzero_byte();
    } else if (mode == 1) {
      unsigned char original = random_nonzero_byte();
      left[mismatch_position] = (char)original;
      right[mismatch_position] = (char)different_folded_byte(original);
      for (size_t i = mismatch_position + 1; i < left_length; ++i)
        left[i] = right[i] = (char)random_nonzero_byte();
    } else if (mode == 2) {
      for (size_t i = mismatch_position; i < right_length; ++i)
        right[i] = (char)random_nonzero_byte();
    } else {
      for (size_t i = mismatch_position; i < left_length; ++i)
        left[i] = (char)random_nonzero_byte();
    }
    left[left_length] = 0;
    right[right_length] = 0;
    for (size_t i = left_length + 1; i <= left_length + left_term_backoff; ++i)
      left[i] = (char)random_nonzero_byte();
    for (size_t i = right_length + 1; i <= right_length + right_term_backoff; ++i)
      right[i] = (char)random_nonzero_byte();

    size_t maximum_length = left_length > right_length ? left_length : right_length;
    unsigned int bound_mode = next_random() % 4u;
    size_t bound;
    if (bound_mode == 0) bound = mismatch_position;
    else if (bound_mode == 1) bound = mismatch_position + 1;
    else if (bound_mode == 2) bound = mismatch_position + 2;
    else bound = next_random() % (maximum_length + 3);
    printf("page-edge,%u,%zu,%zu,%zu,%zu,%d,%d,%u,%u,%d,%d,%d,%d,%d,%d,%d,%d\n", first_case + case_index,
           left_length, right_length, mismatch_position, bound,
           (int)left_term_backoff, (int)right_term_backoff,
           (unsigned char)left[mismatch_position],
           (unsigned char)right[mismatch_position],
           result_sign(call_strncasecmp(left, right, left_length)),
           result_sign(call_strncasecmp(left, right, left_length + 1)),
           result_sign(call_strncasecmp(left, right, left_length + 2)),
           result_sign(call_strncasecmp(left, right, right_length)),
           result_sign(call_strncasecmp(left, right, right_length + 1)),
           result_sign(call_strncasecmp(left, right, right_length + 2)),
           result_sign(call_strcasecmp(left, right)),
           result_sign(call_strncasecmp(left, right, bound)));
  }
  munmap(mapping, page_size * 5);
  return 0;
}

static int randomized_alias_dump(unsigned int first_case) {
  char storage[1025];
  for (unsigned int case_index = 0; case_index < 10000; ++case_index) {
    size_t length = 1u + next_random() % (sizeof storage - 1);
    for (size_t i = 0; i < length; ++i)
      storage[i] = (char)random_nonzero_byte();
    storage[length] = 0;

    size_t same_offset = next_random() % (length + 1);
    const char *same = storage + same_offset;
    size_t same_length = length - same_offset;
    size_t same_bound = next_random() % (same_length + 3);
    printf("same-pointer,%u,%zu,%zu,-1,%zu,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,%d,%d\n",
           first_case + case_index * 2, same_length, same_length, same_bound,
           result_sign(call_strcasecmp(same, same)),
           result_sign(call_strncasecmp(same, same, same_bound)));

    size_t left_offset = next_random() % (length + 1);
    size_t right_offset = next_random() % (length + 1);
    if (length != 0 && right_offset == left_offset)
      right_offset = (right_offset + 1) % (length + 1);
    const char *left = storage + left_offset;
    const char *right = storage + right_offset;
    size_t left_length = length - left_offset;
    size_t right_length = length - right_offset;
    size_t maximum_length = left_length > right_length ? left_length : right_length;
    size_t bound = next_random() % (maximum_length + 3);
    printf("overlap,%u,%zu,%zu,-1,%zu,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,%d,%d\n",
           first_case + case_index * 2 + 1, left_length, right_length, bound,
           result_sign(call_strcasecmp(left, right)),
           result_sign(call_strncasecmp(left, right, bound)));
  }
  return 0;
}

static int randomized_page_edge_alias_dump(unsigned int first_case) {
  long page_size_result = sysconf(_SC_PAGESIZE);
  if (page_size_result <= 0) return 1;
  size_t page_size = (size_t)page_size_result;
  unsigned char *mapping = mmap(NULL, page_size * 3, PROT_NONE,
                                MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
  if (mapping == MAP_FAILED) return 1;
  unsigned char *page = mapping + page_size;
  if (mprotect(page, page_size, PROT_READ | PROT_WRITE) != 0) {
    munmap(mapping, page_size * 3);
    return 1;
  }

  for (unsigned int case_index = 0; case_index < 10000; ++case_index) {
    size_t length = 4u + next_random() % 509u;
    size_t terminator_backoff = next_random() % 16u;
    unsigned char *terminator = page + page_size - terminator_backoff - 1;
    char *start = (char *)(terminator - length);
    for (size_t i = 0; i < length; ++i)
      start[i] = (char)random_nonzero_byte();
    start[length] = 0;
    for (size_t i = length + 1; i <= length + terminator_backoff; ++i)
      start[i] = (char)random_nonzero_byte();

    size_t same_offset = next_random() % (length + 1);
    const char *same = start + same_offset;
    size_t same_length = length - same_offset;
    size_t same_bound = next_random() % (same_length + 3);
    printf("same-page-edge,%u,%zu,%zu,-1,%zu,%d,%d,-1,-1,%d,%d,%d,%d,%d,%d,%d,%d\n",
           first_case + case_index * 2, same_length, same_length, same_bound,
           (int)terminator_backoff, (int)terminator_backoff,
           result_sign(call_strncasecmp(same, same, same_length)),
           result_sign(call_strncasecmp(same, same, same_length + 1)),
           result_sign(call_strncasecmp(same, same, same_length + 2)),
           result_sign(call_strncasecmp(same, same, same_length)),
           result_sign(call_strncasecmp(same, same, same_length + 1)),
           result_sign(call_strncasecmp(same, same, same_length + 2)),
           result_sign(call_strcasecmp(same, same)),
           result_sign(call_strncasecmp(same, same, same_bound)));

    size_t shift = 2u + next_random() % (length - 3u);
    size_t base_offset = next_random() % (length - shift - 1u);
    size_t left_offset = base_offset;
    size_t right_offset = base_offset + shift;
    if (case_index & 1u) {
      size_t temporary = left_offset;
      left_offset = right_offset;
      right_offset = temporary;
    }
    const char *left = start + left_offset;
    const char *right = start + right_offset;
    size_t left_length = length - left_offset;
    size_t right_length = length - right_offset;
    unsigned int mismatch_position;
    if (case_index & 2u) {
      start[left_offset] = 'A';
      start[right_offset] = 'a';
      start[left_offset + 1] = 'B';
      start[right_offset + 1] = 'c';
      mismatch_position = 1;
    } else {
      unsigned char original = (unsigned char)start[left_offset];
      start[right_offset] = (char)different_folded_byte(original);
      mismatch_position = 0;
    }
    size_t maximum_length = left_length > right_length ? left_length : right_length;
    unsigned int bound_mode = next_random() % 4u;
    size_t bound = bound_mode == 0 ? mismatch_position :
                   bound_mode == 1 ? mismatch_position + 1 :
                   bound_mode == 2 ? mismatch_position + 2 :
                   next_random() % (maximum_length + 3);
    printf("overlap-page-edge,%u,%zu,%zu,%u,%zu,%d,%d,%u,%u,%d,%d,%d,%d,%d,%d,%d,%d\n",
           first_case + case_index * 2 + 1, left_length, right_length,
           mismatch_position, bound,
           (int)terminator_backoff, (int)terminator_backoff,
           (unsigned char)left[mismatch_position],
           (unsigned char)right[mismatch_position],
           result_sign(call_strncasecmp(left, right, left_length)),
           result_sign(call_strncasecmp(left, right, left_length + 1)),
           result_sign(call_strncasecmp(left, right, left_length + 2)),
           result_sign(call_strncasecmp(left, right, right_length)),
           result_sign(call_strncasecmp(left, right, right_length + 1)),
           result_sign(call_strncasecmp(left, right, right_length + 2)),
           result_sign(call_strcasecmp(left, right)),
           result_sign(call_strncasecmp(left, right, bound)));
  }
  munmap(mapping, page_size * 3);
  return 0;
}

static int randomized_string_dump(void) {
  if (setlocale(LC_CTYPE, "C") == NULL) return 1;
  enum { RANDOM_CASES = 50000, MAX_STRING_LENGTH = 1280 };
  char left[MAX_STRING_LENGTH + 1], right[MAX_STRING_LENGTH + 1];
  puts("case_type,case,left_length,right_length,mismatch_position,bound,left_term_backoff,right_term_backoff,left_byte_at_compare,right_byte_at_compare,left_before,left_at,left_after,right_before,right_at,right_after,strcasecmp,strncasecmp");
  for (unsigned int case_index = 0; case_index < RANDOM_CASES; ++case_index) {
    unsigned int mode = next_random() % 4u;
    size_t left_length, right_length, mismatch_position;
    if (mode == 0) {
      left_length = next_random() % 1025u;
      right_length = left_length;
      for (size_t i = 0; i < left_length; ++i)
        left[i] = right[i] = (char)random_nonzero_byte();
      mismatch_position = left_length;
    } else if (mode == 1) {
      left_length = 1u + next_random() % 1024u;
      right_length = left_length;
      for (size_t i = 0; i < left_length; ++i)
        left[i] = right[i] = (char)random_nonzero_byte();
      mismatch_position = next_random() % left_length;
      unsigned char original = (unsigned char)left[mismatch_position];
      right[mismatch_position] = (char)different_folded_byte(original);
    } else {
      size_t shared_length = next_random() % 1025u;
      size_t tail_length = 1u + next_random() % 256u;
      for (size_t i = 0; i < shared_length; ++i)
        left[i] = right[i] = (char)random_nonzero_byte();
      if (mode == 2) {
        left_length = shared_length;
        right_length = shared_length + tail_length;
        for (size_t i = shared_length; i < right_length; ++i)
          right[i] = (char)random_nonzero_byte();
      } else {
        right_length = shared_length;
        left_length = shared_length + tail_length;
        for (size_t i = shared_length; i < left_length; ++i)
          left[i] = (char)random_nonzero_byte();
      }
      mismatch_position = shared_length;
    }
    left[left_length] = 0;
    right[right_length] = 0;

    size_t maximum_length = left_length > right_length ? left_length : right_length;
    unsigned int bound_mode = next_random() % 4u;
    size_t bound;
    if (bound_mode == 0) bound = mismatch_position;
    else if (bound_mode == 1) bound = mismatch_position + 1;
    else if (bound_mode == 2) bound = mismatch_position + 2;
    else bound = next_random() % (maximum_length + 3);

    printf("random,%u,%zu,%zu,%zu,%zu,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,%d,%d\n", case_index, left_length,
           right_length, mismatch_position, bound,
           result_sign(call_strcasecmp(left, right)),
           result_sign(call_strncasecmp(left, right, bound)));
  }
  int page_edge_result = randomized_page_edge_dump(RANDOM_CASES);
  if (page_edge_result != 0) return page_edge_result;
  int alias_result = randomized_alias_dump(RANDOM_CASES + 10000);
  if (alias_result != 0) return alias_result;
  return randomized_page_edge_alias_dump(RANDOM_CASES + 30000);
}

static double now_seconds(void) {
  struct timespec ts;
  if (clock_gettime(CLOCK_MONOTONIC, &ts) != 0) return 0.0;
  return (double)ts.tv_sec + (double)ts.tv_nsec / 1e9;
}

static double median_ns(size_t case_index) {
  double samples[SAMPLES];
  for (size_t sample = 0; sample < SAMPLES; ++sample) {
    unsigned long long local = 0;
    double start = now_seconds();
    for (size_t i = 0; i < ITERATIONS; ++i)
      local += (unsigned int)bench_case(&benchmark_cases[case_index]);
    samples[sample] = (now_seconds() - start) * 1e9 / ITERATIONS;
    sink ^= local;
  }
  for (size_t i = 1; i < SAMPLES; ++i) {
    double value = samples[i];
    size_t j = i;
    while (j && samples[j - 1] > value) { samples[j] = samples[j - 1]; --j; }
    samples[j] = value;
  }
  return samples[SAMPLES / 2];
}

int main(int argc, char **argv) {
  if (argc == 2 && argv[1][0] == '-' && argv[1][1] == '-' && argv[1][2] == 'r')
    return randomized_string_dump();
  if (argc == 2 && argv[1][0] == '-' && argv[1][1] == '-')
    return exhaustive_prefix_dump();
  if (page_boundary_checks() != 0) return 2;
  prepare_benchmark_cases();
  if (long_case_checks() != 0) return 3;
  puts("compiler,libc,implementation,function,case,iterations,median_ns_per_call");
  for (size_t i = 0; i < sizeof cases / sizeof cases[0]; ++i)
    printf("%s,%s,%s,%s,%s,%zu,%.2f\n", BENCH_COMPILER, BENCH_LIBC,
           BENCH_IMPLEMENTATION, cases[i].n ? "strncasecmp" : "strcasecmp",
           cases[i].name, (size_t)ITERATIONS, median_ns(i));
  return sink == 1;
}
