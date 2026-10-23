#ifndef NELIBC_SYS_UN_WRAPPER_H
#define NELIBC_SYS_UN_WRAPPER_H

#ifdef NELIBC_STANDALONE
#include <sys/socket.h>

#ifndef UNIX_PATH_MAX
#define UNIX_PATH_MAX 108
#endif

struct sockaddr_un {
  sa_family_t sun_family;
  char sun_path[UNIX_PATH_MAX];
};
#else
#include_next <sys/un.h>
#endif

#ifndef UNIX_PATH_MAX
#define UNIX_PATH_MAX 108
#endif

#endif
