#ifndef NELIBC_CTYPE_H
#define NELIBC_CTYPE_H

int isalnum(int character);
int isalpha(int character);
int isblank(int character);
int iscntrl(int character);
int isdigit(int character);
int isgraph(int character);
int islower(int character);
int isprint(int character);
int ispunct(int character);
int isspace(int character);
int isupper(int character);
int isxdigit(int character);
int tolower(int character);
int toupper(int character);
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
int isascii(int character);
int toascii(int character);

#endif
