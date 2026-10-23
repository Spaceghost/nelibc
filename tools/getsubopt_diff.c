#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static void run_case(const char *input) {
  char buffer[128];
  snprintf(buffer, sizeof buffer, "%s", input);
  char *cursor = buffer;
  char *value = NULL;
  char *tokens[] = {"ro", "rw", "exec", NULL};
  do {
    int result = getsubopt(&cursor, tokens, &value);
    printf("%d:%s:%s:%s\n", result, value ? value : "",
           cursor ? cursor : "<null>", buffer);
  } while (cursor != NULL && cursor[0] != '\0');
}

int main(void) {
  run_case("ro=fast,rw,exec=,unknown=x,ro");
  run_case(",ro,");
  return 0;
}
