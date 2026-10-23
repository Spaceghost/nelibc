#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

static void write_a(void) { (void)write(STDOUT_FILENO, "A", 1); }
static void write_b(void) { (void)write(STDOUT_FILENO, "B", 1); }
static void write_c(void) { (void)write(STDOUT_FILENO, "C", 1); }
static void no_op(void) {}

int main(int argc, char **argv) {
  if (argc == 2 && strcmp(argv[1], "--abort") == 0) {
    pid_t child = fork();
    if (child < 0) return 46;
    if (child == 0) {
      abort();
      _Exit(47);
    }
    int status = 0;
    if (waitpid(child, &status, 0) != child || (status & 0x7f) != 6)
      return 48;
    return 0;
  }
  if (argc == 2 && strcmp(argv[1], "--_Exit") == 0) {
    fputs("X", stdout);
    _Exit(42);
  }
  if (argc == 2 && strcmp(argv[1], "--capacity") == 0) {
    for (int i = 0; i < 32; ++i) {
      if (atexit(no_op) != 0) return 43;
    }
    if (atexit(no_op) == 0) return 44;
    _Exit(0);
  }

  fputs("F", stdout);
  if (atexit(write_a) != 0 || atexit(write_b) != 0 || atexit(write_c) != 0)
    return 45;
  if (argc == 2 && strcmp(argv[1], "--return") == 0) return 0;
  exit(0);
}
