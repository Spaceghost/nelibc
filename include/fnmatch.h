#ifndef NELIBC_FNMATCH_WRAPPER_H
#define NELIBC_FNMATCH_WRAPPER_H

#ifdef NELIBC_STANDALONE
#define FNM_PATHNAME 1
#define FNM_NOESCAPE 2
#define FNM_PERIOD 4
#define FNM_LEADING_DIR 8
#define FNM_CASEFOLD 16
#define FNM_NOMATCH 1
int fnmatch(const char *pattern, const char *text, int flags);
#else
#include_next <fnmatch.h>
#endif

#endif
