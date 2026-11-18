#define _GNU_SOURCE
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <time.h>

static unsigned char left[65632], right[65632];
static volatile uintptr_t sink;
static __attribute__((noinline)) void *copy(void *a,const void *b,size_t n) { return memcpy(a,b,n); }
static __attribute__((noinline)) void *fill(void *a,int b,size_t n) { return memset(a,b,n); }
static __attribute__((noinline)) int compare(const void *a,const void *b,size_t n) { return memcmp(a,b,n); }
static void *(*volatile copy_call)(void *,const void *,size_t)=copy;
static void *(*volatile fill_call)(void *,int,size_t)=fill;
static int (*volatile compare_call)(const void *,const void *,size_t)=compare;
static double now(void) {
  struct timespec t; clock_gettime(CLOCK_MONOTONIC,&t);
  return t.tv_sec+t.tv_nsec*1e-9;
}
static void loop(unsigned op,unsigned char *a,unsigned char *b,size_t n,size_t reps) {
  uintptr_t sum=0;
  switch(op) {
    case 0: for(size_t i=0;i<reps;++i)sum+=(uintptr_t)copy_call(b,a,n);break;
    case 1: for(size_t i=0;i<reps;++i)sum+=(uintptr_t)fill_call(b,0xa5,n);break;
    default:for(size_t i=0;i<reps;++i)sum+=(unsigned)compare_call(a,b,n);break;
  }
  sink^=sum;
}
int main(void) {
  const size_t sizes[]={0,1,3,7,8,15,16,31,32,63,64,127,128,129,255,256,511,1024,2047,2048,4096,8192,65536};
  const size_t alignments[][2]={{0,0},{1,0},{0,1},{7,15}};
  const char *names[]={"memcpy","memset","memcmp-equal","memcmp-first","memcmp-middle","memcmp-last"};
  puts("bytes,left_offset,right_offset,case,ns_per_op,iterations");
  for(size_t k=0;k<sizeof sizes/sizeof *sizes;++k)
    for(unsigned alignment=0;alignment<4;++alignment)for(unsigned op=0;op<6;++op) {
      size_t n=sizes[k];
      unsigned char *a=left+alignments[alignment][0],*b=right+alignments[alignment][1];
      memset(a,0xa5,n);memset(b,0xa5,n);
      if(n && op>2)b[op==3?0:op==4?n/2:n-1]=0x25;
      size_t reps=16;double elapsed;
      do {
        double t=now();loop(op,a,b,n,reps);elapsed=now()-t;
        if(elapsed<0.002)reps*=2;
      } while(elapsed<0.002 && reps<(1u<<28));
      double t=now();loop(op,a,b,n,reps);elapsed=now()-t;
      printf("%zu,%zu,%zu,%s,%.9f,%zu\n",n,alignments[alignment][0],alignments[alignment][1],names[op],elapsed*1e9/reps,reps);
    }
  return sink==UINTPTR_MAX;
}
