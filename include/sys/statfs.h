#ifndef NELIBC_SYS_STATFS_WRAPPER_H
#define NELIBC_SYS_STATFS_WRAPPER_H

#ifdef NELIBC_STANDALONE

#include <sys/types.h>

#ifdef __cplusplus
extern "C" {
#endif

/* Layout written by Linux statfs and fstatfs on x86-64. */
struct statfs {
  long f_type, f_bsize;
  fsblkcnt_t f_blocks, f_bfree, f_bavail;
  fsfilcnt_t f_files, f_ffree;
  struct { int __val[2]; } f_fsid;
  long f_namelen, f_frsize, f_flags, f_spare[4];
};

int statfs(const char *path, struct statfs *out);
int fstatfs(int fd, struct statfs *out);

#ifdef __cplusplus
}
#endif

#else
#include_next <sys/statfs.h>
#endif

#endif
