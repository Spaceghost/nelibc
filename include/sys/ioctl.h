#ifndef NELIBC_SYS_IOCTL_WRAPPER_H
#define NELIBC_SYS_IOCTL_WRAPPER_H

#ifdef NELIBC_STANDALONE
/* Linux UAPI request encodings and request numbers from the compiler sysroot. */
#include <asm/ioctl.h>
#include <asm/ioctls.h>
int ioctl(int fd, unsigned long request, ...);
/* TIOCGWINSZ/TIOCSWINSZ argument, as musl and glibc declare it here. */
#ifndef NELIBC_STRUCT_WINSIZE
#define NELIBC_STRUCT_WINSIZE
struct winsize {
  unsigned short ws_row;
  unsigned short ws_col;
  unsigned short ws_xpixel;
  unsigned short ws_ypixel;
};
#endif
#else
#include_next <sys/ioctl.h>
#endif

#include "nelibc_syscall.h"

#endif
