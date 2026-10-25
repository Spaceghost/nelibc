#include <fcntl.h>
#include <stddef.h>
#include <stdarg.h>
#include <stdint.h>
#include <sys/mman.h>

extern int nelibc_raw_fcntl(int fd, int command, unsigned long argument);
extern int nelibc_fcntl_argument_kind(int command);
extern int nelibc_cancellation_enter(void);
extern void nelibc_cancellation_leave(int previous_type);

static unsigned long fcntl_argument(va_list *arguments, int command) {
  switch (nelibc_fcntl_argument_kind(command)) {
  case 0:
    return 0;
  case 1:
    return (unsigned long)va_arg(*arguments, struct flock *);
  case 2:
    return (unsigned long)va_arg(*arguments, void *);
  case 3:
    return (unsigned long)va_arg(*arguments, uint64_t *);
  default:
    return (unsigned long)va_arg(*arguments, int);
  }
}

int fcntl(int fd, int command, ...) {
  va_list arguments;
  va_start(arguments, command);
  unsigned long argument = fcntl_argument(&arguments, command);
  va_end(arguments);
  if (command == F_SETLKW) {
    int previous_type = nelibc_cancellation_enter();
    int result = nelibc_raw_fcntl(fd, command, argument);
    nelibc_cancellation_leave(previous_type);
    return result;
  }
  return nelibc_raw_fcntl(fd, command, argument);
}

extern void *nelibc_raw_mremap(void *old_address, size_t old_size,
                              size_t new_size, int flags, void *new_address);

void *mremap(void *old_address, size_t old_size, size_t new_size, int flags, ...) {
  void *new_address = NULL;
  if (flags & 2) { /* MREMAP_FIXED, also defined by older system headers as 2. */
    va_list arguments;
    va_start(arguments, flags);
    new_address = va_arg(arguments, void *);
    va_end(arguments);
  }
  return nelibc_raw_mremap(old_address, old_size, new_size, flags, new_address);
}
