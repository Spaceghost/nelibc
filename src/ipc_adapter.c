/* System V IPC for the standalone profile, after musl 1.2.5 src/ipc:
 * ftok, msgget/msgsnd/msgrcv/msgctl, semget/semop/semtimedop/semctl,
 * shmget/shmat/shmdt/shmctl. Each is one platform operation (nelibc_sys.h),
 * so the host profile can answer them too. The structures in <sys/ipc.h>,
 * <sys/msg.h>, <sys/sem.h> and <sys/shm.h> have the kernel's x86-64 64-bit
 * layout, as musl's do, and are passed through unchanged. */
#include <stdarg.h>
#include <stddef.h>
#include "nelibc_sys.h"

extern int *nelibc_errno_location(void);

typedef int key_t;
struct timespec;

static long ret(long r)
{
	if (r < 0 && r >= -4095) {
		*nelibc_errno_location() = (int)-r;
		return -1;
	}
	return r;
}

/* The kernel's x86-64 struct stat, for ftok's device and inode numbers. */
struct kstat {
	unsigned long st_dev, st_ino, st_nlink;
	unsigned int st_mode, st_uid, st_gid, pad0;
	unsigned long st_rdev;
	long st_size, st_blksize, st_blocks;
	unsigned long times[6];
	long reserved[3];
};

key_t ftok(const char *path, int id)
{
	struct kstat st;
	if (ret(nelibc_sys3(NELIBC_SYS_STAT, (unsigned long)path, (unsigned long)&st, 0)) < 0)
		return -1;
	return (key_t)((st.st_ino & 0xffff) | ((st.st_dev & 0xff) << 16) | ((id & 0xffu) << 24));
}

int msgget(key_t key, int flag)
{
	return (int)ret(nelibc_sys3(NELIBC_SYS_MSGGET, (unsigned long)(long)key, (unsigned long)(long)flag, 0));
}

int msgsnd(int q, const void *m, size_t len, int flag)
{
	return (int)ret(nelibc_sys6(NELIBC_SYS_MSGSND, (unsigned long)(long)q, (unsigned long)m, len,
		(unsigned long)(long)flag, 0, 0));
}

long msgrcv(int q, void *m, size_t len, long type, int flag)
{
	return ret(nelibc_sys6(NELIBC_SYS_MSGRCV, (unsigned long)(long)q, (unsigned long)m, len,
		(unsigned long)type, (unsigned long)(long)flag, 0));
}

struct msqid_ds;
int msgctl(int q, int cmd, struct msqid_ds *buf)
{
	return (int)ret(nelibc_sys3(NELIBC_SYS_MSGCTL, (unsigned long)(long)q, (unsigned long)(long)cmd, (unsigned long)buf));
}

int semget(key_t key, int n, int fl)
{
	/* The kernel takes an int and checks it is not negative; a size_t
	 * argument above INT_MAX must not wrap into a valid count (musl). */
	if ((unsigned)n > 0x7fffffff) n = -1;
	return (int)ret(nelibc_sys3(NELIBC_SYS_SEMGET, (unsigned long)(long)key, (unsigned long)(long)n, (unsigned long)(long)fl));
}

struct sembuf;
int semop(int id, struct sembuf *buf, size_t n)
{
	return (int)ret(nelibc_sys3(NELIBC_SYS_SEMOP, (unsigned long)(long)id, (unsigned long)buf, n));
}

int semtimedop(int id, struct sembuf *buf, size_t n, const struct timespec *ts)
{
	return (int)ret(nelibc_sys6(NELIBC_SYS_SEMTIMEDOP, (unsigned long)(long)id, (unsigned long)buf, n,
		(unsigned long)ts, 0, 0));
}

/* semctl's fourth argument is a union semun, present only for these commands. */
int semctl(int id, int num, int cmd, ...)
{
	unsigned long arg = 0;
	switch (cmd & ~0x100) {   /* IPC_64 */
	case 16: /* SETVAL */
	case 13: /* GETALL */
	case 17: /* SETALL */
	case 2:  /* IPC_STAT */
	case 1:  /* IPC_SET */
	case 3:  /* IPC_INFO */
	case 19: /* SEM_INFO */
	case 18: /* SEM_STAT */
	case 20: /* SEM_STAT_ANY */
	{
		va_list ap;
		va_start(ap, cmd);
		arg = va_arg(ap, unsigned long);
		va_end(ap);
	}
	}
	return (int)ret(nelibc_sys6(NELIBC_SYS_SEMCTL, (unsigned long)(long)id, (unsigned long)(long)num,
		(unsigned long)(long)cmd, arg, 0, 0));
}

int shmget(key_t key, size_t size, int flag)
{
	if (size > 0x7fffffffffffffffUL) size = (size_t)-1;   /* PTRDIFF_MAX, as musl */
	return (int)ret(nelibc_sys3(NELIBC_SYS_SHMGET, (unsigned long)(long)key, size, (unsigned long)(long)flag));
}

void *shmat(int id, const void *addr, int flag)
{
	return (void *)ret(nelibc_sys3(NELIBC_SYS_SHMAT, (unsigned long)(long)id, (unsigned long)addr, (unsigned long)(long)flag));
}

int shmdt(const void *addr)
{
	return (int)ret(nelibc_sys3(NELIBC_SYS_SHMDT, (unsigned long)addr, 0, 0));
}

struct shmid_ds;
int shmctl(int id, int cmd, struct shmid_ds *buf)
{
	return (int)ret(nelibc_sys3(NELIBC_SYS_SHMCTL, (unsigned long)(long)id, (unsigned long)(long)cmd, (unsigned long)buf));
}
