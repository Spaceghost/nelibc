#include <getopt.h>
#include <stdio.h>
#include <string.h>

int main(int argc, char **argv) {
  const char *mode = argc > 1 ? argv[1] : "mixed";
  char *mixed[] = {"probe", "file1", "-a", "-bVALUE", "file2", "-c", NULL};
  char *terminator[] = {"probe", "file1", "-a", "--", "file2", "-b", NULL};
  char *missing[] = {"probe", "file1", "-b", NULL};
  char *optional[] = {"probe", "-aVALUE", "-a", "ignored", "-bX", NULL};
  char *cluster[] = {"probe", "file1", "-ac", "file2", NULL};
  char *unknown[] = {"probe", "-az", "file1", NULL};
  char **args = mixed;
  const char *options = "ab:c";
  int count = 6;
  if (strcmp(mode, "terminator") == 0) { args = terminator; count = 6; }
  if (strcmp(mode, "missing") == 0) { args = missing; count = 3; options = ":ab:c"; }
  if (strcmp(mode, "optional") == 0) { args = optional; count = 5; options = "a::b:"; }
  if (strcmp(mode, "cluster") == 0) { args = cluster; count = 4; }
  if (strcmp(mode, "unknown") == 0) { args = unknown; count = 3; }
  if (strcmp(mode, "plus") == 0) options = "+ab:c";
  if (strcmp(mode, "minus") == 0) options = "-ab:c";
  optind = 0;
  opterr = 0;
  int option;
  while ((option = getopt(count, args, options)) != -1)
    printf("%d:%s:%d:%d\n", option, optarg ? optarg : "", optind, optopt);
  printf("end:%d", optind);
  for (int i = 1; i < count; ++i) printf(":%s", args[i]);
  putchar('\n');
  return 0;
}
