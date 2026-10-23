#include <getopt.h>
#include <stdio.h>
#include <string.h>

static int verbose_flag;
static const struct option long_options[] = {
    {"alpha", no_argument, NULL, 'a'},
    {"beta", required_argument, NULL, 'b'},
    {"colour", optional_argument, NULL, 'c'},
    {"colorful", no_argument, NULL, 'C'},
    {"verbose", no_argument, &verbose_flag, 1},
    {NULL, 0, NULL, 0},
};

int main(int argc, char **argv) {
  const char *mode = argc > 1 ? argv[1] : "exact";
  char *exact[] = {"probe", "file", "--alpha", "--beta=VALUE", "-a", NULL};
  char *abbrev[] = {"probe", "--alph", NULL};
  char *required[] = {"probe", "--beta", "VALUE", NULL};
  char *optional[] = {"probe", "--colour=blue", "--colour", "ignored", NULL};
  char *flag[] = {"probe", "--verbose", NULL};
  char *ambiguous[] = {"probe", "--col", NULL};
  char *unknown[] = {"probe", "--missing", NULL};
  char *missing[] = {"probe", "--beta", NULL};
  char *single_dash_long[] = {"probe", "-alpha", NULL};
  char *single_dash_short[] = {"probe", "-a", NULL};
  char *single_dash_short_arg[] = {"probe", "-bVALUE", NULL};
  char **args = exact;
  int count = 5;
  if (strcmp(mode, "abbrev") == 0) { args = abbrev; count = 2; }
  if (strcmp(mode, "required") == 0) { args = required; count = 3; }
  if (strcmp(mode, "optional") == 0) { args = optional; count = 4; }
  if (strcmp(mode, "flag") == 0) { args = flag; count = 2; }
  if (strcmp(mode, "ambiguous") == 0) { args = ambiguous; count = 2; }
  if (strcmp(mode, "unknown") == 0) { args = unknown; count = 2; }
  if (strcmp(mode, "missing") == 0) { args = missing; count = 2; }
  if (strcmp(mode, "single-long") == 0) { args = single_dash_long; count = 2; }
  if (strcmp(mode, "single-short") == 0) { args = single_dash_short; count = 2; }
  if (strcmp(mode, "single-short-arg") == 0) { args = single_dash_short_arg; count = 2; }

  int long_index = -1;
  int option;
  optind = 0;
  opterr = 0;
  verbose_flag = 0;
  const char *short_options = strcmp(mode, "missing") == 0 ? ":ab:c" : "ab:c";
  int long_only = strncmp(mode, "single-", 7) == 0;
  while ((option = long_only ? getopt_long_only(count, args, short_options, long_options, &long_index) :
                              getopt_long(count, args, short_options, long_options, &long_index)) != -1)
    printf("%d:%s:%d:%d:%d\n", option, optarg ? optarg : "", optind,
           long_index, verbose_flag);
  printf("end:%d:%d", optind, verbose_flag);
  for (int i = 1; i < count; ++i) printf(":%s", args[i]);
  putchar('\n');
  return 0;
}
