#ifndef NELIBC_FCNTL_WRAPPER_H
#define NELIBC_FCNTL_WRAPPER_H

#ifdef NELIBC_STANDALONE
/* Use Linux UAPI definitions so standalone builds do not enter libc headers. */
#include <stddef.h>
#include <sys/types.h>
#include <linux/fcntl.h>

int fcntl(int fd, int command, ...);
int open(const char *pathname, int flags, ...);
int openat(int dirfd, const char *pathname, int flags, ...);
int creat(const char *pathname, mode_t mode);
int fallocate(int fd, int mode, off_t offset, off_t length);
int posix_fallocate(int fd, off_t offset, off_t length);
int posix_fadvise(int fd, off_t offset, off_t length, int advice);
ssize_t readahead(int fd, off_t offset, size_t count);
#ifndef POSIX_FADV_NORMAL
#define POSIX_FADV_NORMAL 0
#define POSIX_FADV_RANDOM 1
#define POSIX_FADV_SEQUENTIAL 2
#define POSIX_FADV_WILLNEED 3
#define POSIX_FADV_DONTNEED 4
#define POSIX_FADV_NOREUSE 5
#endif
#else

#include_next <fcntl.h>

/* Add Nelibc's Linux syscall declarations to the standard file API header. */
#include "nelibc_syscall.h"
#endif

/* Large-file aliases have the same ABI as open/openat on Linux x86-64. */
#if defined(_LARGEFILE64_SOURCE) || defined(_GNU_SOURCE)
int open64(const char *pathname, int flags, ...);
int openat64(int dirfd, const char *pathname, int flags, ...);
#endif

#endif
