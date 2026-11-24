/* Keep the Nelibc metadata layout consumed by the C and Nelua modules. */
#include <asm/errno.h>
#include <asm/unistd.h>
#define stat kernel_stat
#include <asm/stat.h>
#undef stat

#include <sys/stat.h>
#undef st_atime
#undef st_mtime
#undef st_ctime

extern long nelibc_raw_syscall3(unsigned long, unsigned long, unsigned long,
                                unsigned long);
extern long nelibc_raw_syscall6(unsigned long, unsigned long, unsigned long,
                                unsigned long, unsigned long, unsigned long,
                                unsigned long);

static void store_stat(struct stat *out, const struct kernel_stat *in) {
  out->st_dev = in->st_dev;
  out->st_ino = in->st_ino;
  out->st_nlink = in->st_nlink;
  out->st_mode = in->st_mode;
  out->st_uid = in->st_uid;
  out->st_gid = in->st_gid;
  out->__pad0 = 0;
  out->st_rdev = in->st_rdev;
  out->st_size = in->st_size;
  out->st_blksize = in->st_blksize;
  out->st_blocks = in->st_blocks;
  out->st_atim.tv_sec = in->st_atime;
  out->st_atim.tv_nsec = in->st_atime_nsec;
  out->st_mtim.tv_sec = in->st_mtime;
  out->st_mtim.tv_nsec = in->st_mtime_nsec;
  out->st_ctim.tv_sec = in->st_ctime;
  out->st_ctim.tv_nsec = in->st_ctime_nsec;
  out->__reserved[0] = 0;
  out->__reserved[1] = 0;
  out->__reserved[2] = 0;
}

long nelibc_aarch64_fstat(unsigned long fd, unsigned long out) {
  struct kernel_stat info;
  if (!out) return -EFAULT;
  long status = nelibc_raw_syscall3(__NR_fstat, fd, (unsigned long)&info, 0);
  if (!status) store_stat((struct stat *)out, &info);
  return status;
}

long nelibc_aarch64_fstatat(unsigned long dir, unsigned long path,
                            unsigned long out, unsigned long flags) {
  struct kernel_stat info;
  if (!out) return -EFAULT;
  long status = nelibc_raw_syscall6(__NR_newfstatat, dir, path,
                                     (unsigned long)&info, flags, 0, 0);
  if (!status) store_stat((struct stat *)out, &info);
  return status;
}
