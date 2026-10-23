#include <fcntl.h>
#include <semaphore.h>
#include <stdarg.h>
#include <sys/types.h>

extern sem_t *nelibc_sem_open_impl(const char *name, int oflag,
                                   unsigned int mode, unsigned int value);

sem_t *sem_open(const char *name, int oflag, ...) {
  unsigned int mode = 0;
  unsigned int value = 0;
  if ((oflag & O_CREAT) != 0) {
    va_list arguments;
    va_start(arguments, oflag);
    mode = (unsigned int)va_arg(arguments, mode_t);
    value = va_arg(arguments, unsigned int);
    va_end(arguments);
  }
  return nelibc_sem_open_impl(name, oflag, mode, value);
}
