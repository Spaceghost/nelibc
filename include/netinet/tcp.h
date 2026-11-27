#ifndef NELIBC_NETINET_TCP_WRAPPER_H
#define NELIBC_NETINET_TCP_WRAPPER_H

#ifdef NELIBC_STANDALONE

#include <netinet/in.h>

/* Linux TCP socket option numbers. */
#define TCP_NODELAY 1
#define TCP_MAXSEG 2
#define TCP_CORK 3
#define TCP_KEEPIDLE 4
#define TCP_KEEPINTVL 5
#define TCP_KEEPCNT 6

#else

#include_next <netinet/tcp.h>

#endif

#endif
