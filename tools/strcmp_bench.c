#define _GNU_SOURCE
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#ifndef COMPARE
#define COMPARE strcmp
#endif
extern int COMPARE(const char *,const char *);
static __attribute__((noinline)) int dispatch(const char*a,const char*b) {return COMPARE(a,b);}
static int (*volatile compare_call)(const char*,const char*)=dispatch;
static unsigned char left[65632],right[65632];
static volatile uintptr_t sink;
static double now(void){struct timespec t;clock_gettime(CLOCK_MONOTONIC,&t);return t.tv_sec+t.tv_nsec*1e-9;}
static void loop(const char*a,const char*b,size_t n){uintptr_t sum=0;for(size_t i=0;i<n;++i)sum+=(unsigned)compare_call(a,b);sink^=sum;}
int main(void){
 const size_t sizes[]={0,1,8,15,16,32,64,128,1024,4096,65536};
 const size_t alignments[][2]={{0,0},{1,0},{0,1},{7,15}};
 puts("bytes,left_offset,right_offset,case,ns_per_op,iterations");
 for(size_t k=0;k<sizeof sizes/sizeof *sizes;++k)for(unsigned align=0;align<4;++align)for(unsigned shape=0;shape<4;++shape){
  size_t n=sizes[k];unsigned char *a=left+alignments[align][0],*b=right+alignments[align][1];
  memset(a,'a',n);memset(b,'a',n);a[n]=b[n]=0;
  const char *name=shape==0?"equal":shape==1?"first-mismatch":shape==2?"middle-mismatch":"last-mismatch";
  if(n&&shape)b[shape==1?0:shape==2?n/2:n-1]='b';
  size_t reps=16;double elapsed;
  do{double t=now();loop((char*)a,(char*)b,reps);elapsed=now()-t;if(elapsed<0.002)reps*=2;}while(elapsed<0.002&&reps<(1u<<28));
  double t=now();loop((char*)a,(char*)b,reps);elapsed=now()-t;
  printf("%zu,%zu,%zu,%s,%.9f,%zu\n",n,alignments[align][0],alignments[align][1],name,elapsed*1e9/reps,reps);
 }
 return sink==UINTPTR_MAX;
}
