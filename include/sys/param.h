#if !defined(NELIBC_STANDALONE)
#include_next <sys/param.h>
#elif !defined(NELIBC_SYS_PARAM_H)
#define NELIBC_SYS_PARAM_H

#include <limits.h>

#define MAXPATHLEN PATH_MAX

#endif
