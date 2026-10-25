#define _GNU_SOURCE
#include <sys/stat.h>
#include <locale.h>
#include <signal.h>
#include <time.h>
#include <fcntl.h>
#include <poll.h>
#include <sys/poll.h>
#include <sys/time.h>
#include <sys/times.h>
#include <sys/uio.h>
#include <sys/un.h>
#include <sys/utsname.h>
#include <unistd.h>
#include <dirent.h>
#include <sys/mman.h>
#include <sys/resource.h>
#include <sys/random.h>
#include <sys/ioctl.h>
#include <sys/select.h>
#include <sys/socket.h>
#include <sys/wait.h>
#include <stdio.h>
#include <stdlib.h>
#include <sched.h>
#include <wchar.h>

_Static_assert(FSOPEN_CLOEXEC == 1, "Linux FSOPEN_CLOEXEC value");
_Static_assert(FSCONFIG_CMD_CREATE == 6, "Linux FSCONFIG_CMD_CREATE value");
_Static_assert(FSMOUNT_CLOEXEC == 1, "Linux FSMOUNT_CLOEXEC value");
_Static_assert(OPEN_TREE_CLONE == 1, "Linux OPEN_TREE_CLONE value");
_Static_assert(MOVE_MOUNT_F_EMPTY_PATH == 4, "Linux MOVE_MOUNT_F_EMPTY_PATH value");
_Static_assert(FSPICK_EMPTY_PATH == 8, "Linux FSPICK_EMPTY_PATH value");
_Static_assert(_PC_FILESIZEBITS == 13, "Linux _PC_FILESIZEBITS value");
_Static_assert(_PC_2_SYMLINKS == 20, "Linux _PC_2_SYMLINKS value");
_Static_assert(PROT_READ == 1 && PROT_WRITE == 2, "Linux mmap protection values");
_Static_assert(PROT_GROWSDOWN == 0x01000000 && PROT_GROWSUP == 0x02000000,
               "Linux mmap stack growth protections");
_Static_assert(MAP_PRIVATE == 2 && MAP_ANONYMOUS == 0x20, "Linux mmap flag values");
_Static_assert(MAP_DROPPABLE == 8 && MAP_HUGE_2MB == (21 << 26) &&
               MAP_HUGE_1GB == (30 << 26), "Linux mmap mapping flags");
_Static_assert(MREMAP_MAYMOVE == 1 && MREMAP_FIXED == 2, "Linux mremap flag values");
_Static_assert(MS_ASYNC == 1 && MS_INVALIDATE == 2 && MS_SYNC == 4 &&
               MCL_CURRENT == 1 && MCL_FUTURE == 2 && MCL_ONFAULT == 4,
               "Linux memory synchronization and lock flags");
_Static_assert(MADV_DONTNEED == 4 && MADV_COLLAPSE == 25, "Linux madvise values");
_Static_assert(MADV_DONTNEED_LOCKED == 24 && MADV_HWPOISON == 100 &&
               MADV_GUARD_INSTALL == 102 && MADV_GUARD_REMOVE == 103,
               "Linux madvise extension values");
_Static_assert(WNOHANG == 1 && WUNTRACED == 2, "Linux wait option values");
_Static_assert(WIFEXITED(0x2a00) && WEXITSTATUS(0x2a00) == 42,
               "wait exit status macros");
_Static_assert(WIFSIGNALED(9) && WTERMSIG(9) == 9,
               "wait signal status macros");
_Static_assert(AT_FDCWD == -100, "Linux AT_FDCWD value");
_Static_assert(POLLIN == 0x001 && POLLOUT == 0x004, "poll event flag values");
_Static_assert(offsetof(struct pollfd, fd) == 0 &&
               offsetof(struct pollfd, events) == 4 &&
               offsetof(struct pollfd, revents) == 6 &&
               sizeof(struct pollfd) == 8,
               "Linux pollfd ABI layout");
_Static_assert(offsetof(struct timeval, tv_usec) == sizeof(long),
               "x86_64 timeval field layout");
_Static_assert(sizeof(struct timezone) == 8,
               "x86_64 timezone field layout");
_Static_assert(sizeof(div_t) == 2 * sizeof(int) &&
               sizeof(ldiv_t) == 2 * sizeof(long) &&
               sizeof(lldiv_t) == 2 * sizeof(long long),
               "standard division result layouts");
_Static_assert(EXIT_SUCCESS == 0 && EXIT_FAILURE == 1 &&
               RAND_MAX == 2147483647,
               "standard standalone stdlib constants");
_Static_assert(LC_CTYPE == 0 && LC_NUMERIC == 1 && LC_ALL == 6,
               "standard locale categories");
_Static_assert(offsetof(struct lconv, int_curr_symbol) == 24 &&
               offsetof(struct lconv, int_p_cs_precedes) == 88 &&
               sizeof(struct lconv) == 96,
               "x86_64 locale conversion structure layout");
static char *(*volatile public_setlocale)(int, const char *) = setlocale;
static struct lconv *(*volatile public_localeconv)(void) = localeconv;
_Static_assert(sizeof(sigset_t) == 128 &&
               offsetof(struct sigaction, sa_mask) == 8 &&
               offsetof(struct sigaction, sa_flags) == 136 &&
               offsetof(struct sigaction, sa_restorer) == 144 &&
               sizeof(struct sigaction) == 152,
               "x86_64 signal ABI layouts");
_Static_assert(SIGUSR1 == 10 && SIGCHLD == 17 && SIG_BLOCK == 0 &&
               SIG_SETMASK == 2,
               "Linux signal constants");
_Static_assert(sizeof(cpu_set_t) == 128 && CPU_SETSIZE == 1024,
               "Linux x86_64 CPU affinity set ABI");
#ifdef NELIBC_STANDALONE
_Static_assert(SA_RESTORER == 0x04000000, "Linux signal restorer flag");
#endif
_Static_assert(sizeof(struct timespec) == 16 &&
               offsetof(struct timespec, tv_nsec) == sizeof(time_t),
               "x86_64 timespec field layout");
_Static_assert(CLOCK_REALTIME == 0 && CLOCK_MONOTONIC == 1 && TIMER_ABSTIME == 1,
               "Linux clock identifiers and timer flags");
_Static_assert(sizeof(pid_t) == 4 && sizeof(uid_t) == 4 &&
               sizeof(gid_t) == 4 && sizeof(off_t) == 8 &&
               sizeof(ssize_t) == 8,
               "Linux standalone system types");
_Static_assert(RLIMIT_CPU == 0 && RLIMIT_NOFILE == 7,
               "Linux resource limit selectors");
_Static_assert(RUSAGE_SELF == 0 && PRIO_PROCESS == 0,
               "Linux resource query selectors");
#ifdef NELIBC_STANDALONE
_Static_assert(sizeof(struct rlimit) == 16 &&
               offsetof(struct rlimit, rlim_max) == sizeof(unsigned long),
               "Linux standalone rlimit layout");
_Static_assert(sizeof(struct rusage) == 144 &&
               offsetof(struct rusage, ru_maxrss) == 32 &&
               offsetof(struct rusage, ru_nivcsw) == 136,
               "Linux standalone rusage layout");
#endif
_Static_assert(GRND_NONBLOCK == 1 && GRND_RANDOM == 2 && GRND_INSECURE == 4,
               "Linux getrandom flags");
_Static_assert(FIONREAD == 0x541B && TCGETS == 0x5401,
               "Linux ioctl request values");
#if defined(__GLIBC__) || defined(NELIBC_STANDALONE)
static int (*ioctl_signature)(int, unsigned long, ...) = ioctl;
#else
static int (*ioctl_signature)(int, int, ...) = ioctl;
#endif
_Static_assert(offsetof(struct stat, st_size) == 48,
               "x86_64 stat st_size offset");
_Static_assert(offsetof(struct stat, st_atim) == 72 && sizeof(struct stat) == 144,
               "x86_64 stat timestamp and total layout");
_Static_assert(S_IFREG == 0100000 && S_ISREG(S_IFREG | 0644),
               "Linux regular-file mode macros");
_Static_assert(offsetof(struct statx, stx_ino) == 32 &&
               offsetof(struct statx, stx_atime) == 64 &&
               sizeof(struct statx) == 256,
               "Linux statx UAPI layout");
#ifdef NELIBC_STANDALONE
_Static_assert(offsetof(struct statx, stx_mnt_id) == 144,
               "Linux standalone statx mount id offset");
#endif
_Static_assert(offsetof(struct tms, tms_stime) == sizeof(clock_t),
               "x86_64 tms field layout");
_Static_assert(FD_SETSIZE == 1024 && sizeof(fd_set) == 128,
               "x86_64 fd_set layout");
_Static_assert(sizeof(struct iovec) == 16 &&
               offsetof(struct iovec, iov_len) == sizeof(void *),
               "x86_64 iovec layout");
_Static_assert(sizeof(wchar_t) == 4 && sizeof(wint_t) == 4,
               "x86_64 wide-character types");
_Static_assert(WEOF == 0xffffffffU, "wide-character EOF value");
#ifdef NELIBC_STANDALONE
_Static_assert(sizeof(mbstate_t) == 128,
               "Nelibc standalone wide-character state");
#endif
_Static_assert(offsetof(struct utsname, machine) == 260 &&
               sizeof(struct utsname) == 390,
               "Linux x86-64 utsname layout");
_Static_assert(offsetof(struct sockaddr_un, sun_path) == 2 &&
               sizeof(struct sockaddr_un) == 110 && UNIX_PATH_MAX == 108,
               "Linux x86-64 sockaddr_un layout");
_Static_assert(AF_UNIX == 1 && AF_INET == 2, "Linux socket family values");
_Static_assert(SOCK_STREAM == 1 && SOCK_DGRAM == 2, "Linux socket type values");
_Static_assert(SOCK_CLOEXEC == 0x80000 && SOCK_NONBLOCK == 0x800,
               "Linux socket type flags");
_Static_assert(sizeof(struct sockaddr) == 16 &&
               sizeof(struct sockaddr_storage) == 128,
               "Linux socket address layouts");
_Static_assert(sizeof(struct msghdr) == 56 &&
               offsetof(struct msghdr, msg_iov) == 16 &&
               offsetof(struct msghdr, msg_control) == 32 &&
               sizeof(struct cmsghdr) == 16,
               "Linux message header layouts");
_Static_assert(offsetof(struct dirent, d_name) == 19, "Linux dirent64 name offset");
_Static_assert(sizeof(struct dirent) == 280, "Linux x86_64 dirent layout");
_Static_assert(MOUNT_ATTR_RDONLY == 1, "Linux MOUNT_ATTR_RDONLY value");
_Static_assert(sizeof(struct nelibc_mnt_id_req) == NELIBC_MNT_ID_REQ_SIZE, "Linux mnt_id_req layout");
_Static_assert(sizeof(struct nelibc_statmount) == NELIBC_STATMOUNT_SIZE, "Linux statmount layout");
_Static_assert(sizeof(sigset_t) == 128, "glibc/musl x86_64 sigset_t layout");
_Static_assert(sizeof(struct sigaction) == 152, "glibc/musl x86_64 sigaction layout");
_Static_assert(offsetof(struct sigaction, sa_mask) == 8, "sigaction mask offset");
_Static_assert(offsetof(struct sigaction, sa_flags) == 136, "sigaction flags offset");
_Static_assert(offsetof(struct sigaction, sa_restorer) == 144, "sigaction restorer offset");

static ssize_t (*volatile public_preadv2)(int, const struct iovec *, int, off_t, int) = preadv2;
static int (*volatile public_fcntl)(int, int, ...) = fcntl;
static int (*volatile public_openat)(int, const char *, int, ...) = openat;
static int (*volatile public_poll)(struct pollfd *, nfds_t, int) = poll;
static int (*volatile public_select)(int, fd_set *, fd_set *, fd_set *,
    struct timeval *) = select;
static ssize_t (*volatile public_readv)(int, const struct iovec *, int) = readv;
static ssize_t (*volatile public_writev)(int, const struct iovec *, int) = writev;
static wchar_t *(*volatile public_wmemcpy)(wchar_t *, const wchar_t *, size_t) = wmemcpy;
static size_t (*volatile public_mbsrtowcs)(wchar_t *, const char **, size_t,
    mbstate_t *) = mbsrtowcs;
static ssize_t (*volatile public_preadv)(int, const struct iovec *, int, off_t) = preadv;
static ssize_t (*volatile public_pwritev)(int, const struct iovec *, int, off_t) = pwritev;
static int (*volatile public_socket)(int, int, int) = socket;
static int (*volatile public_socketpair)(int, int, int, int [2]) = socketpair;
static int (*volatile public_bind)(int, const struct sockaddr *, socklen_t) = bind;
static int (*volatile public_connect)(int, const struct sockaddr *, socklen_t) = connect;
static int (*volatile public_listen)(int, int) = listen;
static int (*volatile public_accept4)(int, struct sockaddr *, socklen_t *, int) = accept4;
static ssize_t (*volatile public_sendto)(int, const void *, size_t, int,
    const struct sockaddr *, socklen_t) = sendto;
static ssize_t (*volatile public_recvfrom)(int, void *, size_t, int,
    struct sockaddr *, socklen_t *) = recvfrom;
static ssize_t (*volatile public_sendmsg)(int, const struct msghdr *, int) = sendmsg;
static ssize_t (*volatile public_recvmsg)(int, struct msghdr *, int) = recvmsg;
static int (*volatile public_shutdown)(int, int) = shutdown;
static int (*volatile public_setsockopt)(int, int, int, const void *, socklen_t) = setsockopt;
static int (*volatile public_getsockopt)(int, int, int, void *, socklen_t *) = getsockopt;
static void *(*volatile public_mmap)(void *, size_t, int, int, int, off_t) = mmap;
static void *(*volatile public_mremap)(void *, size_t, size_t, int, ...) = mremap;
static int (*volatile public_mprotect)(void *, size_t, int) = mprotect;
static int (*volatile public_munmap)(void *, size_t) = munmap;
static int (*volatile public_madvise)(void *, size_t, int) = madvise;
static DIR *(*volatile public_opendir)(const char *) = opendir;
static DIR *(*volatile public_fdopendir)(int) = fdopendir;
static struct dirent *(*volatile public_readdir)(DIR *) = readdir;
static int (*volatile public_closedir)(DIR *) = closedir;
static int (*volatile public_dirfd)(DIR *) = dirfd;
static void (*volatile public_rewinddir)(DIR *) = rewinddir;
static long (*volatile public_telldir)(DIR *) = telldir;
static void (*volatile public_seekdir)(DIR *, long) = seekdir;
static int (*volatile public_scandir)(const char *, struct dirent ***,
    int (*)(const struct dirent *),
    int (*)(const struct dirent **, const struct dirent **)) = scandir;
static int (*volatile public_scandirat)(int, const char *, struct dirent ***,
    int (*)(const struct dirent *),
    int (*)(const struct dirent **, const struct dirent **)) = scandirat;
static int (*volatile public_alphasort)(const struct dirent **,
    const struct dirent **) = alphasort;
static int (*volatile public_versionsort)(const struct dirent **,
    const struct dirent **) = versionsort;
static ssize_t (*volatile public_pwritev2)(int, const struct iovec *, int, off_t, int) = pwritev2;
static int (*volatile public_pipe2)(int [2], int) = pipe2;
static ssize_t (*volatile public_readlink)(const char *, char *, size_t) = readlink;
static int (*volatile public_link)(const char *, const char *) = link;
static int (*volatile public_unlink)(const char *) = unlink;
static int (*volatile public_rmdir)(const char *) = rmdir;
static int (*volatile public_mkdir)(const char *, mode_t) = mkdir;
static int (*volatile public_rename)(const char *, const char *) = rename;
static int (*volatile public_symlink)(const char *, const char *) = symlink;
static int (*volatile public_chmod)(const char *, mode_t) = chmod;
static int (*volatile public_fchmod)(int, mode_t) = fchmod;
static int (*volatile public_fchmodat)(int, const char *, mode_t, int) = fchmodat;
static int (*volatile public_utimensat)(int, const char *, const struct timespec [2], int) = utimensat;
static int (*volatile public_fallocate)(int, int, off_t, off_t) = fallocate;
static int (*volatile public_posix_fallocate)(int, off_t, off_t) = posix_fallocate;
static int (*volatile public_posix_fadvise)(int, off_t, off_t, int) = posix_fadvise;
static ssize_t (*volatile public_readahead)(int, off_t, size_t) = readahead;
static ssize_t (*volatile public_sendfile)(int, int, off_t *, size_t) = sendfile;
static ssize_t (*volatile public_copy_file_range)(int, off_t *, int, off_t *, size_t, unsigned int) = copy_file_range;
static ssize_t (*volatile public_splice)(int, off_t *, int, off_t *, size_t, unsigned int) = splice;
static ssize_t (*volatile public_tee)(int, int, size_t, unsigned int) = tee;
static ssize_t (*volatile public_vmsplice)(int, const struct iovec *, size_t, unsigned int) = vmsplice;
static int (*volatile public_fsetxattr)(int, const char *, const void *, size_t, int) = fsetxattr;
static ssize_t (*volatile public_fgetxattr)(int, const char *, void *, size_t) = fgetxattr;
static ssize_t (*volatile public_flistxattr)(int, char *, size_t) = flistxattr;
static int (*volatile public_fremovexattr)(int, const char *) = fremovexattr;
static int (*volatile public_setxattr)(const char *, const char *, const void *, size_t, int) = setxattr;
static ssize_t (*volatile public_getxattr)(const char *, const char *, void *, size_t) = getxattr;
static ssize_t (*volatile public_listxattr)(const char *, char *, size_t) = listxattr;
static int (*volatile public_removexattr)(const char *, const char *) = removexattr;
static int (*volatile public_lsetxattr)(const char *, const char *, const void *, size_t, int) = lsetxattr;
static ssize_t (*volatile public_lgetxattr)(const char *, const char *, void *, size_t) = lgetxattr;
static ssize_t (*volatile public_llistxattr)(const char *, char *, size_t) = llistxattr;
static int (*volatile public_lremovexattr)(const char *, const char *) = lremovexattr;
static int (*volatile public_setxattrat)(int, const char *, unsigned int, const char *, const struct nelibc_xattr_args *, size_t) = setxattrat;
static ssize_t (*volatile public_getxattrat)(int, const char *, unsigned int, const char *, struct nelibc_xattr_args *, size_t) = getxattrat;
static ssize_t (*volatile public_listxattrat)(int, const char *, unsigned int, char *, size_t) = listxattrat;
static int (*volatile public_removexattrat)(int, const char *, unsigned int, const char *) = removexattrat;
static int (*volatile public_file_getattr)(int, const char *, struct nelibc_file_attr *, size_t, unsigned int) = file_getattr;
static int (*volatile public_file_setattr)(int, const char *, const struct nelibc_file_attr *, size_t, unsigned int) = file_setattr;
static int (*volatile public_open_tree_attr)(int, const char *, unsigned int, const struct nelibc_mount_attr *, size_t) = open_tree_attr;
static int (*volatile public_mount_setattr)(int, const char *, unsigned int, struct mount_attr *, size_t) = mount_setattr;
static int (*volatile public_open_tree)(int, const char *, unsigned int) = open_tree;
static int (*volatile public_move_mount)(int, const char *, int, const char *, unsigned int) = move_mount;
static int (*volatile public_fsmount)(int, unsigned int, unsigned int) = fsmount;
static int (*volatile public_fsopen)(const char *, unsigned int) = fsopen;
static int (*volatile public_fsconfig)(int, unsigned int, const char *, const void *, int) = fsconfig;
static int (*volatile public_fspick)(int, const char *, unsigned int) = fspick;
static int (*volatile public_statmount)(struct nelibc_mnt_id_req *, struct nelibc_statmount *, size_t, unsigned int) = statmount;
static long (*volatile public_listmount)(struct nelibc_mnt_id_req *, uint64_t *, size_t, unsigned int) = listmount;
static int (*volatile public_fchownat)(int, const char *, uid_t, gid_t, int) = fchownat;
static int (*volatile public_chown)(const char *, uid_t, gid_t) = chown;
static int (*volatile public_lchown)(const char *, uid_t, gid_t) = lchown;
static int (*volatile public_mkfifo)(const char *, mode_t) = mkfifo;
static int (*volatile public_mkfifoat)(int, const char *, mode_t) = mkfifoat;
static int (*volatile public_mknod)(const char *, mode_t, dev_t) = mknod;
static int (*volatile public_mknodat)(int, const char *, mode_t, dev_t) = mknodat;
static int (*volatile public_getresuid)(uid_t *, uid_t *, uid_t *) = getresuid;
static int (*volatile public_setresuid)(uid_t, uid_t, uid_t) = setresuid;
static int (*volatile public_getresgid)(gid_t *, gid_t *, gid_t *) = getresgid;
static int (*volatile public_setresgid)(gid_t, gid_t, gid_t) = setresgid;
static int (*volatile public_getgroups)(int, gid_t []) = getgroups;
static int (*volatile public_setgroups)(size_t, const gid_t []) = setgroups;
static int (*volatile public_getrusage)(int, struct rusage *) = getrusage;
static __typeof__(&getrlimit) volatile public_getrlimit = getrlimit;
static __typeof__(&setrlimit) volatile public_setrlimit = setrlimit;
static __typeof__(&getpriority) volatile public_getpriority = getpriority;
static __typeof__(&setpriority) volatile public_setpriority = setpriority;
static int (*volatile public_stat)(const char *, struct stat *) = stat;
static int (*volatile public_lstat)(const char *, struct stat *) = lstat;
static int (*volatile public_fstat)(int, struct stat *) = fstat;
static int (*volatile public_fstatat)(int, const char *, struct stat *, int) = fstatat;
static int (*volatile public_statx)(int, const char *, int, unsigned int,
    struct statx *) = statx;
static int (*volatile public_uname)(struct utsname *) = uname;
static ssize_t (*volatile public_getrandom)(void *, size_t, unsigned int) = getrandom;
static int (*volatile public_getentropy)(void *, size_t) = getentropy;
static clock_t (*volatile public_times)(struct tms *) = times;
static int (*volatile public_gettimeofday)(struct timeval *, void *) = gettimeofday;
static int (*volatile public_sched_yield)(void) = sched_yield;
static int (*volatile public_sched_getaffinity)(pid_t, size_t, cpu_set_t *) = sched_getaffinity;
static int (*volatile public_sched_setaffinity)(pid_t, size_t, const cpu_set_t *) = sched_setaffinity;
static unsigned int (*volatile public_sleep)(unsigned int) = sleep;
static unsigned int (*volatile public_alarm)(unsigned int) = alarm;
static int (*volatile public_pause)(void) = pause;
static pid_t (*volatile public_wait)(int *) = wait;
static pid_t (*volatile public_waitpid)(pid_t, int *, int) = waitpid;
static pid_t (*volatile public_wait4)(pid_t, int *, int, struct rusage *) = wait4;
static int (*volatile public_waitid)(idtype_t, id_t, siginfo_t *, int) = waitid;
static pid_t (*volatile public_fork)(void) = fork;
static int (*volatile public_mount)(const char *, const char *, const char *, unsigned long, const void *) = mount;
static int (*volatile public_umount)(const char *) = umount;
static int (*volatile public_umount2)(const char *, int) = umount2;
static int (*volatile public_pivot_root)(const char *, const char *) = pivot_root;
static int (*volatile public_unshare)(int) = unshare;
static int (*volatile public_setns)(int, int) = setns;
static int (*volatile public_sethostname)(const char *, size_t) = sethostname;
static int (*volatile public_setdomainname)(const char *, size_t) = setdomainname;
static int (*volatile public_gethostname)(char *, size_t) = gethostname;
static int (*volatile public_getdomainname)(char *, size_t) = getdomainname;
static long (*volatile public_pathconf)(const char *, int) = pathconf;
static long (*volatile public_fpathconf)(int, int) = fpathconf;
static int (*volatile public_execve)(const char *, char *const [], char *const []) = execve;
static void (*volatile public__exit)(int) = _exit;

int main(void) {
  return public_opendir == 0 || public_fdopendir == 0 || public_readdir == 0 || public_closedir == 0 ||
         public_dirfd == 0 || public_rewinddir == 0 || public_telldir == 0 || public_seekdir == 0 ||
         public_scandir == 0 || public_scandirat == 0 || public_alphasort == 0 ||
         public_versionsort == 0 ||
         public_preadv2 == 0 || public_fcntl == 0 || public_openat == 0 ||
         public_poll == 0 || public_select == 0 || public_readv == 0 ||
         public_writev == 0 || public_preadv == 0 || public_pwritev == 0 ||
         public_socket == 0 || public_socketpair == 0 || public_bind == 0 ||
         public_connect == 0 || public_listen == 0 || public_accept4 == 0 ||
         public_sendto == 0 || public_recvfrom == 0 || public_sendmsg == 0 ||
         public_recvmsg == 0 || public_shutdown == 0 ||
         public_setsockopt == 0 || public_getsockopt == 0 ||
         public_mmap == 0 || public_mremap == 0 || public_mprotect == 0 ||
         public_munmap == 0 || public_madvise == 0 || public_pwritev2 == 0 ||
         public_pipe2 == 0 || public_readlink == 0 ||
         public_link == 0 || public_unlink == 0 || public_rmdir == 0 || public_mkdir == 0 ||
         public_rename == 0 || public_symlink == 0 || public_chmod == 0 || public_fchmod == 0 ||
         public_fchmodat == 0 || public_utimensat == 0 || public_fallocate == 0 || public_posix_fallocate == 0 ||
         public_posix_fadvise == 0 || public_readahead == 0 || public_sendfile == 0 || public_copy_file_range == 0 || public_splice == 0 || public_tee == 0 || public_vmsplice == 0 || public_fsetxattr == 0 || public_fgetxattr == 0 || public_flistxattr == 0 || public_fremovexattr == 0 || public_setxattr == 0 || public_getxattr == 0 || public_listxattr == 0 || public_removexattr == 0 || public_lsetxattr == 0 || public_lgetxattr == 0 || public_llistxattr == 0 || public_lremovexattr == 0 ||
         public_setxattrat == 0 || public_getxattrat == 0 || public_listxattrat == 0 || public_removexattrat == 0 ||
         public_file_getattr == 0 || public_file_setattr == 0 ||
         public_open_tree_attr == 0 ||
         public_mount_setattr == 0 ||
         public_open_tree == 0 ||
         public_move_mount == 0 ||
         public_fsmount == 0 ||
         public_fsopen == 0 || public_fsconfig == 0 || public_fspick == 0 ||
         public_statmount == 0 || public_listmount == 0 ||
         public_fchownat == 0 || public_chown == 0 || public_lchown == 0 ||
         public_mkfifo == 0 || public_mkfifoat == 0 || public_mknod == 0 || public_mknodat == 0 ||
         public_getresuid == 0 || public_setresuid == 0 || public_getresgid == 0 || public_setresgid == 0 ||
         public_getgroups == 0 || public_setgroups == 0 ||
         public_getrusage == 0 || public_getrlimit == 0 || public_setrlimit == 0 ||
         public_getpriority == 0 || public_setpriority == 0 ||
         public_stat == 0 || public_lstat == 0 || public_fstat == 0 ||
         public_fstatat == 0 || public_statx == 0 || public_uname == 0 ||
         public_getrandom == 0 || public_getentropy == 0 ||
         public_times == 0 || public_gettimeofday == 0 ||
         public_sched_yield == 0 || public_sched_getaffinity == 0 || public_sched_setaffinity == 0 ||
         public_sleep == 0 || public_alarm == 0 || public_pause == 0 || public_wait == 0 ||
         public_waitpid == 0 || public_wait4 == 0 || public_waitid == 0 ||
         public_fork == 0 ||
         public_mount == 0 || public_umount == 0 || public_umount2 == 0 || public_pivot_root == 0 ||
         public_unshare == 0 || public_setns == 0 ||
         public_sethostname == 0 || public_setdomainname == 0 ||
         public_gethostname == 0 || public_getdomainname == 0 ||
         public_pathconf == 0 || public_fpathconf == 0 ||
         public_execve == 0 || public__exit == 0;
}
