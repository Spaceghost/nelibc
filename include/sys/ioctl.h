#ifndef NELIBC_SYS_IOCTL_WRAPPER_H
#define NELIBC_SYS_IOCTL_WRAPPER_H

#ifdef NELIBC_STANDALONE
/* Linux UAPI request encodings and request numbers from the compiler sysroot. */
#include <asm/ioctl.h>
#include <asm/ioctls.h>
int ioctl(int fd, unsigned long request, ...);
#else
#include_next <sys/ioctl.h>
#endif

#include "nelibc_syscall.h"

#endif
