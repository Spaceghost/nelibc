/* Standalone profile, after musl 1.2.5: popen and pclose (src/stdio),
 * <mntent.h> (src/misc/mntent.c, with its octal unescaping), mktemp
 * (src/temp/mktemp.c) and daemon (src/legacy/daemon.c).
 *
 * popen runs /bin/sh -c CMD through posix_spawn with one end of a pipe as the
 * child's standard input or output; the child also closes the streams of
 * earlier popen calls (POSIX). nelibc's FILE has no pipe-pid field, so the
 * (stream, pid) pairs live in a list here, guarded by a spin lock. */
#include <errno.h>
#include <fcntl.h>
#include <limits.h>
#include <mntent.h>
#include <spawn.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/random.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <unistd.h>

extern char **environ;
int pipe2(int fds[2], int flags);

struct popen_entry {
	FILE *f;
	pid_t pid;
	struct popen_entry *next;
};

static struct popen_entry *popen_list;
static volatile int popen_lock_word;

static void popen_lock(void)
{
	while (__atomic_exchange_n(&popen_lock_word, 1, __ATOMIC_ACQUIRE)) { }
}

static void popen_unlock(void)
{
	__atomic_store_n(&popen_lock_word, 0, __ATOMIC_RELEASE);
}

FILE *popen(const char *cmd, const char *mode)
{
	int p[2], op, e;
	pid_t pid;
	FILE *f;
	posix_spawn_file_actions_t fa;
	struct popen_entry *entry;

	if (*mode == 'r') op = 0;
	else if (*mode == 'w') op = 1;
	else { errno = EINVAL; return 0; }

	entry = malloc(sizeof *entry);
	if (!entry) return 0;
	if (pipe2(p, O_CLOEXEC)) { free(entry); return 0; }
	f = fdopen(p[op], op ? "w" : "r");
	if (!f) { close(p[0]); close(p[1]); free(entry); return 0; }

	e = ENOMEM;
	popen_lock();
	if (!posix_spawn_file_actions_init(&fa)) {
		struct popen_entry *l;
		for (l = popen_list; l; l = l->next)
			if (posix_spawn_file_actions_addclose(&fa, fileno(l->f))) goto fail;
		if (!posix_spawn_file_actions_adddup2(&fa, p[1 - op], 1 - op)) {
			char *argv[] = { "sh", "-c", (char *)cmd, 0 };
			if (!(e = posix_spawn(&pid, "/bin/sh", &fa, 0, argv, environ))) {
				posix_spawn_file_actions_destroy(&fa);
				entry->f = f;
				entry->pid = pid;
				entry->next = popen_list;
				popen_list = entry;
				popen_unlock();
				if (!strchr(mode, 'e')) fcntl(p[op], F_SETFD, 0);
				close(p[1 - op]);
				return f;
			}
		}
fail:
		posix_spawn_file_actions_destroy(&fa);
	}
	popen_unlock();
	fclose(f);
	close(p[1 - op]);
	free(entry);
	errno = e;
	return 0;
}

int pclose(FILE *f)
{
	struct popen_entry **link, *entry = 0;
	pid_t pid;
	int status, r;

	popen_lock();
	for (link = &popen_list; *link; link = &(*link)->next)
		if ((*link)->f == f) { entry = *link; *link = entry->next; break; }
	popen_unlock();
	if (!entry) { errno = ECHILD; return -1; }
	pid = entry->pid;
	free(entry);
	fclose(f);
	while ((r = waitpid(pid, &status, 0)) < 0 && errno == EINTR) { }
	if (r < 0) return -1;
	return status;
}

/* <mntent.h> */

static char *internal_buf;
static size_t internal_bufsize;
#define SENTINEL ((char *)&internal_buf)

FILE *setmntent(const char *name, const char *mode)
{
	return fopen(name, mode);
}

int endmntent(FILE *f)
{
	if (f) fclose(f);
	return 1;
}

/* \ooo octal escapes (\040 for a space) and \\ (musl 1.2.4 and later). */
static char *unescape_ent(char *beg)
{
	char *dest = beg;
	const char *src = beg;
	while (*src) {
		const char *val;
		unsigned char cval = 0;
		int i;
		if (*src != '\\') { *dest++ = *src++; continue; }
		if (src[1] == '\\') { ++src; *dest++ = *src++; continue; }
		val = src + 1;
		for (i = 0; i < 3; ++i) {
			if (*val >= '0' && *val <= '7') { cval <<= 3; cval += *val++ - '0'; }
			else break;
		}
		if (cval) { *dest++ = cval; src = val; }
		else { *dest++ = *src++; }
	}
	*dest = 0;
	return beg;
}

struct mntent *getmntent_r(FILE *f, struct mntent *mnt, char *linebuf, int buflen)
{
	int n[8], use_internal = (linebuf == SENTINEL);
	size_t len, i;

	mnt->mnt_freq = 0;
	mnt->mnt_passno = 0;
	do {
		if (use_internal) {
			getline(&internal_buf, &internal_bufsize, f);
			linebuf = internal_buf;
		} else {
			fgets(linebuf, buflen, f);
		}
		if (feof(f) || ferror(f)) return 0;
		if (!strchr(linebuf, '\n')) {
			fscanf(f, "%*[^\n]%*[\n]");
			errno = ERANGE;
			return 0;
		}
		len = strlen(linebuf);
		if (len > INT_MAX) continue;
		for (i = 0; i < sizeof n / sizeof *n; i++) n[i] = (int)len;
		sscanf(linebuf, " %n%*[^ \t]%n %n%*[^ \t]%n %n%*[^ \t]%n %n%*[^ \t]%n %d %d",
			n, n + 1, n + 2, n + 3, n + 4, n + 5, n + 6, n + 7,
			&mnt->mnt_freq, &mnt->mnt_passno);
	} while (linebuf[n[0]] == '#' || n[1] == (int)len);

	linebuf[n[1]] = 0;
	linebuf[n[3]] = 0;
	linebuf[n[5]] = 0;
	linebuf[n[7]] = 0;

	mnt->mnt_fsname = unescape_ent(linebuf + n[0]);
	mnt->mnt_dir = unescape_ent(linebuf + n[2]);
	mnt->mnt_type = unescape_ent(linebuf + n[4]);
	mnt->mnt_opts = unescape_ent(linebuf + n[6]);
	return mnt;
}

struct mntent *getmntent(FILE *f)
{
	static struct mntent mnt;
	return getmntent_r(f, &mnt, SENTINEL, 0);
}

int addmntent(FILE *f, const struct mntent *mnt)
{
	if (fseek(f, 0, SEEK_END)) return 1;
	return fprintf(f, "%s\t%s\t%s\t%s\t%d\t%d\n",
		mnt->mnt_fsname, mnt->mnt_dir, mnt->mnt_type, mnt->mnt_opts,
		mnt->mnt_freq, mnt->mnt_passno) < 0;
}

char *hasmntopt(const struct mntent *mnt, const char *opt)
{
	return strstr(mnt->mnt_opts, opt);
}

/* mktemp: deprecated and racy, but POSIX.1-2001 and libc-test still use it. */
char *mktemp(char *template)
{
	static const char chars[] = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789";
	size_t l = strlen(template);
	int retries = 100;
	struct stat st;

	if (l < 6 || memcmp(template + l - 6, "XXXXXX", 6)) {
		errno = EINVAL;
		*template = 0;
		return template;
	}
	do {
		unsigned char r[6];
		int i;
		if (getrandom(r, sizeof r, 0) != (ssize_t)sizeof r) { *template = 0; return template; }
		for (i = 0; i < 6; i++) template[l - 6 + i] = chars[r[i] % (sizeof chars - 1)];
		if (stat(template, &st)) {
			if (errno != ENOENT) *template = 0;
			return template;
		}
	} while (--retries);
	*template = 0;
	errno = EEXIST;
	return template;
}

int daemon(int nochdir, int noclose)
{
	if (!nochdir && chdir("/")) return -1;
	if (!noclose) {
		int fd, failed = 0;
		if ((fd = open("/dev/null", O_RDWR)) < 0) return -1;
		if (dup2(fd, 0) < 0 || dup2(fd, 1) < 0 || dup2(fd, 2) < 0) failed++;
		if (fd > 2) close(fd);
		if (failed) return -1;
	}
	switch (fork()) {
	case 0: break;
	case -1: return -1;
	default: _exit(0);
	}
	if (setsid() < 0) return -1;
	switch (fork()) {
	case 0: break;
	case -1: return -1;
	default: _exit(0);
	}
	return 0;
}
