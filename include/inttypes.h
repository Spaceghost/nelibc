#ifndef NELIBC_INTTYPES_WRAPPER_H
#define NELIBC_INTTYPES_WRAPPER_H

#if !defined(NELIBC_STANDALONE)
/* Reuse the toolchain's fixed-width and format-macro definitions. */
#include_next <inttypes.h>
#else
/* Standalone: no host libc to defer to. The compiler's freestanding
 * <stdint.h> supplies the types; the format macros are spelled here. */
#include <stdint.h>
typedef struct { intmax_t quot, rem; } imaxdiv_t;
/* Length modifiers that match the compiler's <stdint.h> types. */
#if __INT_FAST8_MAX__ == __LONG_MAX__
#define NELIBC_INTTYPES_FAST8 "l"
#elif __INT_FAST8_MAX__ == __LONG_LONG_MAX__
#define NELIBC_INTTYPES_FAST8 "ll"
#elif __INT_FAST8_MAX__ == __INT_MAX__
#define NELIBC_INTTYPES_FAST8 ""
#elif __INT_FAST8_MAX__ == __SHRT_MAX__
#define NELIBC_INTTYPES_FAST8 "h"
#else
#define NELIBC_INTTYPES_FAST8 "hh"
#endif
#if __INT_FAST16_MAX__ == __LONG_MAX__
#define NELIBC_INTTYPES_FAST16 "l"
#elif __INT_FAST16_MAX__ == __LONG_LONG_MAX__
#define NELIBC_INTTYPES_FAST16 "ll"
#elif __INT_FAST16_MAX__ == __INT_MAX__
#define NELIBC_INTTYPES_FAST16 ""
#elif __INT_FAST16_MAX__ == __SHRT_MAX__
#define NELIBC_INTTYPES_FAST16 "h"
#else
#define NELIBC_INTTYPES_FAST16 "hh"
#endif
#if __INT_FAST32_MAX__ == __LONG_MAX__
#define NELIBC_INTTYPES_FAST32 "l"
#elif __INT_FAST32_MAX__ == __LONG_LONG_MAX__
#define NELIBC_INTTYPES_FAST32 "ll"
#elif __INT_FAST32_MAX__ == __INT_MAX__
#define NELIBC_INTTYPES_FAST32 ""
#elif __INT_FAST32_MAX__ == __SHRT_MAX__
#define NELIBC_INTTYPES_FAST32 "h"
#else
#define NELIBC_INTTYPES_FAST32 "hh"
#endif
#if __INT_FAST64_MAX__ == __LONG_MAX__
#define NELIBC_INTTYPES_FAST64 "l"
#elif __INT_FAST64_MAX__ == __LONG_LONG_MAX__
#define NELIBC_INTTYPES_FAST64 "ll"
#elif __INT_FAST64_MAX__ == __INT_MAX__
#define NELIBC_INTTYPES_FAST64 ""
#elif __INT_FAST64_MAX__ == __SHRT_MAX__
#define NELIBC_INTTYPES_FAST64 "h"
#else
#define NELIBC_INTTYPES_FAST64 "hh"
#endif
#if __INTMAX_MAX__ == __LONG_MAX__
#define NELIBC_INTTYPES_MAX "l"
#elif __INTMAX_MAX__ == __LONG_LONG_MAX__
#define NELIBC_INTTYPES_MAX "ll"
#elif __INTMAX_MAX__ == __INT_MAX__
#define NELIBC_INTTYPES_MAX ""
#elif __INTMAX_MAX__ == __SHRT_MAX__
#define NELIBC_INTTYPES_MAX "h"
#else
#define NELIBC_INTTYPES_MAX "hh"
#endif
#if __INTPTR_MAX__ == __LONG_MAX__
#define NELIBC_INTTYPES_PTR "l"
#elif __INTPTR_MAX__ == __LONG_LONG_MAX__
#define NELIBC_INTTYPES_PTR "ll"
#elif __INTPTR_MAX__ == __INT_MAX__
#define NELIBC_INTTYPES_PTR ""
#elif __INTPTR_MAX__ == __SHRT_MAX__
#define NELIBC_INTTYPES_PTR "h"
#else
#define NELIBC_INTTYPES_PTR "hh"
#endif
#define PRId8 "d"
#define PRIdLEAST8 PRId8
#define PRIdFAST8 NELIBC_INTTYPES_FAST8 "d"
#define PRId16 "d"
#define PRIdLEAST16 PRId16
#define PRIdFAST16 NELIBC_INTTYPES_FAST16 "d"
#define PRId32 "d"
#define PRIdLEAST32 PRId32
#define PRIdFAST32 NELIBC_INTTYPES_FAST32 "d"
#define PRId64 "ld"
#define PRIdLEAST64 PRId64
#define PRIdFAST64 NELIBC_INTTYPES_FAST64 "d"
#define PRIdMAX NELIBC_INTTYPES_MAX "d"
#define PRIdPTR NELIBC_INTTYPES_PTR "d"
#define PRIi8 "i"
#define PRIiLEAST8 PRIi8
#define PRIiFAST8 NELIBC_INTTYPES_FAST8 "i"
#define PRIi16 "i"
#define PRIiLEAST16 PRIi16
#define PRIiFAST16 NELIBC_INTTYPES_FAST16 "i"
#define PRIi32 "i"
#define PRIiLEAST32 PRIi32
#define PRIiFAST32 NELIBC_INTTYPES_FAST32 "i"
#define PRIi64 "li"
#define PRIiLEAST64 PRIi64
#define PRIiFAST64 NELIBC_INTTYPES_FAST64 "i"
#define PRIiMAX NELIBC_INTTYPES_MAX "i"
#define PRIiPTR NELIBC_INTTYPES_PTR "i"
#define PRIo8 "o"
#define PRIoLEAST8 PRIo8
#define PRIoFAST8 NELIBC_INTTYPES_FAST8 "o"
#define PRIo16 "o"
#define PRIoLEAST16 PRIo16
#define PRIoFAST16 NELIBC_INTTYPES_FAST16 "o"
#define PRIo32 "o"
#define PRIoLEAST32 PRIo32
#define PRIoFAST32 NELIBC_INTTYPES_FAST32 "o"
#define PRIo64 "lo"
#define PRIoLEAST64 PRIo64
#define PRIoFAST64 NELIBC_INTTYPES_FAST64 "o"
#define PRIoMAX NELIBC_INTTYPES_MAX "o"
#define PRIoPTR NELIBC_INTTYPES_PTR "o"
#define PRIu8 "u"
#define PRIuLEAST8 PRIu8
#define PRIuFAST8 NELIBC_INTTYPES_FAST8 "u"
#define PRIu16 "u"
#define PRIuLEAST16 PRIu16
#define PRIuFAST16 NELIBC_INTTYPES_FAST16 "u"
#define PRIu32 "u"
#define PRIuLEAST32 PRIu32
#define PRIuFAST32 NELIBC_INTTYPES_FAST32 "u"
#define PRIu64 "lu"
#define PRIuLEAST64 PRIu64
#define PRIuFAST64 NELIBC_INTTYPES_FAST64 "u"
#define PRIuMAX NELIBC_INTTYPES_MAX "u"
#define PRIuPTR NELIBC_INTTYPES_PTR "u"
#define PRIx8 "x"
#define PRIxLEAST8 PRIx8
#define PRIxFAST8 NELIBC_INTTYPES_FAST8 "x"
#define PRIx16 "x"
#define PRIxLEAST16 PRIx16
#define PRIxFAST16 NELIBC_INTTYPES_FAST16 "x"
#define PRIx32 "x"
#define PRIxLEAST32 PRIx32
#define PRIxFAST32 NELIBC_INTTYPES_FAST32 "x"
#define PRIx64 "lx"
#define PRIxLEAST64 PRIx64
#define PRIxFAST64 NELIBC_INTTYPES_FAST64 "x"
#define PRIxMAX NELIBC_INTTYPES_MAX "x"
#define PRIxPTR NELIBC_INTTYPES_PTR "x"
#define PRIX8 "X"
#define PRIXLEAST8 PRIX8
#define PRIXFAST8 NELIBC_INTTYPES_FAST8 "X"
#define PRIX16 "X"
#define PRIXLEAST16 PRIX16
#define PRIXFAST16 NELIBC_INTTYPES_FAST16 "X"
#define PRIX32 "X"
#define PRIXLEAST32 PRIX32
#define PRIXFAST32 NELIBC_INTTYPES_FAST32 "X"
#define PRIX64 "lX"
#define PRIXLEAST64 PRIX64
#define PRIXFAST64 NELIBC_INTTYPES_FAST64 "X"
#define PRIXMAX NELIBC_INTTYPES_MAX "X"
#define PRIXPTR NELIBC_INTTYPES_PTR "X"
#define SCNd8 "hhd"
#define SCNdLEAST8 SCNd8
#define SCNdFAST8 NELIBC_INTTYPES_FAST8 "d"
#define SCNd16 "hd"
#define SCNdLEAST16 SCNd16
#define SCNdFAST16 NELIBC_INTTYPES_FAST16 "d"
#define SCNd32 "d"
#define SCNdLEAST32 SCNd32
#define SCNdFAST32 NELIBC_INTTYPES_FAST32 "d"
#define SCNd64 "ld"
#define SCNdLEAST64 SCNd64
#define SCNdFAST64 NELIBC_INTTYPES_FAST64 "d"
#define SCNdMAX NELIBC_INTTYPES_MAX "d"
#define SCNdPTR NELIBC_INTTYPES_PTR "d"
#define SCNi8 "hhi"
#define SCNiLEAST8 SCNi8
#define SCNiFAST8 NELIBC_INTTYPES_FAST8 "i"
#define SCNi16 "hi"
#define SCNiLEAST16 SCNi16
#define SCNiFAST16 NELIBC_INTTYPES_FAST16 "i"
#define SCNi32 "i"
#define SCNiLEAST32 SCNi32
#define SCNiFAST32 NELIBC_INTTYPES_FAST32 "i"
#define SCNi64 "li"
#define SCNiLEAST64 SCNi64
#define SCNiFAST64 NELIBC_INTTYPES_FAST64 "i"
#define SCNiMAX NELIBC_INTTYPES_MAX "i"
#define SCNiPTR NELIBC_INTTYPES_PTR "i"
#define SCNo8 "hho"
#define SCNoLEAST8 SCNo8
#define SCNoFAST8 NELIBC_INTTYPES_FAST8 "o"
#define SCNo16 "ho"
#define SCNoLEAST16 SCNo16
#define SCNoFAST16 NELIBC_INTTYPES_FAST16 "o"
#define SCNo32 "o"
#define SCNoLEAST32 SCNo32
#define SCNoFAST32 NELIBC_INTTYPES_FAST32 "o"
#define SCNo64 "lo"
#define SCNoLEAST64 SCNo64
#define SCNoFAST64 NELIBC_INTTYPES_FAST64 "o"
#define SCNoMAX NELIBC_INTTYPES_MAX "o"
#define SCNoPTR NELIBC_INTTYPES_PTR "o"
#define SCNu8 "hhu"
#define SCNuLEAST8 SCNu8
#define SCNuFAST8 NELIBC_INTTYPES_FAST8 "u"
#define SCNu16 "hu"
#define SCNuLEAST16 SCNu16
#define SCNuFAST16 NELIBC_INTTYPES_FAST16 "u"
#define SCNu32 "u"
#define SCNuLEAST32 SCNu32
#define SCNuFAST32 NELIBC_INTTYPES_FAST32 "u"
#define SCNu64 "lu"
#define SCNuLEAST64 SCNu64
#define SCNuFAST64 NELIBC_INTTYPES_FAST64 "u"
#define SCNuMAX NELIBC_INTTYPES_MAX "u"
#define SCNuPTR NELIBC_INTTYPES_PTR "u"
#define SCNx8 "hhx"
#define SCNxLEAST8 SCNx8
#define SCNxFAST8 NELIBC_INTTYPES_FAST8 "x"
#define SCNx16 "hx"
#define SCNxLEAST16 SCNx16
#define SCNxFAST16 NELIBC_INTTYPES_FAST16 "x"
#define SCNx32 "x"
#define SCNxLEAST32 SCNx32
#define SCNxFAST32 NELIBC_INTTYPES_FAST32 "x"
#define SCNx64 "lx"
#define SCNxLEAST64 SCNx64
#define SCNxFAST64 NELIBC_INTTYPES_FAST64 "x"
#define SCNxMAX NELIBC_INTTYPES_MAX "x"
#define SCNxPTR NELIBC_INTTYPES_PTR "x"
#endif
/* The wide conversions below are declared with wchar_t (C11 7.8.1). */
#include <stddef.h>

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
