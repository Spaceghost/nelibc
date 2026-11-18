#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/mman.h>
#include <unistd.h>
static size_t (*volatile query)(const char *)=strlen;
static unsigned long checks;
static void verify(unsigned char *s,size_t n) {
  memset(s,0xa5,n);s[n]=0;
  if(query((char *)s)!=n) { fprintf(stderr,"strlen mismatch: %zu\n",n);exit(1); }
  ++checks;
}
int main(void) {
  size_t page=(size_t)sysconf(_SC_PAGESIZE);
  unsigned char *area=mmap(NULL,4*page,PROT_NONE,MAP_PRIVATE|MAP_ANONYMOUS,-1,0);
  if(area==MAP_FAILED || mprotect(area+page,2*page,PROT_READ|PROT_WRITE))return 2;
  for(size_t n=0;n<2*page;++n)verify(area+3*page-n-1,n);
  const size_t sizes[]={0,1,7,15,16,31,32,63,64,65,127,128,129,255,256,257,1023,1024,2047,4095};
  for(size_t k=0;k<sizeof sizes/sizeof *sizes;++k)for(size_t offset=0;offset<64;++offset) {
    size_t n=sizes[k];
    verify(area+page+offset,n);
    unsigned char *s=area+3*page-n-offset-1;
    if(s>area+page)s[-1]=0; /* Ignore a NUL before an unaligned input. */
    verify(s,n);
  }
  munmap(area,4*page);
  printf("strlen: %lu high-byte, alignment, boundary-length and guard-page checks pass\n",checks);
  return 0;
}
