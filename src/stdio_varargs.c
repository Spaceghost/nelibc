#include <float.h>
#include <stdarg.h>
#include <stddef.h>
#include <stdint.h>


/* Formatters own a va_list copy and share its address so helpers advance
 * the same cursor whether va_list is an array or a struct. */
int nelibc_va_arg_int(va_list *arguments) { return va_arg(*arguments, int); }
void *nelibc_va_arg_pointer(va_list *arguments) { return va_arg(*arguments, void *); }
__attribute__((no_stack_protector))
void *nelibc_va_arg_pointer_at(va_list *arguments, size_t position) {
  va_list copy;
  va_copy(copy, *arguments);
  void *value = NULL;
  for (size_t index = 0; index < position; ++index)
    value = va_arg(copy, void *);
  va_end(copy);
  return value;
}
long nelibc_va_arg_long(va_list *arguments) { return va_arg(*arguments, long); }
long long nelibc_va_arg_long_long(va_list *arguments) { return va_arg(*arguments, long long); }
unsigned long nelibc_va_arg_unsigned_long(va_list *arguments) { return va_arg(*arguments, unsigned long); }
unsigned long long nelibc_va_arg_unsigned_long_long(va_list *arguments) {
  return va_arg(*arguments, unsigned long long);
}
size_t nelibc_va_arg_size(va_list *arguments) { return va_arg(*arguments, size_t); }
unsigned int nelibc_va_arg_unsigned_int(va_list *arguments) { return va_arg(*arguments, unsigned int); }
double nelibc_va_arg_double(va_list *arguments) { return va_arg(*arguments, double); }
__attribute__((no_stack_protector))
static unsigned long long read_word(const unsigned char *raw) {
  unsigned long long value = 0;
  for (int index = 7; index >= 0; --index) value = (value << 8) | raw[index];
  return value;
}

/* Return an explicit significand as two words on both supported LE ABIs. */
__attribute__((no_stack_protector))
void nelibc_long_double_words(long double value, unsigned long long *low,
                              unsigned long long *high, unsigned *sign_exponent) {
  const unsigned char *raw = (const unsigned char *)&value;
  *low = read_word(raw);
#if LDBL_MANT_DIG == 113
  unsigned long long upper = read_word(raw + 8);
  *sign_exponent = (unsigned)(upper >> 48);
  *high = upper & 0x0000ffffffffffffULL;
  unsigned exponent = *sign_exponent & 0x7fff;
  if (exponent != 0 && exponent != 0x7fff) *high |= 1ULL << 48;
#else
  *high = 0;
  *sign_exponent = (unsigned)raw[8] | ((unsigned)raw[9] << 8);
#endif
}

__attribute__((no_stack_protector))
void nelibc_va_arg_long_double_bits(va_list *arguments, unsigned long long *low,
                                    unsigned long long *high, unsigned *sign_exponent) {
  nelibc_long_double_words(va_arg(*arguments, long double), low, high, sign_exponent);
}
/* The double a long double argument equals, with exact set when nothing was lost. */
__attribute__((no_stack_protector))
double nelibc_va_arg_long_double_as_double(va_list *arguments, int *exact) {
  long double value = va_arg(*arguments, long double);
  double narrowed = (double)value;
  *exact = (value != value) || ((long double)narrowed == value);
  return narrowed;
}
/* Keep binary128 finite values on the exact path without a narrowing
 * conversion that could raise inexact, overflow, or underflow. */
__attribute__((no_stack_protector))
double nelibc_va_arg_long_double_parts(va_list *arguments, int *exact,
                                       unsigned long long *low,
                                       unsigned long long *high,
                                       unsigned *sign_exponent) {
  long double value = va_arg(*arguments, long double);
  nelibc_long_double_words(value, low, high, sign_exponent);
#if LDBL_MANT_DIG == 113
  *exact = (*sign_exponent & 0x7fff) == 0x7fff;
  return *exact ? (double)value : 0.0;
#else
  double narrowed = (double)value;
  *exact = (value != value) || ((long double)narrowed == value);
  return narrowed;
#endif
}
void nelibc_va_copy(va_list destination, va_list source) {
  va_copy(destination, source);
}

void nelibc_store_format_count(void *pointer, int length, size_t count) {
  switch (length) {
    case 1: *(long *)pointer = (long)count; break;
    case 2: *(long long *)pointer = (long long)count; break;
    case 3: *(ptrdiff_t *)pointer = (ptrdiff_t)count; break;
    case 4: *(signed char *)pointer = (signed char)count; break;
    case 5: *(short *)pointer = (short)count; break;
    default: *(int *)pointer = (int)count; break;
  }
}

#include <stdlib.h>

long long nelibc_scan_strtoll(const char *text, char **end_text, int base) {
  return strtoll(text, end_text, base);
}
unsigned long long nelibc_scan_strtoull(const char *text, char **end_text, int base) {
  return strtoull(text, end_text, base);
}
float nelibc_scan_strtof(const char *text, char **end_text) {
  return strtof(text, end_text);
}
double nelibc_scan_strtod(const char *text, char **end_text) {
  return strtod(text, end_text);
}
long double nelibc_scan_strtold(const char *text, char **end_text) {
  return strtold(text, end_text);
}
