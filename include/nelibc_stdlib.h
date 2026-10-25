#ifndef NELIBC_STDLIB_EXTENSIONS_H
#define NELIBC_STDLIB_EXTENSIONS_H

#include <stddef.h>

#if defined(_GNU_SOURCE) || defined(NELIBC_STANDALONE)
#include <locale.h>
#endif

void exit(int status);
void _Exit(int status);
int atexit(void (*function)(void));
void abort(void);
int system(const char *command);
int getsubopt(char **optionp, char *const *tokens, char **valuep);
int mblen(const char *text, size_t size);
int mbtowc(wchar_t *restrict wide, const char *restrict text, size_t size);
int wctomb(char *output, wchar_t wide);
size_t mbstowcs(wchar_t *restrict output, const char *restrict text, size_t limit);
size_t wcstombs(char *restrict output, const wchar_t *restrict wide_text, size_t limit);

/* Nelua checks overflow, then delegates to this build's matching realloc. */
void *reallocarray(void *ptr, size_t count, size_t size);

#if defined(_GNU_SOURCE) || defined(NELIBC_STANDALONE)
/* GNU context-aware sort. */
void qsort_r(void *base, size_t count, size_t width,
             int (*compare)(const void *, const void *, void *), void *context);
#endif

/* Hybrid archives keep these on the host allocator; standalone uses Nelibc. */
void *aligned_alloc(size_t alignment, size_t size);
int posix_memalign(void **memptr, size_t alignment, size_t size);

char *getenv(const char *name);
int clearenv(void);
int setenv(const char *name, const char *value, int overwrite);
int unsetenv(const char *name);
int putenv(char *string);
char *realpath(const char *restrict path, char *restrict resolved_path);
float strtof(const char *restrict text, char **restrict end_text);
double strtod(const char *restrict text, char **restrict end_text);
long double strtold(const char *restrict text, char **restrict end_text);
#if defined(_GNU_SOURCE) || defined(NELIBC_STANDALONE)
float strtof_l(const char *restrict text, char **restrict end_text, locale_t locale);
double strtod_l(const char *restrict text, char **restrict end_text, locale_t locale);
long double strtold_l(const char *restrict text, char **restrict end_text, locale_t locale);
#endif
long strtol(const char *restrict text, char **restrict end_text, int base);
unsigned long strtoul(const char *restrict text, char **restrict end_text, int base);
long long strtoll(const char *restrict text, char **restrict end_text, int base);
unsigned long long strtoull(const char *restrict text, char **restrict end_text, int base);
long __isoc23_strtol(const char *restrict text, char **restrict end_text, int base);
unsigned long __isoc23_strtoul(const char *restrict text, char **restrict end_text, int base);
long long __isoc23_strtoll(const char *restrict text, char **restrict end_text, int base);
unsigned long long __isoc23_strtoull(const char *restrict text, char **restrict end_text, int base);
int atoi(const char *text);
long atol(const char *text);
long long atoll(const char *text);
double atof(const char *text);
int abs(int value);
long labs(long value);
long long llabs(long long value);
int rand(void);
int rand_r(unsigned int *seed);
void srand(unsigned int seed);
int mkstemp(char *template);
int mkstemps(char *template, int suffix_length);
int mkostemp(char *template, int flags);
int mkostemps(char *template, int suffix_length, int flags);
char *mkdtemp(char *template);
void qsort(void *base, size_t count, size_t width,
           int (*compare)(const void *, const void *));
void *bsearch(const void *key, const void *base, size_t count,
              size_t width, int (*compare)(const void *, const void *));

#endif
