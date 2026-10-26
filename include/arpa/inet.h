#ifndef NELIBC_ARPA_INET_H
#define NELIBC_ARPA_INET_H

#ifdef NELIBC_STANDALONE
#include <stdint.h>
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

#ifdef __cplusplus
}
#endif

#endif
