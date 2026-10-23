#include <stddef.h>

/* Overridden by the Nelua stdio module when it is linked. */
__attribute__((weak)) void nelibc_stdio_init(void) {}

/* The host-FILE formatter sink is used only by hybrid builds. */
__attribute__((weak)) size_t nelibc_host_format_write(
    void *stream, const void *buffer, size_t count) {
  (void)stream;
  (void)buffer;
  (void)count;
  return 0;
}
