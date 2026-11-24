#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include "nelibc_sys.h"

static void mark(char value) {
  if (nelibc_sys3(NELIBC_SYS_WRITE, 1, (unsigned long)&value, 1) != 1)
    _exit(1);
}

/* Observe the stdio hooks while linking the real standalone exit code. */
void nelibc_stdio_init(void) { mark('S'); }
int fflush(FILE *stream) {
  if (stream) _exit(2);
  mark('F');
  return 0;
}

static void preinit(void) { mark('I'); }
__attribute__((used, section(".preinit_array")))
static void (*const preinit_entry)(void) = preinit;

__attribute__((constructor)) static void init(void) { mark('C'); }
__attribute__((destructor(101))) static void fini_first(void) { mark('X'); }
__attribute__((destructor(102))) static void fini_second(void) { mark('Y'); }

static void exit_first(void) { mark('A'); }
static void exit_second(void) { mark('B'); }

int main(void) {
  if (atexit(exit_first) || atexit(exit_second)) return 3;
  mark('M');
  return 42;
}
