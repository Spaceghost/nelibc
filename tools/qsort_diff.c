#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

#define CASES 256
#define MAX_ITEMS 4096

#ifdef NELIBC_QSORT_DIAGNOSTICS
extern unsigned int nelibc_test_qsort_depth_fallbacks(void);
#endif

static const size_t boundary_counts[] = {
  0, 1, 2, 7, 15, 16, 17, 31, 255, 256, 257, 511, 1023, 1024, 1025,
  2047, 2048, 2049, 4095, 4096
};

struct item {
  int key;
  unsigned int tag;
};

static int compare_ascending(const void *left, const void *right) {
  int a = ((const struct item *)left)->key;
  int b = ((const struct item *)right)->key;
  return (a > b) - (a < b);
}

static int compare_descending(const void *left, const void *right) {
  return compare_ascending(right, left);
}

static uint32_t next_random(uint32_t *state) {
  *state = *state * UINT32_C(1664525) + UINT32_C(1013904223);
  return *state;
}

static int exercise(size_t count, uint32_t *state, int descending,
                    unsigned int case_number, unsigned int pattern) {
  struct item items[MAX_ITEMS];
  for (size_t i = 0; i < count; ++i) {
    if (pattern == 0) {
      items[i].key = (int)(next_random(state) % 65u) - 32;
    } else if (pattern == 1) {
      items[i].key = (int)i;
    } else if (pattern == 2) {
      items[i].key = (int)(count - i);
    } else if (pattern == 3) {
      items[i].key = 7;
    } else if (pattern == 4) {
      size_t midpoint = count / 2;
      items[i].key = (int)(i <= midpoint ? i : count - i);
    } else {
      items[i].key = (int)(i % 2 == 0 ? i / 2 : count - i / 2);
    }
    items[i].tag = (unsigned int)i;
  }

  int (*compare)(const void *, const void *) =
      descending ? compare_descending : compare_ascending;
  qsort(items, count, sizeof items[0], compare);

  for (size_t i = 1; i < count; ++i) {
    if (compare(&items[i - 1], &items[i]) > 0) {
      fprintf(stderr, "qsort order mismatch case=%u index=%zu\n", case_number, i);
      return 1;
    }
  }

  for (int key = -33; key <= 33; ++key) {
    struct item query = {key, 0};
    struct item *found = bsearch(&query, items, count, sizeof items[0], compare);
    int present = 0;
    for (size_t i = 0; i < count; ++i) present |= items[i].key == key;
    if ((found != NULL) != present || (found != NULL && found->key != key)) {
      fprintf(stderr, "bsearch mismatch case=%u key=%d\n", case_number, key);
      return 1;
    }
  }

  printf("%u,%zu,%d", case_number, count, descending);
  for (size_t i = 0; i < count; ++i) printf(",%d", items[i].key);
  putchar('\n');
  return 0;
}

int main(void) {
  uint32_t state = UINT32_C(0x4e454c55);
  for (unsigned int test = 0; test < CASES; ++test) {
    size_t count = test < sizeof boundary_counts / sizeof boundary_counts[0]
                       ? boundary_counts[test]
                       : next_random(&state) % (MAX_ITEMS + 1u);
    if (exercise(count, &state, 0, test * 2u, 0) ||
        exercise(count, &state, 1, test * 2u + 1u, 0)) return 1;
  }

  static const size_t adversarial_counts[] = {17, 256, 1024, 4096};
  unsigned int case_number = CASES * 2u;
  for (unsigned int pattern = 1; pattern <= 5; ++pattern) {
    for (size_t size_index = 0;
         size_index < sizeof adversarial_counts / sizeof adversarial_counts[0];
         ++size_index) {
      size_t count = adversarial_counts[size_index];
#ifdef NELIBC_QSORT_DIAGNOSTICS
      unsigned int before = nelibc_test_qsort_depth_fallbacks();
#endif
      if (exercise(count, &state, 0, case_number++, pattern) ||
          exercise(count, &state, 1, case_number++, pattern)) return 1;
#ifdef NELIBC_QSORT_DIAGNOSTICS
      unsigned int fallbacks = nelibc_test_qsort_depth_fallbacks() - before;
      if (count == 1024 && pattern == 4 && fallbacks == 0) {
        fprintf(stderr, "qsort depth fallback not reached for organ-pipe input\n");
        return 1;
      }
#endif
    }
  }
  return 0;
}
