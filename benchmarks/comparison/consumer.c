#define _GNU_SOURCE
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <time.h>
#include <errno.h>
#include <limits.h>
#include <float.h>

/* One unchanged C consumer for each libc +/- the production Nelibc archive.
 * -fno-builtin is required. Timings include the loop and checksum overhead.
 * This is a hot-buffer microbenchmark, not a memory bandwidth benchmark. */
static unsigned char a[131074], b[131074];
static char text[131074], out[131074];
static volatile uintptr_t sink;
/* Volatile pointers to noinline direct-call thunks prevent pure/const call
 * hoisting without taking the address of a libc IFUNC symbol. Cosmopolitan
 * 4.0.2's strstr pointer failed in our toolchain; a direct call works. Use the
 * same thunk arrangement in ALL profiles. Its overhead is not subtracted. */
static __attribute__((noinline)) void *dispatch_memcpy(void *a,const void *b,size_t n) {return memcpy(a,b,n);}
static __attribute__((noinline)) void *dispatch_memmove(void *a,const void *b,size_t n) {return memmove(a,b,n);}
static __attribute__((noinline)) void *dispatch_memset(void *a,int b,size_t n) {return memset(a,b,n);}
static __attribute__((noinline)) int dispatch_memcmp(const void *a,const void *b,size_t n) {return memcmp(a,b,n);}
static __attribute__((noinline)) void *dispatch_memchr(const void *a,int b,size_t n) {return memchr(a,b,n);}
static __attribute__((noinline)) size_t dispatch_strlen(const char *a) {return strlen(a);}
static __attribute__((noinline)) int dispatch_strcmp(const char *a,const char *b) {return strcmp(a,b);}
static __attribute__((noinline)) char *dispatch_strstr(const char *a,const char *b) {return strstr(a,b);}
static __attribute__((noinline)) double dispatch_sin(double a) {return sin(a);}
static __attribute__((noinline)) double dispatch_sqrt(double a) {return sqrt(a);}
static void *(*volatile call_memcpy)(void *, const void *, size_t) = dispatch_memcpy;
static void *(*volatile call_memmove)(void *, const void *, size_t) = dispatch_memmove;
static void *(*volatile call_memset)(void *, int, size_t) = dispatch_memset;
static int (*volatile call_memcmp)(const void *, const void *, size_t) = dispatch_memcmp;
static void *(*volatile call_memchr)(const void *, int, size_t) = dispatch_memchr;
static size_t (*volatile call_strlen)(const char *) = dispatch_strlen;
static int (*volatile call_strcmp)(const char *, const char *) = dispatch_strcmp;
static char *(*volatile call_strstr)(const char *, const char *) = dispatch_strstr;
static double (*volatile call_sin)(double) = dispatch_sin;
static double (*volatile call_sqrt)(double) = dispatch_sqrt;
static double seconds(void) {
  struct timespec ts;
  if (clock_gettime(CLOCK_MONOTONIC, &ts)) exit(2);
  return ts.tv_sec + ts.tv_nsec * 1e-9;
}
static int cmpint(const void *a, const void *b) {
  int x = *(const int *)a, y = *(const int *)b;
  return (x > y) - (x < y);
}
static uintptr_t parse_record(void) {
  char line[256], *save, *end;
  static const char record[] = "alpha,12345,3.141592653589793,needle=value";
  memcpy(line, record, sizeof record);
  char *name = strtok_r(line, ",", &save);
  char *integer = strtok_r(NULL, ",", &save);
  char *decimal = strtok_r(NULL, ",", &save);
  char *value = strtok_r(NULL, ",", &save);
  return strlen(name) + strtoul(integer, &end, 10) +
    (uintptr_t)(strtod(decimal, &end) * 1000) + (strstr(value, "value") != NULL);
}
#define LOOP(expr) for (size_t i=0; i<reps; ++i) { sum += (uintptr_t)(expr); }
static uintptr_t run(unsigned op, size_t n, size_t reps) {
  uintptr_t sum = 0;
  char *end;
  switch (op) {
    case 0: LOOP(call_memcpy(b, a, n)); break;
    case 1: LOOP(call_memmove(b + 1, b, n)); break;
    case 2: LOOP(call_memset(b, 0x61, n)); break;
    case 3: LOOP(call_memcmp(a, b, n)); break;
    case 4: LOOP(call_memchr(a, 0xff, n)); break;
    case 5: LOOP(call_strlen(text)); break;
    case 6: LOOP(call_strcmp(text, out)); break;
    case 7: LOOP(call_strstr(text, "aaaaz")); break;
    case 8: LOOP(strtol("123456789", &end, 10)); break;
    case 9: LOOP((strtod("3.141592653589793", &end) * 1000)); break;
    case 10: LOOP((call_sin(0.5 + (i & 1) * 0.001) * 1000 + 1000)); break;
    case 11: LOOP((call_sqrt(81.0 + (i & 1)) * 1000)); break;
    case 12:
      for (size_t i=0; i<reps; ++i) {
        unsigned char *p = malloc(n);
        if (!p) exit(3);
        p[0]=(unsigned char)i; p[n-1]=(unsigned char)(i+1);
        sum += p[0] + p[n-1]; free(p);
      } break;
    case 13: LOOP(snprintf(out, sizeof out, "%s:%ld:%.6f", "value", 12345L, 3.141592)); break;
    case 14:
      for (size_t i=0; i<reps; ++i) {
        int values[128];
        for (size_t j=0; j<128; ++j) values[j]=(int)((j*73+19)%128);
        qsort(values,128,sizeof(int),cmpint); sum += values[0] + values[127];
      } break;
    case 15: LOOP(parse_record()); break;
  }
  sink ^= sum;
  return sum;
}
static void prepare(size_t n) {
  memset(a, 'a', sizeof a); memset(b, 'a', sizeof b);
  memset(text, 'a', sizeof text); memset(out, 'a', sizeof out);
  text[n]='\0'; out[n]='\0';
  if (n) { text[n-1]='z'; out[n-1]='z'; }
}
static void bench(const char *name, const char *family, unsigned op, size_t n) {
  prepare(n);
  size_t reps=16;
  double elapsed;
  do {
    double start=seconds(); run(op,n,reps); elapsed=seconds()-start;
    if (elapsed < 0.005) reps *= 2;
  } while (elapsed < 0.005 && reps < (1u<<28));
  double start=seconds(); run(op,n,reps); elapsed=seconds()-start;
  printf("%s,%s,%zu,%.9f,%zu\n",name,family,n,elapsed*1e9/reps,reps);
}
static unsigned failures, checks;
#define CHECK(expr) do { ++checks; if (!(expr)) { ++failures; fprintf(stderr,"check failed at %d: %s\n",__LINE__,#expr); } } while(0)
static int check(void) {
  const size_t sizes[]={0,1,2,7,8,15,16,31,32,63,64,127,128,255,256,1023,4096,65536};
  for (size_t k=0;k<sizeof sizes/sizeof *sizes;++k) for(size_t off=0;off<8;++off) {
    size_t n=sizes[k];
    for(size_t i=0;i<n+32;++i) a[i]=(unsigned char)(i*73+11);
    call_memset(b,0xcc,n+32);
    CHECK(call_memcpy(b+off,a+off,n)==b+off);
    int equal=1;
    for(size_t i=0;i<n;++i) if(b[off+i]!=a[off+i]) equal=0;
    CHECK(equal);
    CHECK(b[off+n]==0xcc);
    CHECK(call_memcmp(a+off,b+off,n)==0);
    CHECK(call_memmove(b+off+1,b+off,n)==b+off+1);
    equal=1;
    for(size_t i=0;i<n;++i) if(b[off+1+i]!=a[off+i]) equal=0;
    CHECK(equal);
    CHECK(call_memset(b+off,0x3d,n)==b+off);
    equal=1;
    for(size_t i=0;i<n;++i) if(b[off+i]!=0x3d) equal=0;
    CHECK(equal);
    CHECK(call_memchr(b+off,0xff,n)==NULL);
    if(n) CHECK(call_memchr(b+off,0x3d,n)==b+off);
    prepare(n);
    CHECK(call_strlen(text)==n);
    CHECK(call_strcmp(text,out)==0);
    CHECK(call_strstr(text,"")==text);
    if(n>=5) CHECK(call_strstr(text,"aaaaz")==text+n-5);
  }
  char *end;
  CHECK(strtol(" -123tail",&end,10)==-123 && call_strcmp(end,"tail")==0);
  errno=0; CHECK(strtol("99999999999999999999999999999999",&end,10)==LONG_MAX && errno==ERANGE);
  CHECK(strtod("12.5rest",&end)==12.5 && call_strcmp(end,"rest")==0);
  errno=0; CHECK(isinf(strtod("1e9999",&end)) && errno==ERANGE);
  CHECK(fabs(call_sin(0.5)-0.479425538604203)<1e-14);
  CHECK(call_sqrt(81)==9);
  CHECK(parse_record()==15492);
  void *p=malloc(128); CHECK(p!=NULL); free(p);
  CHECK(snprintf(out,sizeof out,"%s:%ld:%.6f","value",12345L,3.141592)==20);
  CHECK(call_strcmp(out,"value:12345:3.141592")==0);
  int nums[]={3,1,2};qsort(nums,3,sizeof(int),cmpint);CHECK(nums[0]==1 && nums[1]==2 && nums[2]==3);
  printf("{\"checks\":%u,\"failures\":%u}\n",checks,failures);
  return failures!=0;
}
int main(int argc,char **argv) {
  if(argc>1 && strcmp(argv[1],"--check")==0) return check();
  puts("case,family,bytes,ns_per_op,iterations");
  const char *names[]={"memcpy","memmove-overlap","memset","memcmp-equal","memchr-absent","strlen","strcmp-equal","strstr-tail"};
  const size_t sizes[]={8,32,128,1024,4096,65536};
  for(unsigned op=0;op<8;++op) for(size_t k=0;k<sizeof sizes/sizeof *sizes;++k) bench(names[op],op<5?"memory":"strings",op,sizes[k]);
  bench("strtol-decimal","conversion",8,0);bench("strtod-decimal","conversion",9,0);
  bench("sin","math",10,0);bench("sqrt","math",11,0);
  bench("malloc-free","allocation",12,64);bench("malloc-free","allocation",12,4096);
  bench("snprintf-mixed","formatting",13,0);bench("qsort-128-integers","sorting",14,0);
  bench("parse-record","workload",15,0);
  return sink==UINTPTR_MAX;
}
