#include "nelibc_stdlib.h"
#include <stdint.h>
#include <stdio.h>

static unsigned int reference_rand_r(unsigned int *seed) {
  uint64_t state = (uint64_t)*seed + UINT64_C(0x9e3779b97f4a7c15);
  uint64_t value = state;
  value = (value ^ (value >> 30)) * UINT64_C(0xbf58476d1ce4e5b9);
  value = (value ^ (value >> 27)) * UINT64_C(0x94d049bb133111eb);
  value ^= value >> 31;
  *seed = (unsigned int)state;
  return (unsigned int)(value >> 33);
}

int main(void) {
  static const unsigned int initial_seeds[] = {0U, 1U, 0x12345678U, 0xffffffffU};
  for (size_t seed_index = 0;
       seed_index < sizeof initial_seeds / sizeof initial_seeds[0];
       ++seed_index) {
    unsigned int actual_seed = initial_seeds[seed_index];
    unsigned int reference_seed = initial_seeds[seed_index];
    for (size_t call = 0; call < 4096; ++call) {
      unsigned int expected = reference_rand_r(&reference_seed);
      int actual = rand_r(&actual_seed);
      if (actual < 0 || (unsigned int)actual != expected ||
          actual_seed != reference_seed) {
        fprintf(stderr, "rand_r mismatch: seed-index=%zu call=%zu actual=%d/%u expected=%u/%u\n",
                seed_index, call, actual, actual_seed, expected, reference_seed);
        return 1;
      }
    }
  }
  puts("rand_r matches the SplitMix64 reference over 16,384 calls");
  return 0;
}
