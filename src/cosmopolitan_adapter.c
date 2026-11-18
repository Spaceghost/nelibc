/* Compile only with Cosmopolitan headers: share its errno and locale. */
#include <errno.h>
#include <stddef.h>
#include <string.h>
#ifndef __COSMOPOLITAN__
#error "This adapter requires the Cosmopolitan toolchain"
#endif

int *nelibc_errno_location(void) { return &errno; }
int nelibc_radix_character(void) { return '.'; }
static _Thread_local char *strtok_cursor;
static _Thread_local char signal_buffer[128];
char **nelibc_strtok_state(void) { return &strtok_cursor; }
char *nelibc_strsignal_buffer(void) { return signal_buffer; }
/* Cosmopolitan implements C-locale collation. Its _l symbols alias the
 * public symbols, so calling them here would pull duplicate definitions. */
int nelibc_locale_strcoll(const char *a, const char *b) {
  return strcmp(a, b);
}
size_t nelibc_locale_strxfrm(char *out, const char *in, size_t n) {
  size_t length = strlen(in);
  if (n) {
    size_t count = length < n ? length : n - 1;
    memcpy(out, in, count);
    out[count] = 0;
  }
  return length;
}
