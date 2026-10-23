#ifndef NELIBC_SYS_XATTR_H
#define NELIBC_SYS_XATTR_H

#ifdef NELIBC_STANDALONE

#include <linux/xattr.h>
#include "nelibc_syscall.h"

#else

#include_next <sys/xattr.h>

#endif

#endif
