#define _GNU_SOURCE
#include <stdlib.h>

struct item {
  int key;
  unsigned char padding[76];
};

static int compare_items(const void *left, const void *right, void *context) {
  const struct item *a = left;
  const struct item *b = right;
  const int direction = *(const int *)context;
  return ((a->key > b->key) - (a->key < b->key)) * direction;
}

static int compare_ints(const void *left, const void *right, void *context) {
  const int direction = *(const int *)context;
  const int a = *(const int *)left;
  const int b = *(const int *)right;
  return ((a > b) - (a < b)) * direction;
}

static int run(int direction) {
  struct item values[] = {{7}, {-3}, {7}, {0}, {42}, {-3}, {1}};
  qsort_r(values, sizeof(values) / sizeof(values[0]), sizeof(values[0]),
          compare_items, &direction);
  for (size_t i = 1; i < sizeof(values) / sizeof(values[0]); ++i) {
    if (direction > 0 && values[i - 1].key > values[i].key) return 1;
    if (direction < 0 && values[i - 1].key < values[i].key) return 1;
  }
  return 0;
}

int main(void) {
  int direction = -1;
  int small[] = {4, -2, 9, 0, 4};
  qsort_r(small, sizeof(small) / sizeof(small[0]), sizeof(small[0]),
          compare_ints, &direction);
  for (size_t i = 1; i < sizeof(small) / sizeof(small[0]); ++i)
    if (small[i - 1] < small[i]) return 3;
  return run(1) || run(-1);
}
