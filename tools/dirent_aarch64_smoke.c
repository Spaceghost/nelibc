#include <dirent.h>
#include <fcntl.h>
#include <stdlib.h>
#include <string.h>

int main(void) {
  DIR *dir = opendir(".");
  if (!dir) return 1;
  if (fcntl(dirfd(dir), F_GETFD) != FD_CLOEXEC) return 2;
  struct dirent **entries;
  int count = scandirat(dirfd(dir), ".", &entries, NULL, NULL);
  if (count < 0) return 3;
  int found_dot = 0;
  for (int i = 0; i < count; ++i) {
    if (strcmp(entries[i]->d_name, ".") == 0) found_dot = 1;
    free(entries[i]);
  }
  free(entries);
  return closedir(dir) != 0 || !found_dot;
}
