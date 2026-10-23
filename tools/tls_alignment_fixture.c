#include <stdint.h>

_Alignas(64) static _Thread_local unsigned char aligned_tls_value = 0x5a;

int nelibc_tls_alignment_check(void) {
  uintptr_t address = (uintptr_t)&aligned_tls_value;
  if (address % 64 != 0 || aligned_tls_value != 0x5a) return 0;
  aligned_tls_value = 0xa5;
  return 1;
}

int nelibc_tls_alignment_recheck(void) {
  return ((uintptr_t)&aligned_tls_value % 64 == 0 && aligned_tls_value == 0xa5);
}
