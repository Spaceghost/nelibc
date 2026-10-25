#include <complex.h>
#include <math.h>

int main(void) {
  float _Complex zf = 3.0f + 4.0f * I;
  double _Complex z = -3.0 + 4.0 * I;
  long double _Complex zl = 3.0L - 4.0L * I;
  if (cabsf(zf) != 5.0f || cabs(z) != 5.0 || cabsl(zl) != 5.0L) return 1;
  if (cabsf(0.0f + 0.0f * I) != 0.0f || cabs(0.0 + -0.0 * I) != 0.0 ||
      cabsl(0.0L + 0.0L * I) != 0.0L) return 2;
  if (!isinf(cabs(INFINITY + 0.0 * I))) return 3;
  return 0;
}
