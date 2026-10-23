#include <stdlib.h>

static volatile int handler_state;

static void mark_exit_handler(void) {
  handler_state = 0x5a;
}

static void verify_exit_handler_order(void) {
  _Exit(handler_state == 0x5a ? 0 : 99);
}

int main(void) {
  if (atexit(verify_exit_handler_order) != 0) return 1;
  if (atexit(mark_exit_handler) != 0) return 2;
  exit(0);
}
