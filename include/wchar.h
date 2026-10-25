#ifndef NELIBC_WCHAR_WRAPPER_H
#define NELIBC_WCHAR_WRAPPER_H

#if defined(_GNU_SOURCE) || defined(NELIBC_STANDALONE)
#include <locale.h>
#endif

#ifdef NELIBC_STANDALONE

#include <stddef.h>
#include <stdarg.h>

typedef unsigned int wint_t;
typedef union {
    unsigned int __nelibc_state;
    unsigned long __nelibc_align;
    unsigned char __nelibc_storage[128];
} mbstate_t;

#define WEOF ((wint_t)0xffffffffU)

struct nelibc_FILE;
int fwide(struct nelibc_FILE *, int);
wint_t fgetwc(struct nelibc_FILE *);
wint_t fgetwc_unlocked(struct nelibc_FILE *);
wint_t getwc(struct nelibc_FILE *);
wint_t getwc_unlocked(struct nelibc_FILE *);
wint_t getwchar(void);
wint_t getwchar_unlocked(void);
wint_t fputwc(wint_t, struct nelibc_FILE *);
wint_t fputwc_unlocked(wint_t, struct nelibc_FILE *);
wint_t putwc(wint_t, struct nelibc_FILE *);
wint_t putwc_unlocked(wint_t, struct nelibc_FILE *);
wint_t putwchar(wint_t);
wint_t putwchar_unlocked(wint_t);
wint_t ungetwc(wint_t, struct nelibc_FILE *);
wchar_t *fgetws(wchar_t *restrict, int, struct nelibc_FILE *restrict);
wchar_t *fgetws_unlocked(wchar_t *restrict, int, struct nelibc_FILE *restrict);
int fputws(const wchar_t *restrict, struct nelibc_FILE *restrict);
int fputws_unlocked(const wchar_t *restrict, struct nelibc_FILE *restrict);
int swprintf(wchar_t *restrict, size_t, const wchar_t *restrict, ...);
int vswprintf(wchar_t *restrict, size_t, const wchar_t *restrict, va_list);
int fwprintf(struct nelibc_FILE *restrict, const wchar_t *restrict, ...);
int vfwprintf(struct nelibc_FILE *restrict, const wchar_t *restrict, va_list);
int wprintf(const wchar_t *restrict, ...);
int vwprintf(const wchar_t *restrict, va_list);
struct tm;
size_t wcsftime(wchar_t *restrict, size_t, const wchar_t *restrict,
                const struct tm *restrict);
size_t wcsftime_l(wchar_t *restrict, size_t, const wchar_t *restrict,
                  const struct tm *restrict, locale_t);

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
int wcscasecmp(const wchar_t *, const wchar_t *);
int wcsncasecmp(const wchar_t *, const wchar_t *, size_t);
int wcscasecmp_l(const wchar_t *, const wchar_t *, locale_t);
int wcsncasecmp_l(const wchar_t *, const wchar_t *, size_t, locale_t);
int wcwidth(wchar_t);
int wcswidth(const wchar_t *, size_t);
float wcstof(const wchar_t *restrict, wchar_t **restrict);
double wcstod(const wchar_t *restrict, wchar_t **restrict);
long double wcstold(const wchar_t *restrict, wchar_t **restrict);
int wcscoll(const wchar_t *, const wchar_t *);
int wcscoll_l(const wchar_t *, const wchar_t *, locale_t);
size_t wcsxfrm(wchar_t *restrict, const wchar_t *restrict, size_t);
size_t wcsxfrm_l(wchar_t *restrict, const wchar_t *restrict, size_t, locale_t);
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
wchar_t *wcpcpy(wchar_t *restrict, const wchar_t *restrict);
wchar_t *wcpncpy(wchar_t *restrict, const wchar_t *restrict, size_t);
wchar_t *wcswcs(const wchar_t *, const wchar_t *);

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
wint_t towlower(wint_t);
wint_t towupper(wint_t);
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
long wcstol_l(const wchar_t *restrict, wchar_t **restrict, int, locale_t);
unsigned long wcstoul_l(const wchar_t *restrict, wchar_t **restrict, int, locale_t);
long long wcstoll_l(const wchar_t *restrict, wchar_t **restrict, int, locale_t);
unsigned long long wcstoull_l(const wchar_t *restrict, wchar_t **restrict, int, locale_t);

#else

/* Reuse the toolchain's wide-character types and standard API declarations. */
#include_next <wchar.h>

#endif

#if defined(_GNU_SOURCE) || defined(NELIBC_STANDALONE)
long wcstol_l(const wchar_t *restrict, wchar_t **restrict, int, locale_t);
unsigned long wcstoul_l(const wchar_t *restrict, wchar_t **restrict, int, locale_t);
long long wcstoll_l(const wchar_t *restrict, wchar_t **restrict, int, locale_t);
unsigned long long wcstoull_l(const wchar_t *restrict, wchar_t **restrict, int, locale_t);
#endif

/* C23 integer conversion entry points used by GNU-mode headers. */
#if defined(_GNU_SOURCE) || defined(NELIBC_STANDALONE) || \
    (defined(__STDC_VERSION__) && __STDC_VERSION__ >= 202311L)
long __isoc23_wcstol(const wchar_t *restrict, wchar_t **restrict, int);
long long __isoc23_wcstoll(const wchar_t *restrict, wchar_t **restrict, int);
unsigned long __isoc23_wcstoul(const wchar_t *restrict, wchar_t **restrict, int);
unsigned long long __isoc23_wcstoull(const wchar_t *restrict, wchar_t **restrict, int);
long __isoc23_wcstol_l(const wchar_t *restrict, wchar_t **restrict, int, locale_t);
unsigned long __isoc23_wcstoul_l(const wchar_t *restrict, wchar_t **restrict, int, locale_t);
long long __isoc23_wcstoll_l(const wchar_t *restrict, wchar_t **restrict, int, locale_t);
unsigned long long __isoc23_wcstoull_l(const wchar_t *restrict, wchar_t **restrict, int, locale_t);
#endif

#endif
