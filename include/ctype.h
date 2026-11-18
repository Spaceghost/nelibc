#ifndef NELIBC_CTYPE_H
#define NELIBC_CTYPE_H
#include "nelibc_attributes.h"

int isalnum(int character) NELIBC_PURE;
int isalpha(int character) NELIBC_PURE;
int isblank(int character) NELIBC_PURE;
int iscntrl(int character) NELIBC_PURE;
int isdigit(int character) NELIBC_PURE;
int isgraph(int character) NELIBC_PURE;
int islower(int character) NELIBC_PURE;
int isprint(int character) NELIBC_PURE;
int ispunct(int character) NELIBC_PURE;
int isspace(int character) NELIBC_PURE;
int isupper(int character) NELIBC_PURE;
int isxdigit(int character) NELIBC_PURE;
int tolower(int character) NELIBC_PURE;
int toupper(int character) NELIBC_PURE;
#ifndef NELIBC_STANDALONE
#include <features.h>
#endif
/* POSIX.1-2008 locale-object classification (the _l suffix family). */
#if defined(_GNU_SOURCE) || defined(NELIBC_STANDALONE) || \
    (defined(_POSIX_C_SOURCE) && _POSIX_C_SOURCE >= 200809L) || \
    (defined(_XOPEN_SOURCE) && _XOPEN_SOURCE >= 700)
#include <locale.h>
int isalnum_l(int character, locale_t locale);
int isalpha_l(int character, locale_t locale);
int isblank_l(int character, locale_t locale);
int iscntrl_l(int character, locale_t locale);
int isdigit_l(int character, locale_t locale);
int isgraph_l(int character, locale_t locale);
int islower_l(int character, locale_t locale);
int isprint_l(int character, locale_t locale);
int ispunct_l(int character, locale_t locale);
int isspace_l(int character, locale_t locale);
int isupper_l(int character, locale_t locale);
int isxdigit_l(int character, locale_t locale);
int tolower_l(int character, locale_t locale);
int toupper_l(int character, locale_t locale);
#endif
int isascii(int character) NELIBC_CONST;
int toascii(int character) NELIBC_CONST;

#endif
