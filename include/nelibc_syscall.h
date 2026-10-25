#ifndef NELIBC_SYSCALL_H
#define NELIBC_SYSCALL_H

#include <stddef.h>
#include <stdint.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/uio.h>
#include <signal.h>
#include <fcntl.h>
#include <poll.h>
#include <sys/select.h>
#include <time.h>
#include <sys/time.h>
#include <sys/random.h>
#include <sys/mman.h>
#include <sys/socket.h>
#include <sys/utsname.h>
#include <sys/resource.h>
#include <sys/times.h>
#include <sys/wait.h>
#include <sched.h>
#include <sys/sendfile.h>
#include <sys/xattr.h>
#include <sys/mount.h>

struct statx;

/* Fixed Linux UAPI layout used by the xattrat syscalls (kernel >= 6.17). */
struct nelibc_xattr_args {
  uint64_t value;
  uint32_t size;
  uint32_t flags;
};

/* Fixed Linux file_attr UAPI layout used by file_getattr/file_setattr. */
struct nelibc_file_attr {
  uint64_t fa_xflags;
  uint32_t fa_extsize;
  uint32_t fa_nextents;
  uint32_t fa_projid;
  uint32_t fa_cowextsize;
};
#define NELIBC_FILE_ATTR_SIZE 24

/* Fixed Linux mount_attr UAPI layout used by open_tree_attr. */
struct nelibc_mount_attr {
  uint64_t attr_set;
  uint64_t attr_clr;
  uint64_t propagation;
  uint64_t userns_fd;
};
#define NELIBC_MOUNT_ATTR_SIZE 32

/* musl headers may predate the mount_setattr UAPI declarations. */
#ifndef MOUNT_ATTR_SIZE_VER0
struct mount_attr {
  uint64_t attr_set;
  uint64_t attr_clr;
  uint64_t propagation;
  uint64_t userns_fd;
};
#define MOUNT_ATTR_SIZE_VER0 32
#endif

/* Linux mount API flags absent from older glibc, musl, and TinyCC headers. */
#ifndef FSMOUNT_CLOEXEC
#define FSMOUNT_CLOEXEC 0x00000001
#endif
#ifndef FSMOUNT_NAMESPACE
#define FSMOUNT_NAMESPACE 0x00000002
#endif
#ifndef MOUNT_ATTR_RDONLY
#define MOUNT_ATTR_RDONLY 0x00000001
#endif
#ifndef MOUNT_ATTR_NOSUID
#define MOUNT_ATTR_NOSUID 0x00000002
#endif
#ifndef MOUNT_ATTR_NODEV
#define MOUNT_ATTR_NODEV 0x00000004
#endif
#ifndef MOUNT_ATTR_NOEXEC
#define MOUNT_ATTR_NOEXEC 0x00000008
#endif
#ifndef MOUNT_ATTR__ATIME
#define MOUNT_ATTR__ATIME 0x00000070
#endif
#ifndef MOUNT_ATTR_RELATIME
#define MOUNT_ATTR_RELATIME 0x00000000
#endif
#ifndef MOUNT_ATTR_NOATIME
#define MOUNT_ATTR_NOATIME 0x00000010
#endif
#ifndef MOUNT_ATTR_STRICTATIME
#define MOUNT_ATTR_STRICTATIME 0x00000020
#endif
#ifndef MOUNT_ATTR_NODIRATIME
#define MOUNT_ATTR_NODIRATIME 0x00000080
#endif
#ifndef MOUNT_ATTR_IDMAP
#define MOUNT_ATTR_IDMAP 0x00100000
#endif
#ifndef MOUNT_ATTR_NOSYMFOLLOW
#define MOUNT_ATTR_NOSYMFOLLOW 0x00200000
#endif
#ifndef MOVE_MOUNT_F_SYMLINKS
#define MOVE_MOUNT_F_SYMLINKS 0x00000001
#endif
#ifndef MOVE_MOUNT_F_AUTOMOUNTS
#define MOVE_MOUNT_F_AUTOMOUNTS 0x00000002
#endif
#ifndef MOVE_MOUNT_F_EMPTY_PATH
#define MOVE_MOUNT_F_EMPTY_PATH 0x00000004
#endif
#ifndef MOVE_MOUNT_T_SYMLINKS
#define MOVE_MOUNT_T_SYMLINKS 0x00000010
#endif
#ifndef MOVE_MOUNT_T_AUTOMOUNTS
#define MOVE_MOUNT_T_AUTOMOUNTS 0x00000020
#endif
#ifndef MOVE_MOUNT_T_EMPTY_PATH
#define MOVE_MOUNT_T_EMPTY_PATH 0x00000040
#endif
#ifndef MOVE_MOUNT_SET_GROUP
#define MOVE_MOUNT_SET_GROUP 0x00000100
#endif
#ifndef MOVE_MOUNT_BENEATH
#define MOVE_MOUNT_BENEATH 0x00000200
#endif
#ifndef FSPICK_CLOEXEC
#define FSPICK_CLOEXEC 0x00000001
#endif
#ifndef FSPICK_SYMLINK_NOFOLLOW
#define FSPICK_SYMLINK_NOFOLLOW 0x00000002
#endif
#ifndef FSPICK_NO_AUTOMOUNT
#define FSPICK_NO_AUTOMOUNT 0x00000004
#endif
#ifndef FSPICK_EMPTY_PATH
#define FSPICK_EMPTY_PATH 0x00000008
#endif
#ifndef FSCONFIG_SET_FLAG
#define FSCONFIG_SET_FLAG 0
#endif
#ifndef FSCONFIG_SET_STRING
#define FSCONFIG_SET_STRING 1
#endif
#ifndef FSCONFIG_SET_BINARY
#define FSCONFIG_SET_BINARY 2
#endif
#ifndef FSCONFIG_SET_PATH
#define FSCONFIG_SET_PATH 3
#endif
#ifndef FSCONFIG_SET_PATH_EMPTY
#define FSCONFIG_SET_PATH_EMPTY 4
#endif
#ifndef FSCONFIG_SET_FD
#define FSCONFIG_SET_FD 5
#endif
#ifndef FSCONFIG_CMD_CREATE
#define FSCONFIG_CMD_CREATE 6
#endif
#ifndef FSCONFIG_CMD_RECONFIGURE
#define FSCONFIG_CMD_RECONFIGURE 7
#endif
#ifndef FSCONFIG_CMD_CREATE_EXCL
#define FSCONFIG_CMD_CREATE_EXCL 8
#endif
#ifndef FSOPEN_CLOEXEC
#define FSOPEN_CLOEXEC 0x00000001
#endif
#ifndef OPEN_TREE_CLONE
#define OPEN_TREE_CLONE 0x00000001
#endif
#ifndef OPEN_TREE_NAMESPACE
#define OPEN_TREE_NAMESPACE 0x00000002
#endif
#ifndef OPEN_TREE_DROP_MNTNS_MOUNTS
#define OPEN_TREE_DROP_MNTNS_MOUNTS 0x00000004
#endif
#ifndef OPEN_TREE_CLOEXEC
#define OPEN_TREE_CLOEXEC O_CLOEXEC
#endif
#ifndef STATMOUNT_BY_FD
#define STATMOUNT_BY_FD 0x00000001U
#endif
#ifndef STATMOUNT_SB_BASIC
#define STATMOUNT_SB_BASIC 0x00000001U
#endif
#ifndef STATMOUNT_MNT_BASIC
#define STATMOUNT_MNT_BASIC 0x00000002U
#endif
#ifndef STATMOUNT_PROPAGATE_FROM
#define STATMOUNT_PROPAGATE_FROM 0x00000004U
#endif
#ifndef STATMOUNT_MNT_ROOT
#define STATMOUNT_MNT_ROOT 0x00000008U
#endif
#ifndef STATMOUNT_MNT_POINT
#define STATMOUNT_MNT_POINT 0x00000010U
#endif
#ifndef STATMOUNT_FS_TYPE
#define STATMOUNT_FS_TYPE 0x00000020U
#endif
#ifndef STATMOUNT_MNT_NS_ID
#define STATMOUNT_MNT_NS_ID 0x00000040U
#endif
#ifndef STATMOUNT_MNT_OPTS
#define STATMOUNT_MNT_OPTS 0x00000080U
#endif
#ifndef STATMOUNT_FS_SUBTYPE
#define STATMOUNT_FS_SUBTYPE 0x00000100U
#endif
#ifndef STATMOUNT_SB_SOURCE
#define STATMOUNT_SB_SOURCE 0x00000200U
#endif
#ifndef STATMOUNT_OPT_ARRAY
#define STATMOUNT_OPT_ARRAY 0x00000400U
#endif
#ifndef STATMOUNT_OPT_SEC_ARRAY
#define STATMOUNT_OPT_SEC_ARRAY 0x00000800U
#endif
#ifndef STATMOUNT_SUPPORTED_MASK
#define STATMOUNT_SUPPORTED_MASK 0x00001000U
#endif
#ifndef STATMOUNT_MNT_UIDMAP
#define STATMOUNT_MNT_UIDMAP 0x00002000U
#endif
#ifndef STATMOUNT_MNT_GIDMAP
#define STATMOUNT_MNT_GIDMAP 0x00004000U
#endif
#ifndef LISTMOUNT_REVERSE
#define LISTMOUNT_REVERSE 0x00000001U
#endif
#ifndef LSMT_ROOT
#define LSMT_ROOT UINT64_MAX
#endif

/* Linux mount enumeration UAPI layouts (kernel 6.8+). */
struct nelibc_mnt_id_req {
  uint32_t size;
  union {
    uint32_t mnt_ns_fd;
    uint32_t mnt_fd;
  };
  uint64_t mnt_id;
  uint64_t param;
  uint64_t mnt_ns_id;
};
#define NELIBC_MNT_ID_REQ_SIZE 32

struct nelibc_statmount {
  uint32_t size;
  uint32_t mnt_opts;
  uint64_t mask;
  uint32_t sb_dev_major;
  uint32_t sb_dev_minor;
  uint64_t sb_magic;
  uint32_t sb_flags;
  uint32_t fs_type;
  uint64_t mnt_id;
  uint64_t mnt_parent_id;
  uint32_t mnt_id_old;
  uint32_t mnt_parent_id_old;
  uint64_t mnt_attr;
  uint64_t mnt_propagation;
  uint64_t mnt_peer_group;
  uint64_t mnt_master;
  uint64_t propagate_from;
  uint32_t mnt_root;
  uint32_t mnt_point;
  uint64_t mnt_ns_id;
  uint32_t fs_subtype;
  uint32_t sb_source;
  uint32_t opt_num;
  uint32_t opt_array;
  uint32_t opt_sec_num;
  uint32_t opt_sec_array;
  uint32_t mnt_uidmap_num;
  uint32_t mnt_uidmap;
  uint32_t mnt_gidmap_num;
  uint32_t mnt_gidmap;
  uint64_t supported_mask;
  uint64_t __spare2[43];
  char str[];
};
#define NELIBC_STATMOUNT_SIZE 512

#ifndef RWF_HIPRI
#define RWF_HIPRI 0x00000001
#endif
#ifndef RWF_DSYNC
#define RWF_DSYNC 0x00000002
#endif
#ifndef RWF_SYNC
#define RWF_SYNC 0x00000004
#endif
#ifndef RWF_NOWAIT
#define RWF_NOWAIT 0x00000008
#endif
#ifndef RWF_APPEND
#define RWF_APPEND 0x00000010
#endif

/* Linux x86_64 only. Returns the raw kernel result, including negative errno. */
long nelibc_linux_write(int fd, const void *buf, size_t count);
int mount(const char *source, const char *target, const char *filesystem,
          unsigned long mountflags, const void *data);
int umount(const char *target);
int umount2(const char *target, int flags);
int pivot_root(const char *new_root, const char *put_old);
int unshare(int flags);
int setns(int fd, int nstype);
int sethostname(const char *name, size_t length);
int setdomainname(const char *name, size_t length);
int gethostname(char *name, size_t length);
int getdomainname(char *name, size_t length);

/* POSIX process query implemented with the Linux x86_64 getpid syscall. */
int getpid(void);
int getdtablesize(void);
pid_t fork(void);
#if defined(_DEFAULT_SOURCE) || defined(_GNU_SOURCE) || defined(NELIBC_STANDALONE)
int usleep(unsigned int usec);
unsigned int ualarm(unsigned int usecs, unsigned int interval);
#endif
int execve(const char *pathname, char *const argv[], char *const envp[]);
int fexecve(int fd, char *const argv[], char *const envp[]);
int execv(const char *pathname, char *const argv[]);
int execvp(const char *file, char *const argv[]);
#if defined(_GNU_SOURCE) || defined(NELIBC_STANDALONE)
int execvpe(const char *file, char *const argv[], char *const envp[]);
#endif
int execveat(int dirfd, const char *pathname, char *const argv[], char *const envp[], int flags);
void _exit(int status);

/* POSIX socket interfaces implemented with Linux x86_64 syscalls. */
int socket(int domain, int type, int protocol);
int socketpair(int domain, int type, int protocol, int sockets[2]);
int bind(int socket, const struct sockaddr *address, socklen_t address_length);
int connect(int socket, const struct sockaddr *address, socklen_t address_length);
int listen(int socket, int backlog);
int accept(int socket, struct sockaddr *address, socklen_t *address_length);
int accept4(int socket, struct sockaddr *address, socklen_t *address_length, int flags);
ssize_t send(int socket, const void *buffer, size_t length, int flags);
ssize_t recv(int socket, void *buffer, size_t length, int flags);
ssize_t sendto(int socket, const void *buffer, size_t length, int flags,
               const struct sockaddr *address, socklen_t address_length);
ssize_t recvfrom(int socket, void *buffer, size_t length, int flags,
                 struct sockaddr *address, socklen_t *address_length);
ssize_t sendmsg(int socket, const struct msghdr *message, int flags);
ssize_t recvmsg(int socket, struct msghdr *message, int flags);
int shutdown(int socket, int how);
int getsockname(int socket, struct sockaddr *address, socklen_t *address_length);
int getpeername(int socket, struct sockaddr *address, socklen_t *address_length);
int setsockopt(int socket, int level, int option_name, const void *option_value,
               socklen_t option_length);
int getsockopt(int socket, int level, int option_name, void *option_value,
               socklen_t *option_length);

/* POSIX descriptor I/O implemented with Linux x86_64 syscalls. */
ssize_t read(int fd, void *buf, size_t count);
ssize_t write(int fd, const void *buf, size_t count);
int close(int fd);

/* Linux-specific descriptor range operation (kernel >= 5.9). */
#ifndef CLOSE_RANGE_UNSHARE
#define CLOSE_RANGE_UNSHARE (1U << 1)
#endif
#ifndef CLOSE_RANGE_CLOEXEC
#define CLOSE_RANGE_CLOEXEC (1U << 2)
#endif
int close_range(unsigned int first, unsigned int last, int flags);

/* POSIX descriptor pair creation implemented with the Linux x86_64 pipe syscall. */
int pipe(int pipefd[2]);
/* Linux-specific: atomically create a pipe and apply flags such as O_CLOEXEC/O_NONBLOCK. */
int pipe2(int pipefd[2], int flags);
int dup(int oldfd);
int dup2(int oldfd, int newfd);
int dup3(int oldfd, int newfd, int flags); /* Linux-specific. */
ssize_t readv(int fd, const struct iovec *iov, int iovcnt);
ssize_t writev(int fd, const struct iovec *iov, int iovcnt);
int poll(struct pollfd *fds, nfds_t nfds, int timeout);
#if defined(_GNU_SOURCE) || defined(NELIBC_STANDALONE)
int ppoll(struct pollfd *fds, nfds_t nfds, const struct timespec *timeout,
          const sigset_t *sigmask);
#endif
int select(int nfds, fd_set *readfds, fd_set *writefds, fd_set *exceptfds,
           struct timeval *timeout);
int nanosleep(const struct timespec *requested, struct timespec *remaining);
int clock_nanosleep(clockid_t clock_id, int flags,
                    const struct timespec *requested, struct timespec *remaining);
unsigned int sleep(unsigned int seconds);
unsigned int alarm(unsigned int seconds);
int pause(void);
int clock_gettime(clockid_t clock_id, struct timespec *time_value);
int clock_getres(clockid_t clock_id, struct timespec *resolution);
ssize_t getrandom(void *buffer, size_t length, unsigned int flags);
int getentropy(void *buffer, size_t length);
ssize_t copy_file_range(int fd_in, off_t *off_in, int fd_out, off_t *off_out, size_t length, unsigned int flags);
ssize_t splice(int fd_in, off_t *off_in, int fd_out, off_t *off_out, size_t length, unsigned int flags);
ssize_t tee(int fd_in, int fd_out, size_t length, unsigned int flags);
ssize_t vmsplice(int fd, const struct iovec *iov, size_t iovcnt, unsigned int flags);
int fsetxattr(int fd, const char *name, const void *value, size_t size, int flags);
ssize_t fgetxattr(int fd, const char *name, void *value, size_t size);
ssize_t flistxattr(int fd, char *list, size_t size);
int fremovexattr(int fd, const char *name);
int setxattr(const char *pathname, const char *name, const void *value, size_t size, int flags);
ssize_t getxattr(const char *pathname, const char *name, void *value, size_t size);
ssize_t listxattr(const char *pathname, char *list, size_t size);
int removexattr(const char *pathname, const char *name);
int lsetxattr(const char *pathname, const char *name, const void *value, size_t size, int flags);
ssize_t lgetxattr(const char *pathname, const char *name, void *value, size_t size);
ssize_t llistxattr(const char *pathname, char *list, size_t size);
int lremovexattr(const char *pathname, const char *name);
int setxattrat(int dirfd, const char *pathname, unsigned int at_flags, const char *name,
               const struct nelibc_xattr_args *args, size_t args_size);
ssize_t getxattrat(int dirfd, const char *pathname, unsigned int at_flags, const char *name,
                   struct nelibc_xattr_args *args, size_t args_size);
ssize_t listxattrat(int dirfd, const char *pathname, unsigned int at_flags, char *list, size_t size);
int removexattrat(int dirfd, const char *pathname, unsigned int at_flags, const char *name);
int file_getattr(int dirfd, const char *pathname, struct nelibc_file_attr *attributes,
                 size_t attributes_size, unsigned int at_flags);
int file_setattr(int dirfd, const char *pathname, const struct nelibc_file_attr *attributes,
                 size_t attributes_size, unsigned int at_flags);
int open_tree_attr(int dirfd, const char *pathname, unsigned int flags,
                   const struct nelibc_mount_attr *attributes, size_t attributes_size);
int mount_setattr(int dirfd, const char *pathname, unsigned int flags,
                  struct mount_attr *attributes, size_t attributes_size);
int open_tree(int dirfd, const char *pathname, unsigned int flags);
int move_mount(int from_dirfd, const char *from_path, int to_dirfd, const char *to_path,
               unsigned int flags);
int fsmount(int fsfd, unsigned int flags, unsigned int attr_flags);
int fsopen(const char *fsname, unsigned int flags);
int fsconfig(int fd, unsigned int command, const char *key, const void *value, int aux);
int fspick(int dirfd, const char *pathname, unsigned int flags);
int statmount(struct nelibc_mnt_id_req *request, struct nelibc_statmount *buffer,
              size_t buffer_size, unsigned int flags);
long listmount(struct nelibc_mnt_id_req *request, uint64_t *mount_ids,
               size_t mount_id_count, unsigned int flags);
void *mmap(void *address, size_t length, int protection, int flags, int fd, off_t offset);
void *mremap(void *old_address, size_t old_size, size_t new_size, int flags, ...);
int mprotect(void *address, size_t length, int protection);
int msync(void *address, size_t length, int flags);
int munmap(void *address, size_t length);
int madvise(void *address, size_t length, int advice);
mode_t umask(mode_t mask);
int uname(struct utsname *name);

/* Linux positioned I/O implemented with Linux x86_64 syscalls. */
ssize_t pread(int fd, void *buf, size_t count, off_t offset);
ssize_t pwrite(int fd, const void *buf, size_t count, off_t offset);
ssize_t preadv(int fd, const struct iovec *iov, int iovcnt, off_t offset);
ssize_t pwritev(int fd, const struct iovec *iov, int iovcnt, off_t offset);
ssize_t preadv2(int fd, const struct iovec *iov, int iovcnt, off_t offset, int flags);
ssize_t pwritev2(int fd, const struct iovec *iov, int iovcnt, off_t offset, int flags);

/* 64-bit aliases share off_t's ABI on the supported x86-64 target. */
#if defined(_LARGEFILE64_SOURCE) || defined(_GNU_SOURCE)
void *mmap64(void *address, size_t length, int protection, int flags, int fd,
             off_t offset);
ssize_t pread64(int fd, void *buf, size_t count, off_t offset);
ssize_t pwrite64(int fd, const void *buf, size_t count, off_t offset);
ssize_t preadv64(int fd, const struct iovec *iov, int iovcnt, off_t offset);
ssize_t pwritev64(int fd, const struct iovec *iov, int iovcnt, off_t offset);
off_t lseek64(int fd, off_t offset, int whence);
int ftruncate64(int fd, off_t length);
#endif

/* POSIX seek implemented with the Linux x86_64 lseek syscall. */
off_t lseek(int fd, off_t offset, int whence);
int ftruncate(int fd, off_t length);
int truncate(const char *pathname, off_t length);
int fsync(int fd);
int fdatasync(int fd);
void sync(void);
int syncfs(int fd);

/* POSIX variadic entry point; pass mode when flags include O_CREAT or O_TMPFILE. */
int openat(int dirfd, const char *pathname, int flags, ...);

/* POSIX relative or absolute path removal implemented with unlinkat syscall. */
int unlinkat(int dirfd, const char *pathname, int flags);
int unlink(const char *pathname);
int rmdir(const char *pathname);
int mkdirat(int dirfd, const char *pathname, mode_t mode);
int mkdir(const char *pathname, mode_t mode);
int mknod(const char *pathname, mode_t mode, dev_t device);
int mknodat(int dirfd, const char *pathname, mode_t mode, dev_t device);
int mkfifoat(int dirfd, const char *pathname, mode_t mode);
int mkfifo(const char *pathname, mode_t mode);
int renameat(int olddirfd, const char *oldpath, int newdirfd, const char *newpath);
int rename(const char *oldpath, const char *newpath);
int linkat(int olddirfd, const char *oldpath, int newdirfd, const char *newpath, int flags);
int link(const char *oldpath, const char *newpath);
int fchownat(int dirfd, const char *pathname, uid_t owner, gid_t group, int flags);
int fchown(int fd, uid_t owner, gid_t group);
int chown(const char *pathname, uid_t owner, gid_t group);
int lchown(const char *pathname, uid_t owner, gid_t group);
int futimens(int fd, const struct timespec timestamps[2]);
int faccessat(int dirfd, const char *pathname, int mode, int flags);
int faccessat2(int dirfd, const char *pathname, int mode, int flags);
int access(const char *pathname, int mode);
int isatty(int fd);
int tcdrain(int fd);
#if defined(__GLIBC__) || defined(NELIBC_STANDALONE)
int ioctl(int fd, unsigned long request, ...);
#else
int ioctl(int fd, int request, ...);
#endif

/* POSIX metadata lookup using the Linux x86_64 struct stat syscall ABI. */
int fstatat(int dirfd, const char *pathname, struct stat *statbuf, int flags);
int statx(int dirfd, const char *pathname, int flags, unsigned int mask, struct statx *statbuf);
int fstat(int fd, struct stat *statbuf);
int stat(const char *pathname, struct stat *statbuf);
int lstat(const char *pathname, struct stat *statbuf);
int chmod(const char *pathname, mode_t mode);
int fchmod(int fd, mode_t mode);
int fchmodat(int dirfd, const char *pathname, mode_t mode, int flags);
int symlinkat(const char *target, int newdirfd, const char *linkpath);
int symlink(const char *target, const char *linkpath);
ssize_t readlinkat(int dirfd, const char *pathname, char *buf, size_t bufsiz);
ssize_t readlink(const char *pathname, char *buf, size_t bufsiz);
char *getcwd(char *buf, size_t size);
#if defined(_GNU_SOURCE) || defined(NELIBC_STANDALONE)
char *get_current_dir_name(void);
#endif
void swab(const void *restrict from, void *restrict to, ssize_t count);
int chdir(const char *pathname);
int fchdir(int fd);
uid_t getuid(void);
uid_t geteuid(void);
gid_t getgid(void);
gid_t getegid(void);
int setuid(uid_t uid);
int seteuid(uid_t euid);
int setgid(gid_t gid);
int setegid(gid_t egid);
int getresuid(uid_t *real_uid, uid_t *effective_uid, uid_t *saved_uid);
int setresuid(uid_t real_uid, uid_t effective_uid, uid_t saved_uid);
int setreuid(uid_t real_uid, uid_t effective_uid);
int getresgid(gid_t *real_gid, gid_t *effective_gid, gid_t *saved_gid);
int setresgid(gid_t real_gid, gid_t effective_gid, gid_t saved_gid);
int setregid(gid_t real_gid, gid_t effective_gid);
int getgroups(int size, gid_t groups[]);
int setgroups(size_t size, const gid_t groups[]);
pid_t getppid(void);
pid_t gettid(void); /* Linux-specific thread ID. */
pid_t getpgid(pid_t pid);
pid_t getpgrp(void);
int setpgid(pid_t pid, pid_t pgid);
pid_t getsid(pid_t pid);
pid_t setsid(void);
int kill(pid_t pid, int signal);
int raise(int signal);
/* Linux x86_64 kernel signal syscalls consume only the first 8 bytes of sigset_t. */
int sigprocmask(int how, const sigset_t *set, sigset_t *oldset);
int sigpending(sigset_t *set);
int sigismember(const sigset_t *set, int signal);
int sigemptyset(sigset_t *set);
int sigaddset(sigset_t *set, int signal);
int sigdelset(sigset_t *set, int signal);
int sigfillset(sigset_t *set);
int sigsuspend(const sigset_t *mask);
int sigwait(const sigset_t *set, int *signal);
int sigwaitinfo(const sigset_t *set, siginfo_t *info);
int sigtimedwait(const sigset_t *set, siginfo_t *info,
                 const struct timespec *timeout);
struct sigaction;
int sigaction(int signal, const struct sigaction *action,
              struct sigaction *old_action);

#endif
