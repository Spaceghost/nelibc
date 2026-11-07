#ifndef NELIBC_LIBGEN_WRAPPER_H
#define NELIBC_LIBGEN_WRAPPER_H

/* Hybrid builds use the host's <libgen.h>. The standalone profile provides
 * basename and dirname from src/extras_standalone.nelua (after musl 1.2.5);
 * both may modify their argument, as POSIX allows. */
#ifdef NELIBC_STANDALONE

#ifdef __cplusplus
extern "C" {
#endif

char *dirname(char *);
char *basename(char *);

#ifdef __cplusplus
}
#endif

#else

#include_next <libgen.h>

#endif

#endif
