#ifndef NELIBC_WCHAR_WRAPPER_H
#define NELIBC_WCHAR_WRAPPER_H

#ifdef NELIBC_STANDALONE

#include <stddef.h>

typedef unsigned int wint_t;
typedef union {
    unsigned int __nelibc_state;
    unsigned long __nelibc_align;
    unsigned char __nelibc_storage[128];
} mbstate_t;

#define WEOF ((wint_t)0xffffffffU)

wchar_t *wmemcpy(wchar_t *restrict, const wchar_t *restrict, size_t);
wchar_t *wmemmove(wchar_t *, const wchar_t *, size_t);
wchar_t *wmemset(wchar_t *, wchar_t, size_t);
int wmemcmp(const wchar_t *, const wchar_t *, size_t);
wchar_t *wmemchr(const wchar_t *, wchar_t, size_t);
size_t wcslen(const wchar_t *);
size_t wcsnlen(const wchar_t *, size_t);
wchar_t *wcsdup(const wchar_t *);
int wcscmp(const wchar_t *, const wchar_t *);
int wcsncmp(const wchar_t *, const wchar_t *, size_t);
wchar_t *wcschr(const wchar_t *, wchar_t);
wchar_t *wcsrchr(const wchar_t *, wchar_t);
wchar_t *wcscpy(wchar_t *restrict, const wchar_t *restrict);
wchar_t *wcsncpy(wchar_t *restrict, const wchar_t *restrict, size_t);
wchar_t *wcscat(wchar_t *restrict, const wchar_t *restrict);
wchar_t *wcsncat(wchar_t *restrict, const wchar_t *restrict, size_t);
size_t wcsspn(const wchar_t *, const wchar_t *);
size_t wcscspn(const wchar_t *, const wchar_t *);
wchar_t *wcspbrk(const wchar_t *, const wchar_t *);
wchar_t *wcstok(wchar_t *restrict, const wchar_t *restrict,
                wchar_t **restrict);
wchar_t *wcsstr(const wchar_t *, const wchar_t *);

int mblen(const char *, size_t);
int mbtowc(wchar_t *restrict, const char *restrict, size_t);
int wctomb(char *, wchar_t);
size_t mbrtowc(wchar_t *restrict, const char *restrict, size_t,
               mbstate_t *restrict);
size_t mbrlen(const char *restrict, size_t, mbstate_t *restrict);
int mbsinit(const mbstate_t *);
size_t wcrtomb(char *restrict, wchar_t, mbstate_t *restrict);
wint_t btowc(int);
int wctob(wint_t);
size_t mbstowcs(wchar_t *restrict, const char *restrict, size_t);
size_t wcstombs(char *restrict, const wchar_t *restrict, size_t);
size_t mbsrtowcs(wchar_t *restrict, const char **restrict, size_t,
                 mbstate_t *restrict);
size_t mbsnrtowcs(wchar_t *restrict, const char **restrict, size_t, size_t,
                  mbstate_t *restrict);
size_t wcsrtombs(char *restrict, const wchar_t **restrict, size_t,
                 mbstate_t *restrict);
size_t wcsnrtombs(char *restrict, const wchar_t **restrict, size_t, size_t,
                  mbstate_t *restrict);

long wcstol(const wchar_t *restrict, wchar_t **restrict, int);
long long wcstoll(const wchar_t *restrict, wchar_t **restrict, int);
unsigned long wcstoul(const wchar_t *restrict, wchar_t **restrict, int);
unsigned long long wcstoull(const wchar_t *restrict, wchar_t **restrict, int);

#else

/* Reuse the toolchain's wide-character types and standard API declarations. */
#include_next <wchar.h>

#endif

/* C23 integer conversion entry points used by GNU-mode headers. */
#if defined(_GNU_SOURCE) || defined(NELIBC_STANDALONE) || \
    (defined(__STDC_VERSION__) && __STDC_VERSION__ >= 202311L)
long __isoc23_wcstol(const wchar_t *restrict, wchar_t **restrict, int);
long long __isoc23_wcstoll(const wchar_t *restrict, wchar_t **restrict, int);
unsigned long __isoc23_wcstoul(const wchar_t *restrict, wchar_t **restrict, int);
unsigned long long __isoc23_wcstoull(const wchar_t *restrict, wchar_t **restrict, int);
#endif

#endif
