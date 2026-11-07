#ifndef NELIBC_SYS_STATVFS_WRAPPER_H
#define NELIBC_SYS_STATVFS_WRAPPER_H

/* Hybrid builds use the host's <sys/statvfs.h>. The standalone profile
 * provides statvfs and fstatvfs from src/extras_standalone.nelua, filled
 * from statfs(2) as musl 1.2.5 does (musl's x86-64 struct layout). */
#ifdef NELIBC_STANDALONE

#include <sys/types.h>

#ifdef __cplusplus
extern "C" {
#endif

struct statvfs {
  unsigned long f_bsize, f_frsize;
  fsblkcnt_t f_blocks, f_bfree, f_bavail;
  fsfilcnt_t f_files, f_ffree, f_favail;
  unsigned long f_fsid;
  unsigned long f_flag, f_namemax;
  unsigned int f_type;
  int __reserved[5];
};

int statvfs(const char *__restrict, struct statvfs *__restrict);
int fstatvfs(int, struct statvfs *);

#define ST_RDONLY 1
#define ST_NOSUID 2
#define ST_NODEV  4
#define ST_NOEXEC 8
#define ST_SYNCHRONOUS 16
#define ST_MANDLOCK    64
#define ST_WRITE       128
#define ST_APPEND      256
#define ST_IMMUTABLE   512
#define ST_NOATIME     1024
#define ST_NODIRATIME  2048
#define ST_RELATIME    4096

#ifdef __cplusplus
}
#endif

#else

#include_next <sys/statvfs.h>

#endif

#endif
