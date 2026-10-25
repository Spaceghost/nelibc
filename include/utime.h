#ifndef NELIBC_UTIME_WRAPPER_H
#define NELIBC_UTIME_WRAPPER_H

#ifdef NELIBC_STANDALONE

#include <sys/types.h>

#ifndef __utimbuf_defined
#define __utimbuf_defined 1
struct utimbuf {
  time_t actime;
  time_t modtime;
};
#endif

#else

#include_next <utime.h>

#endif

int utime(const char *pathname, const struct utimbuf *times);

#endif
