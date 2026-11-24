#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <wchar.h>

int main(void) {
  char text[96], word[32];
  wchar_t wide[96];
  int first = 0, second = 0, count = 0;
  double fraction = 0.0;
  if (snprintf(text, sizeof text, "%*.*s %d/%u %.1f%n", 5, 2, "alpha",
               17, 42u, 2.5, &count) != 15 ||
      count != 15 || strcmp(text, "   al 17/42 2.5")) return 1;
  if (sscanf("11 29 beta 2.5", "%d %d %31s %lf", &first, &second, word, &fraction) != 4 ||
      first != 11 || second != 29 || strcmp(word, "beta") || fraction != 2.5) return 2;
  if (swprintf(wide, sizeof wide / sizeof *wide, L"%d:%ls:%d", 11, L"wide", 29) != 10 ||
      wcscmp(wide, L"11:wide:29")) return 3;
  if (swscanf(L"73 59", L"%d %d", &first, &second) != 2 || first != 73 || second != 59) return 4;
  char *owned = 0;
  if (asprintf(&owned, "%d %s %d", 1, "ok", 2) != 6 || !owned ||
      strcmp(owned, "1 ok 2")) return 5;
  free(owned);
  return 0;
}
