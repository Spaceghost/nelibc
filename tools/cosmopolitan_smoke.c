#include <arpa/inet.h>
#include <assert.h>
#include <ctype.h>
#include <errno.h>
#include <math.h>
#include <netdb.h>
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#ifndef __COSMOPOLITAN__
#error "Consumer must use Cosmopolitan headers"
#endif
static void *worker(void *unused) {
  (void)unused;
  char text[] = "thread:local";
  assert(strcmp(strtok(text, ":"), "thread") == 0);
  assert(strcmp(strtok(NULL, ":"), "local") == 0);
  return NULL;
}
int main(void) {
  char bytes[64];
  memset(bytes, 0, sizeof bytes);
  memcpy(bytes, "nelibc", 7);
  memmove(bytes + 1, bytes, 7);
  assert(strcmp(bytes + 1, "nelibc") == 0);
  char *copy = strdup(bytes + 1);
  assert(copy && strlen(copy) == 6);
  free(copy);
  assert((isalpha)('A') && !(isdigit)('A'));
  char *end;
  assert(strtod("12.5end", &end) == 12.5 && strcmp(end, "end") == 0);
  errno = 0;
  assert(isinf(strtod("1e9999", &end)) && errno == ERANGE);
  assert(strcoll("abc", "abd") < 0);
  assert(strxfrm(bytes, "collation", sizeof bytes) == 9);
  assert(strcmp(bytes, "collation") == 0);
  assert(fabs(sin(0.5) - 0.479425538604203) < 1e-14);
  assert(sqrt(81.0) == 9.0);
  errno = 0;
  assert(inet_pton(-1, "127.0.0.1", bytes) == -1 && errno == EAFNOSUPPORT);
  struct in_addr address;
  char ip[INET_ADDRSTRLEN];
  assert(inet_pton(AF_INET, "127.0.0.1", &address) == 1);
  assert(inet_ntop(AF_INET, &address, ip, sizeof ip));
  assert(strcmp(ip, "127.0.0.1") == 0);
  /* Missing Nelibc services resolve to Cosmopolitan, sharing its FILE/heap. */
  FILE *stream = tmpfile();
  assert(stream);
  assert(fputs("cosmopolitan", stream) >= 0);
  rewind(stream);
  assert(fgets(bytes, sizeof bytes, stream));
  assert(strcmp(bytes, "cosmopolitan") == 0);
  assert(fclose(stream) == 0);
  struct addrinfo hints = {0}, *result = NULL;
  hints.ai_flags = AI_NUMERICHOST;
  assert(getaddrinfo("127.0.0.1", "80", &hints, &result) == 0);
  freeaddrinfo(result);
  char tokens[] = "main:state";
  assert(strcmp(strtok(tokens, ":"), "main") == 0);
  pthread_t thread;
  assert(pthread_create(&thread, NULL, worker, NULL) == 0);
  assert(pthread_join(thread, NULL) == 0);
  assert(strcmp(strtok(NULL, ":"), "state") == 0);
  puts("nelibc + Cosmopolitan: core, errno, stdio, allocator, resolver, threads OK");
  return 0;
}
