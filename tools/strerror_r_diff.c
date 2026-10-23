#define _POSIX_C_SOURCE 200809L
#include <errno.h>
#include <stdio.h>
#include <string.h>

#ifdef NELIBC_STRERROR_R_HOST
/* glibc's XSI interface is exported under this symbol alongside GNU strerror_r. */
extern int __xpg_strerror_r(int, char *, size_t);
#undef strerror_r
#define strerror_r __xpg_strerror_r
#endif

int main(void) {
  char buffer[128];
  for (int error_number = -2; error_number <= 133; error_number++) {
    int result = strerror_r(error_number, buffer, sizeof buffer);
    printf("%d:%d\n", error_number, result);
  }
  /* Linux reserves kernel EFTYPE 134, which glibc does not expose. */
  for (int error_number = 135; error_number <= 140; error_number++) {
    int result = strerror_r(error_number, buffer, sizeof buffer);
    printf("%d:%d\n", error_number, result);
  }

  if (strerror_r(2, buffer, sizeof buffer) != 0 ||
      strcmp(buffer, "No such file or directory") != 0)
    return 1;

  char short_buffer[2] = {'x', 'y'};
  int short_result = strerror_r(2, short_buffer, sizeof short_buffer);
  printf("short:%d\n", short_result);
  return 0;
}
