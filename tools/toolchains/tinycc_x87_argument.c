/* Regression for TinyCC's x86-64 long-double argument double-pop. */
#include <fenv.h>

__attribute__((noinline)) static int predicate(long double value) {
  return value != value;
}

__attribute__((noinline)) static long double identity(long double value) {
  if (predicate(value)) return 0.0L;
  return value;
}

int main(void) {
  if (feclearexcept(FE_ALL_EXCEPT)) return 1;
  volatile long double result = identity(1.0L);
  if (fetestexcept(FE_ALL_EXCEPT)) return 2;
  return result != 1.0L;
}
