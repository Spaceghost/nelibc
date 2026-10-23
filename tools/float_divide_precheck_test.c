#include <stdint.h>
#include <stdio.h>

int nelibc_radix_character(void) { return '.'; }

int nelibc_test_big_divide_u64_precheck(uint64_t low, uint32_t high,
                                        uint32_t divisor, int should_succeed);
int nelibc_test_big_divide_capacity_precheck(void);

int main(void) {
  struct test_case {
    uint64_t low;
    uint32_t high;
    uint32_t divisor;
    int should_succeed;
    const char *name;
  } cases[] = {
      {UINT64_MAX, 0, 1, 1, "below 2^64, normalization shift 31"},
      {UINT64_MAX, 0, 3, 1, "below 2^64, normalization shift 30"},
      {UINT64_MAX, 0, UINT32_C(0x80000000), 1, "below 2^64, normalization shift 1"},
      {UINT64_MAX, 0, UINT32_MAX, 1, "below 2^64, no normalization shift"},
      {0, 1, 3, 0, "quotient exactly 2^64"},
      {1, 1, 3, 0, "quotient just above 2^64"},
  };

  for (size_t i = 0; i < sizeof cases / sizeof cases[0]; ++i) {
    int result = nelibc_test_big_divide_u64_precheck(cases[i].low,
                                                       cases[i].high, cases[i].divisor,
                                                       cases[i].should_succeed);
    if (result != 0) {
      fprintf(stderr, "%s failed: probe result %d\n", cases[i].name, result);
      return 1;
    }
  }
  int capacity_result = nelibc_test_big_divide_capacity_precheck();
  if (capacity_result != 0) {
    fprintf(stderr, "normalization capacity boundary failed: probe result %d\n",
            capacity_result);
    return 1;
  }
  puts("BigUInt quotient precheck boundaries passed");
  return 0;
}
