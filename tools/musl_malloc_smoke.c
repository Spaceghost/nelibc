#define _GNU_SOURCE
#include <dirent.h>
#include <locale.h>
#include <netdb.h>
#include <regex.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <wordexp.h>
#include "nelibc_alloc.h"

int main(void) {
  void *block = malloc(64);
  if (block == NULL || !nelibc_allocator_owns(block)) return 11;
  memset(block, 0x5a, 64);
  block = realloc(block, 128);
  if (block == NULL || !nelibc_allocator_owns(block) ||
      ((unsigned char *)block)[0] != 0x5a) return 12;
  free(block);

  char *copy = strdup("Nelibc allocator through musl");
  if (copy == NULL || !nelibc_allocator_owns(copy) ||
      strcmp(copy, "Nelibc allocator through musl") != 0) return 13;
  free(copy);

  DIR *directory = opendir("/proc/self");
  if (directory == NULL) return 14;
  while (readdir(directory) != NULL) {}
  if (closedir(directory) != 0) return 15;

  struct dirent **entries = NULL;
  int entry_count = scandir("/proc/self/fd", &entries, NULL, alphasort);
  if (entry_count < 0) return 16;
  for (int i = 0; i < entry_count; ++i) {
    if (!nelibc_allocator_owns(entries[i])) return 17;
    free(entries[i]);
  }
  free(entries);

  locale_t locale = newlocale(LC_ALL_MASK, "C", (locale_t)0);
  if (locale == (locale_t)0) return 18;
  locale_t locale_copy = duplocale(locale);
  if (locale_copy == (locale_t)0 || !nelibc_allocator_owns(locale_copy)) return 19;
  freelocale(locale_copy);
  freelocale(locale);

  char *stream_buffer = NULL;
  size_t stream_length = 0;
  FILE *stream = open_memstream(&stream_buffer, &stream_length);
  if (stream == NULL || fputs("musl allocation dispatch", stream) < 0 ||
      fclose(stream) != 0) return 20;
  if (stream_buffer == NULL || !nelibc_allocator_owns(stream_buffer) ||
      stream_length != strlen("musl allocation dispatch")) return 21;
  free(stream_buffer);

  regex_t expression;
  if (regcomp(&expression, "(nelibc|musl)+", REG_EXTENDED) != 0) return 22;
  regmatch_t match;
  if (regexec(&expression, "nelibcmusl", 1, &match, 0) != 0) return 23;
  regfree(&expression);

  wordexp_t words = {0};
  if (wordexp("one two", &words, 0) != 0 || words.we_wordc != 2) return 24;
  wordfree(&words);

  struct addrinfo hints = {0};
  struct addrinfo *addresses = NULL;
  hints.ai_family = AF_UNSPEC;
  hints.ai_socktype = SOCK_STREAM;
  if (getaddrinfo("127.0.0.1", "80", &hints, &addresses) != 0) return 25;
  freeaddrinfo(addresses);

  if (setenv("NELIBC_MUSL_ALLOC_PROBE", "1", 1) != 0 ||
      getenv("NELIBC_MUSL_ALLOC_PROBE") == NULL ||
      unsetenv("NELIBC_MUSL_ALLOC_PROBE") != 0) return 26;

  return 0;
}
