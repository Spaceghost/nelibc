#include <fcntl.h>
#include <stdarg.h>
#include <sys/types.h>

struct mq_attr;
extern int __nelibc_mq_open_impl(const char *name, int flags, unsigned int mode,
                                 const struct mq_attr *attributes);

/* POSIX mq_open is variadic only when O_CREAT supplies mode and attributes. */
int mq_open(const char *name, int flags, ...) {
  unsigned int mode = 0;
  const struct mq_attr *attributes = 0;
  if ((flags & O_CREAT) != 0) {
    va_list arguments;
    va_start(arguments, flags);
    mode = va_arg(arguments, mode_t);
    attributes = va_arg(arguments, const struct mq_attr *);
    va_end(arguments);
  }
  return __nelibc_mq_open_impl(name, flags, mode, attributes);
}
