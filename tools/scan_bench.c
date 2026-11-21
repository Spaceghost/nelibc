#define _GNU_SOURCE
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <time.h>
static _Alignas(64) unsigned char text[65632];
static volatile uintptr_t sink;
static __attribute__((noinline)) size_t length(const char *s) { return strlen(s); }
static __attribute__((noinline)) void *search(const void *s,int c,size_t n) { return memchr(s,c,n); }
static size_t (*volatile length_call)(const char *)=length;
static void *(*volatile search_call)(const void *,int,size_t)=search;
static double now(void) { struct timespec t;clock_gettime(CLOCK_MONOTONIC,&t);return t.tv_sec+t.tv_nsec*1e-9; }
static void loop(unsigned op,unsigned char *s,size_t n,size_t reps) {
  uintptr_t sum=0;
  if(!op)for(size_t i=0;i<reps;++i)sum+=length_call((char *)s);
  else for(size_t i=0;i<reps;++i)sum+=(uintptr_t)search_call(s,0x5a,n);
  sink^=sum;
}
int main(void) {
  const size_t sizes[]={0,1,3,7,8,9,10,11,12,13,14,15,16,31,32,63,64,127,128,256,1024,4096,65536};
  const size_t offsets[]={0,1,7,15,31};
  const char *names[]={"strlen","memchr-absent","memchr-first","memchr-middle","memchr-last"};
  /* Measure steady-state dispatch for every implementation, including short
   * strings. Startup/cold feature-detection cost is outside this benchmark. */
  memset(text,0xa5,4096);text[4096]=0;sink=length_call((char *)text);
  puts("bytes,left_offset,right_offset,case,ns_per_op,iterations");
  for(size_t k=0;k<sizeof sizes/sizeof *sizes;++k)for(unsigned alignment=0;alignment<5;++alignment)
    for(unsigned op=0;op<5;++op) {
      size_t n=sizes[k];unsigned char *s=text+offsets[alignment];
      memset(s,0xa5,n);s[n]=0;
      if(n && op>1)s[op==2?0:op==3?n/2:n-1]=0x5a;
      size_t reps=16;double elapsed;
      do { double t=now();loop(op,s,n,reps);elapsed=now()-t;if(elapsed<0.002)reps*=2; }
      while(elapsed<0.002 && reps<(1u<<28));
      double t=now();loop(op,s,n,reps);elapsed=now()-t;
      printf("%zu,%zu,0,%s,%.9f,%zu\n",n,offsets[alignment],names[op],elapsed*1e9/reps,reps);
    }
  return sink==UINTPTR_MAX;
}
