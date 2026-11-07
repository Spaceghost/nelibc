#ifndef NELIBC_PWD_WRAPPER_H
#define NELIBC_PWD_WRAPPER_H

/* Hybrid builds use the host's <pwd.h> (and its NSS). The standalone
 * profile reads /etc/passwd in src/pwd_standalone.nelua (after musl 1.2.5). */
#ifdef NELIBC_STANDALONE

#include <stddef.h>
#include <sys/types.h>

#ifdef __cplusplus
extern "C" {
#endif

struct passwd {
  char *pw_name;
  char *pw_passwd;
  uid_t pw_uid;
  gid_t pw_gid;
  char *pw_gecos;
  char *pw_dir;
  char *pw_shell;
};

struct passwd *getpwuid(uid_t);
struct passwd *getpwnam(const char *);
int getpwuid_r(uid_t, struct passwd *, char *, size_t, struct passwd **);
int getpwnam_r(const char *, struct passwd *, char *, size_t, struct passwd **);

#if !defined(_POSIX_C_SOURCE) || defined(_XOPEN_SOURCE) || \
    defined(_GNU_SOURCE) || defined(_DEFAULT_SOURCE) || defined(_BSD_SOURCE)
void setpwent(void);
void endpwent(void);
struct passwd *getpwent(void);
#endif

#if (!defined(_POSIX_C_SOURCE) && !defined(_XOPEN_SOURCE)) || \
    defined(_GNU_SOURCE) || defined(_DEFAULT_SOURCE) || defined(_BSD_SOURCE)
struct nelibc_FILE;
struct passwd *fgetpwent(struct nelibc_FILE *);
int putpwent(const struct passwd *, struct nelibc_FILE *);
#endif

#ifdef __cplusplus
}
#endif

#else

#include_next <pwd.h>

#endif

#endif
