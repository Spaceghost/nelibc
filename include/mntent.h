#ifndef NELIBC_MNTENT_WRAPPER_H
#define NELIBC_MNTENT_WRAPPER_H

/* Hybrid builds use the host's <mntent.h>. The standalone profile provides
 * these in src/stdio_extras_adapter.c (after musl 1.2.5). */
#ifdef NELIBC_STANDALONE

#include <stdio.h>

#ifdef __cplusplus
extern "C" {
#endif

#define MOUNTED "/etc/mtab"

#define MNTTYPE_IGNORE "ignore"
#define MNTTYPE_NFS    "nfs"
#define MNTTYPE_SWAP   "swap"
#define MNTOPT_DEFAULTS "defaults"
#define MNTOPT_RO      "ro"
#define MNTOPT_RW      "rw"
#define MNTOPT_SUID    "suid"
#define MNTOPT_NOSUID  "nosuid"
#define MNTOPT_NOAUTO  "noauto"

struct mntent {
  char *mnt_fsname;
  char *mnt_dir;
  char *mnt_type;
  char *mnt_opts;
  int mnt_freq;
  int mnt_passno;
};

FILE *setmntent(const char *, const char *);
int endmntent(FILE *);
struct mntent *getmntent(FILE *);
struct mntent *getmntent_r(FILE *, struct mntent *, char *, int);
int addmntent(FILE *, const struct mntent *);
char *hasmntopt(const struct mntent *, const char *);

#ifdef __cplusplus
}
#endif

#else

#include_next <mntent.h>

#endif

#endif
