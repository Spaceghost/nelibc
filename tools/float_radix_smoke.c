#include <errno.h>
#include <stdlib.h>
#include <string.h>

int nelibc_radix_character(void) { return ','; }

int main(void) {
  char *end = NULL;

  errno = 0;
  double double_value = strtod("123,5tail", &end);
  if (double_value != 123.5 || end == NULL || strcmp(end, "tail") != 0 || errno != 0)
    return 1;

  errno = 0;
  float float_value = strtof("-0,125tail", &end);
  if (float_value != -0.125f || end == NULL || strcmp(end, "tail") != 0 || errno != 0)
    return 2;

  errno = 0;
  long double long_value = strtold("2,5e1tail", &end);
  if (long_value != 25.0L || end == NULL || strcmp(end, "tail") != 0 || errno != 0)
    return 3;

  errno = 0;
  double hex_value = strtod("0x1,8p+1tail", &end);
  if (hex_value != 3.0 || end == NULL || strcmp(end, "tail") != 0 || errno != 0)
    return 4;

  errno = 0;
  double period_value = strtod("1.5", &end);
  if (period_value != 1.0 || end == NULL || strcmp(end, ".5") != 0 || errno != 0)
    return 5;

  return 0;
}
