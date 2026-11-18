#define _GNU_SOURCE
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <sys/mman.h>
#include <unistd.h>
#include <stdlib.h>
#ifndef COMPARE
#define COMPARE strcmp
#endif
extern int COMPARE(const char *, const char *);
static int (*volatile compare_call)(const char *,const char *)=COMPARE;
static unsigned long checks;
static int reference(const unsigned char *a,const unsigned char *b) {
  while (*a && *a==*b) {++a;++b;} return (int)*a-(int)*b;
}
static void verify(unsigned char *a,unsigned char*b,size_t n) {
  int expected=reference(a,b),actual=compare_call((char*)a,(char*)b);
  ++checks;
  if ((actual>0)-(actual<0)!=(expected>0)-(expected<0)) {
    fprintf(stderr,"strcmp mismatch length=%zu a=%p b=%p actual=%d expected=%d\n",n,(void*)a,(void*)b,actual,expected);exit(1);
  }
}
static void exercise(unsigned char*a,unsigned char*b,size_t n) {
  for(size_t j=0;j<n;++j)a[j]=b[j]=(unsigned char)(1+(j*73)%255);
  a[n]=b[n]=0;verify(a,b,n);verify(b,a,n);
  if(!n)return;
  size_t positions[]={0,n/2,n-1};
  for(unsigned k=0;k<3;++k){
    size_t pos=positions[k];unsigned char saved=b[pos];
    b[pos]^=0x80;verify(a,b,n);verify(b,a,n);
    b[pos]=0;verify(a,b,n);verify(b,a,n);b[pos]=saved;
  }
}
int main(void) {
  static unsigned char left[8352],right[8352];
  const size_t lengths[]={0,1,2,3,7,8,15,16,17,31,32,33,47,48,63,64,65,127,128,129,255,256,257,1023,4095,4096,4097,8191};
  for(size_t k=0;k<sizeof lengths/sizeof *lengths;++k)
    for(size_t x=0;x<32;++x)for(size_t y=0;y<32;++y)exercise(left+x,right+y,lengths[k]);
  size_t page=(size_t)sysconf(_SC_PAGESIZE);
  unsigned char *a=mmap(NULL,4*page,PROT_NONE,MAP_PRIVATE|MAP_ANONYMOUS,-1,0);
  unsigned char *b=mmap(NULL,4*page,PROT_NONE,MAP_PRIVATE|MAP_ANONYMOUS,-1,0);
  if(a==MAP_FAILED||b==MAP_FAILED||mprotect(a+page,2*page,PROT_READ|PROT_WRITE)||mprotect(b+page,2*page,PROT_READ|PROT_WRITE))return 2;
  for(size_t n=0;n<=256;++n)for(size_t shift=0;shift<32;++shift){
    exercise(a+3*page-n-1,b+3*page-n-shift-1,n);
    exercise(a+3*page-n-shift-1,b+3*page-n-1,n);
    exercise(a+page+shift,b+3*page-n-1,n);
    exercise(a+3*page-n-1,b+page+shift,n);
  }
  for(size_t k=0;k<sizeof lengths/sizeof *lengths;++k){
    size_t n=lengths[k];if(n>=2*page)continue;
    exercise(a+3*page-n-1,b+3*page-n-1,n);
  }
  munmap(a,4*page);munmap(b,4*page);
  printf("strcmp: %lu checks; independent alignments, high bytes, early NULs, page crossings and guard pages pass\n",checks);
  return 0;
}
