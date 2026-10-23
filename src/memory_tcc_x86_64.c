#include <stddef.h>
#include <stdint.h>

/* TinyCC leaves Nelua's unrolled byte assignments in the stack frame. Keep
 * the TCC hybrid path on compact hardware helpers instead. */
void *nelibc_tcc_memcpy(void *restrict destination,
                        const void *restrict source, size_t length) {
  void *result = destination;
#if defined(__x86_64__)
  __asm__ volatile("rep movsb"
                   : "+D"(destination), "+S"(source), "+c"(length)
                   :
                   : "memory");
#else
  unsigned char *output = (unsigned char *)destination;
  const unsigned char *input = (const unsigned char *)source;
  for (size_t index = 0; index < length; ++index) output[index] = input[index];
#endif
  return result;
}

void *nelibc_tcc_memset(void *destination, int value, size_t length) {
#if defined(__x86_64__)
  extern void *nelibc_tcc_sse_memset(void *, int, size_t);
  /* SSE2 wins for small fills; ERMS is faster for larger ranges. */
  if (length <= 256)
    return nelibc_tcc_sse_memset(destination, value, length);
  void *result = destination;
  unsigned char byte_value = (unsigned char)value;
  __asm__ volatile("rep stosb"
                   : "+D"(destination), "+c"(length)
                   : "a"(byte_value)
                   : "memory");
  return result;
#else
  void *result = destination;
  unsigned char *output = (unsigned char *)destination;
  for (size_t index = 0; index < length; ++index)
    output[index] = (unsigned char)value;
  return result;
#endif
}
