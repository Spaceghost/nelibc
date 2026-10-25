#ifndef NELIBC_WCTYPE_WRAPPER_H
#define NELIBC_WCTYPE_WRAPPER_H

#ifdef NELIBC_STANDALONE

#include <wchar.h>

typedef unsigned long wctype_t;

int iswalnum(wint_t);
int iswalpha(wint_t);
int iswblank(wint_t);
int iswcntrl(wint_t);
int iswdigit(wint_t);
int iswgraph(wint_t);
int iswlower(wint_t);
int iswprint(wint_t);
int iswpunct(wint_t);
int iswspace(wint_t);
int iswupper(wint_t);
int iswxdigit(wint_t);

#else

#include_next <wctype.h>

#endif

#endif
