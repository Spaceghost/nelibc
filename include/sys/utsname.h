#ifndef NELIBC_SYS_UTSNAME_WRAPPER_H
#define NELIBC_SYS_UTSNAME_WRAPPER_H

#ifdef NELIBC_STANDALONE
/* Linux kernel newuname ABI: six fixed-width, NUL-terminated fields. */
struct utsname {
  char sysname[65];
  char nodename[65];
  char release[65];
  char version[65];
  char machine[65];
  char domainname[65];
};
#else
#include_next <sys/utsname.h>
#endif

/* Add Nelibc's Linux syscall declarations to the standard uname API. */
#include "nelibc_syscall.h"

#endif
