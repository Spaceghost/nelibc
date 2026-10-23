#ifndef NELIBC_SYS_STAT_WRAPPER_H
#define NELIBC_SYS_STAT_WRAPPER_H

#ifdef NELIBC_STANDALONE
/* Own the public stat tag unless a preceding platform header already did. */
#if !defined(_BITS_STRUCT_STAT_H) && !defined(__DEFINED_struct_stat)
#define NELIBC_DEFINE_STAT_LAYOUT 1
#define _BITS_STRUCT_STAT_H 1
#endif
struct stat;
#include <sys/types.h>
#include <time.h>

/* Linux x86-64 struct stat ABI used by the raw newfstatat syscall. */
#ifdef NELIBC_DEFINE_STAT_LAYOUT
struct stat {
  dev_t st_dev;
  ino_t st_ino;
  nlink_t st_nlink;
  mode_t st_mode;
  uid_t st_uid;
  gid_t st_gid;
  int __pad0;
  dev_t st_rdev;
  off_t st_size;
  long st_blksize;
  long st_blocks;
  struct timespec st_atim;
  struct timespec st_mtim;
  struct timespec st_ctim;
  long __reserved[3];
};
#undef NELIBC_DEFINE_STAT_LAYOUT
#endif

#ifndef S_IFMT
#define S_IFMT 0170000
#endif
#ifndef S_IFSOCK
#define S_IFSOCK 0140000
#endif
#ifndef S_IFLNK
#define S_IFLNK 0120000
#endif
#ifndef S_IFREG
#define S_IFREG 0100000
#endif
#ifndef S_IFBLK
#define S_IFBLK 0060000
#endif
#ifndef S_IFDIR
#define S_IFDIR 0040000
#endif
#ifndef S_IFCHR
#define S_IFCHR 0020000
#endif
#ifndef S_IFIFO
#define S_IFIFO 0010000
#endif
#ifndef S_ISUID
#define S_ISUID 0004000
#endif
#ifndef S_ISGID
#define S_ISGID 0002000
#endif
#ifndef S_ISVTX
#define S_ISVTX 0001000
#endif
#ifndef S_IRWXU
#define S_IRWXU 00700
#endif
#ifndef S_IRUSR
#define S_IRUSR 00400
#endif
#ifndef S_IWUSR
#define S_IWUSR 00200
#endif
#ifndef S_IXUSR
#define S_IXUSR 00100
#endif
#ifndef S_IRWXG
#define S_IRWXG 00070
#endif
#ifndef S_IRGRP
#define S_IRGRP 00040
#endif
#ifndef S_IWGRP
#define S_IWGRP 00020
#endif
#ifndef S_IXGRP
#define S_IXGRP 00010
#endif
#ifndef S_IRWXO
#define S_IRWXO 00007
#endif
#ifndef S_IROTH
#define S_IROTH 00004
#endif
#ifndef S_IWOTH
#define S_IWOTH 00002
#endif
#ifndef S_IXOTH
#define S_IXOTH 00001
#endif
#ifndef S_ISREG
#define S_ISREG(mode) (((mode) & S_IFMT) == S_IFREG)
#endif
#ifndef S_ISDIR
#define S_ISDIR(mode) (((mode) & S_IFMT) == S_IFDIR)
#endif
#ifndef S_ISCHR
#define S_ISCHR(mode) (((mode) & S_IFMT) == S_IFCHR)
#endif
#ifndef S_ISBLK
#define S_ISBLK(mode) (((mode) & S_IFMT) == S_IFBLK)
#endif
#ifndef S_ISFIFO
#define S_ISFIFO(mode) (((mode) & S_IFMT) == S_IFIFO)
#endif
#ifndef S_ISLNK
#define S_ISLNK(mode) (((mode) & S_IFMT) == S_IFLNK)
#endif
#ifndef S_ISSOCK
#define S_ISSOCK(mode) (((mode) & S_IFMT) == S_IFSOCK)
#endif
#ifndef UTIME_NOW
#define UTIME_NOW  ((1L << 30) - 1L)
#endif
#ifndef UTIME_OMIT
#define UTIME_OMIT ((1L << 30) - 2L)
#endif

/* Linux statx UAPI layout, independent of libc-private stat headers. */
#ifndef STATX_TYPE
struct statx_timestamp {
  long long tv_sec;
  unsigned int tv_nsec;
  int __reserved;
};

struct statx {
  unsigned int stx_mask;
  unsigned int stx_blksize;
  unsigned long long stx_attributes;
  unsigned int stx_nlink;
  unsigned int stx_uid;
  unsigned int stx_gid;
  unsigned short stx_mode;
  unsigned short __spare0[1];
  unsigned long long stx_ino;
  unsigned long long stx_size;
  unsigned long long stx_blocks;
  unsigned long long stx_attributes_mask;
  struct statx_timestamp stx_atime;
  struct statx_timestamp stx_btime;
  struct statx_timestamp stx_ctime;
  struct statx_timestamp stx_mtime;
  unsigned int stx_rdev_major;
  unsigned int stx_rdev_minor;
  unsigned int stx_dev_major;
  unsigned int stx_dev_minor;
  unsigned long long stx_mnt_id;
  unsigned int stx_dio_mem_align;
  unsigned int stx_dio_offset_align;
  unsigned long long stx_subvol;
  unsigned int stx_atomic_write_unit_min;
  unsigned int stx_atomic_write_unit_max;
  unsigned int stx_atomic_write_segments_max;
  unsigned int stx_dio_read_offset_align;
  unsigned int stx_atomic_write_unit_max_opt;
  unsigned int __spare2[1];
  unsigned long long __spare3[8];
};

#define STATX_TYPE 0x00000001U
#define STATX_MODE 0x00000002U
#define STATX_NLINK 0x00000004U
#define STATX_UID 0x00000008U
#define STATX_GID 0x00000010U
#define STATX_ATIME 0x00000020U
#define STATX_MTIME 0x00000040U
#define STATX_CTIME 0x00000080U
#define STATX_INO 0x00000100U
#define STATX_SIZE 0x00000200U
#define STATX_BLOCKS 0x00000400U
#define STATX_BASIC_STATS 0x000007ffU
#define STATX_BTIME 0x00000800U
#define STATX_MNT_ID 0x00001000U
#define STATX_DIOALIGN 0x00002000U
#define STATX_MNT_ID_UNIQUE 0x00004000U
#define STATX_SUBVOL 0x00008000U
#define STATX_WRITE_ATOMIC 0x00010000U
#define STATX__RESERVED 0x80000000U
#define STATX_ALL 0x00000fffU
#endif
#else
#include_next <sys/stat.h>
#endif

/* Add Nelibc's POSIX and Linux syscall declarations to the public stat API. */
#include "nelibc_syscall.h"

#ifdef NELIBC_STANDALONE
int utimensat(int dirfd, const char *pathname,
              const struct timespec timestamps[2], int flags);
#endif

#endif
