#include <stdarg.h>
#include <stddef.h>
#include <stdint.h>


/* A va_list parameter is already adjusted by the C ABI on array-based
 * implementations. Consume it in C so Nelua never takes its address. */
int nelibc_va_arg_int(va_list arguments) { return va_arg(arguments, int); }
void *nelibc_va_arg_pointer(va_list arguments) { return va_arg(arguments, void *); }
long nelibc_va_arg_long(va_list arguments) { return va_arg(arguments, long); }
long long nelibc_va_arg_long_long(va_list arguments) { return va_arg(arguments, long long); }
unsigned long nelibc_va_arg_unsigned_long(va_list arguments) { return va_arg(arguments, unsigned long); }
unsigned long long nelibc_va_arg_unsigned_long_long(va_list arguments) {
  return va_arg(arguments, unsigned long long);
}
size_t nelibc_va_arg_size(va_list arguments) { return va_arg(arguments, size_t); }
unsigned int nelibc_va_arg_unsigned_int(va_list arguments) { return va_arg(arguments, unsigned int); }
double nelibc_va_arg_double(va_list arguments) { return va_arg(arguments, double); }
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
