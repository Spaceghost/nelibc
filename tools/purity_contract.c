/* Compile with -O2 -fno-builtin -Iinclude; inspect external calls. */
#include "string.h"
#include "ctype.h"
#include "nelibc_stdlib.h"
#include "math.h"
int repeated_query(const char *a,const char *b) { return strcmp(a,b)+strcmp(a,b); }
int query_after_store(char *a,const char *b) { int x=strcmp(a,b);a[0]^=1;return x+strcmp(a,b); }
int discard_query(const char *a,const char *b) { (void)strcmp(a,b);return 0; }
int repeated_scalar(int c) { return isalpha(c)+isalpha(c); }
void retain_write(char*a,const char*b) { strcpy(a,b); }
void retain_conversion(const char *a,char **end) { strtol(a,end,10); }
void retain_math(double x) { sin(x); }
int ctype_after_locale(int c) { int x=isalpha(c);setlocale(LC_CTYPE,"C");return x+isalpha(c); }
