#ifndef NELIBC_SYS_FILE_WRAPPER_H
#define NELIBC_SYS_FILE_WRAPPER_H

#ifdef NELIBC_STANDALONE

#include <fcntl.h>

#define LOCK_SH 1
#define LOCK_EX 2
#define LOCK_NB 4
#define LOCK_UN 8

#ifdef __cplusplus
extern "C" {
#endif
int flock(int fd, int operation);
#ifdef __cplusplus
}
#endif

#else
#include_next <sys/file.h>
#endif

#endif
