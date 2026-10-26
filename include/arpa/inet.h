#ifndef NELIBC_ARPA_INET_H
#define NELIBC_ARPA_INET_H

#ifdef NELIBC_STANDALONE
#include <stdint.h>
#include <sys/socket.h>
typedef uint32_t in_addr_t;
struct in_addr { in_addr_t s_addr; };
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
in_addr_t inet_addr(const char *cp);
int inet_aton(const char *cp, struct in_addr *inp);
char *inet_ntoa(struct in_addr in);

#define INET_ADDRSTRLEN 16
#define INET6_ADDRSTRLEN 46
#ifndef INADDR_ANY
#define INADDR_ANY ((in_addr_t)0x00000000)
#endif
#ifndef INADDR_BROADCAST
#define INADDR_BROADCAST ((in_addr_t)0xffffffff)
#endif
#ifndef INADDR_NONE
#define INADDR_NONE ((in_addr_t)0xffffffff)
#endif

#ifdef __cplusplus
}
#endif

#endif
