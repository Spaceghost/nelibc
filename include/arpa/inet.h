#ifndef NELIBC_ARPA_INET_H
#define NELIBC_ARPA_INET_H

#ifdef NELIBC_STANDALONE
#include <stdint.h>
#include <sys/socket.h>
#else
#include_next <arpa/inet.h>
#endif

#ifdef __cplusplus
extern "C" {
#endif

uint32_t htonl(uint32_t hostlong);
uint16_t htons(uint16_t hostshort);
uint32_t ntohl(uint32_t netlong);
uint16_t ntohs(uint16_t netshort);
int inet_pton(int af, const char *__restrict src, void *__restrict dst);
const char *inet_ntop(int af, const void *__restrict src, char *__restrict dst,
                      socklen_t size);

#define INET_ADDRSTRLEN 16
#define INET6_ADDRSTRLEN 46

#ifdef __cplusplus
}
#endif

#endif
