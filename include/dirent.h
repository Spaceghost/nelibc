#ifndef NELIBC_DIRENT_WRAPPER_H
#define NELIBC_DIRENT_WRAPPER_H

#ifdef NELIBC_STANDALONE
#include <sys/types.h>

struct dirent {
  ino_t d_ino;
  off_t d_off;
  unsigned short d_reclen;
  unsigned char d_type;
  char d_name[256];
};
typedef struct __dirstream DIR;

#ifndef DT_UNKNOWN
#define DT_UNKNOWN 0
#define DT_FIFO 1
#define DT_CHR 2
#define DT_DIR 4
#define DT_BLK 6
#define DT_REG 8
#define DT_LNK 10
#define DT_SOCK 12
#define DT_WHT 14
#endif
#else
#include_next <dirent.h>
#endif

DIR *opendir(const char *name);
DIR *fdopendir(int fd);
struct dirent *readdir(DIR *dirp);
int closedir(DIR *dirp);
int dirfd(DIR *dirp);
void rewinddir(DIR *dirp);
long telldir(DIR *dirp);
void seekdir(DIR *dirp, long location);
int scandir(const char *name, struct dirent ***namelist,
            int (*selector)(const struct dirent *),
            int (*compare)(const struct dirent **, const struct dirent **));
int scandirat(int dirfd, const char *name, struct dirent ***namelist,
              int (*selector)(const struct dirent *),
              int (*compare)(const struct dirent **, const struct dirent **));
int alphasort(const struct dirent **left, const struct dirent **right);
int versionsort(const struct dirent **left, const struct dirent **right);

#endif
