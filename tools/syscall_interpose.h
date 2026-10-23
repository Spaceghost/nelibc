#ifndef NELIBC_SYSCALL_INTERPOSE_H
#define NELIBC_SYSCALL_INTERPOSE_H

#define NELIBC_TEST_EINTR_FLAGS 0xfedcba98u

void nelibc_test_short_write_on_fd(int fd, unsigned long successful_calls);
void nelibc_test_disable_short_write(void);
void nelibc_test_fail_next_mmap(void);

#endif
