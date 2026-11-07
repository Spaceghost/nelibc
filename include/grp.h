#ifndef NELIBC_GRP_WRAPPER_H
#define NELIBC_GRP_WRAPPER_H

/* Hybrid builds use the host's <grp.h> (and its NSS). The standalone
 * profile reads /etc/group in src/pwd_standalone.nelua (after musl 1.2.5). */
#ifdef NELIBC_STANDALONE

#include <stddef.h>
#include <sys/types.h>

#ifdef __cplusplus
extern "C" {
#endif

struct group {
  char *gr_name;
  char *gr_passwd;
  gid_t gr_gid;
  char **gr_mem;
};

struct group *getgrgid(gid_t);
struct group *getgrnam(const char *);
int getgrgid_r(gid_t, struct group *, char *, size_t, struct group **);
int getgrnam_r(const char *, struct group *, char *, size_t, struct group **);

#if !defined(_POSIX_C_SOURCE) || defined(_XOPEN_SOURCE) || \
    defined(_GNU_SOURCE) || defined(_DEFAULT_SOURCE) || defined(_BSD_SOURCE)
struct group *getgrent(void);
void endgrent(void);
void setgrent(void);
#endif

#if (!defined(_POSIX_C_SOURCE) && !defined(_XOPEN_SOURCE)) || \
    defined(_GNU_SOURCE) || defined(_DEFAULT_SOURCE) || defined(_BSD_SOURCE)
struct nelibc_FILE;
struct group *fgetgrent(struct nelibc_FILE *);
int putgrent(const struct group *, struct nelibc_FILE *);
int getgrouplist(const char *, gid_t, gid_t *, int *);
int setgroups(size_t, const gid_t *);
int initgroups(const char *, gid_t);
#endif

#ifdef __cplusplus
}
#endif

#else

#include_next <grp.h>

#endif

#endif
