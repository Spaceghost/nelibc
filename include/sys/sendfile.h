#ifndef NELIBC_SYS_SENDFILE_H
#define NELIBC_SYS_SENDFILE_H

#ifdef NELIBC_STANDALONE

#include <stddef.h>
#include <sys/types.h>

ssize_t sendfile(int out_fd, int in_fd, off_t *offset, size_t count);
ssize_t sendfile64(int out_fd, int in_fd, off_t *offset, size_t count);

#else

#include_next <sys/sendfile.h>

#endif

#endif
