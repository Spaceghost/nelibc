#ifndef NELIBC_INTTYPES_WRAPPER_H
#define NELIBC_INTTYPES_WRAPPER_H

/* Reuse the toolchain's fixed-width and format-macro definitions. */
#include_next <inttypes.h>

intmax_t strtoimax(const char *restrict, char **restrict, int);
uintmax_t strtoumax(const char *restrict, char **restrict, int);
intmax_t wcstoimax(const wchar_t *restrict, wchar_t **restrict, int);
uintmax_t wcstoumax(const wchar_t *restrict, wchar_t **restrict, int);
intmax_t imaxabs(intmax_t);
imaxdiv_t imaxdiv(intmax_t, intmax_t);

#if defined(_GNU_SOURCE) || defined(NELIBC_STANDALONE) || \
    (defined(__STDC_VERSION__) && __STDC_VERSION__ >= 202311L)
intmax_t __isoc23_strtoimax(const char *restrict, char **restrict, int);
uintmax_t __isoc23_strtoumax(const char *restrict, char **restrict, int);
intmax_t __isoc23_wcstoimax(const wchar_t *restrict, wchar_t **restrict, int);
uintmax_t __isoc23_wcstoumax(const wchar_t *restrict, wchar_t **restrict, int);
#endif

#endif
