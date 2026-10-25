#define _GNU_SOURCE
#include <errno.h>
#include <fcntl.h>
#include <mqueue.h>
#include <pthread.h>
#include <poll.h>
#include <signal.h>
#include <semaphore.h>
#include <stdio.h>
#include <stdlib.h>
#include <stddef.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/mman.h>
#include <sys/epoll.h>
#include <sys/stat.h>
#include <sys/un.h>
#include <sys/uio.h>
#include <sys/wait.h>
#include <termios.h>
#include <time.h>
#include <unistd.h>

static int read_fd;
static int read_started;
static int write_fd;
static int write_started;
static int readv_fd;
static int readv_started;
static int writev_fd;
static int writev_started;
static int listen_fd;
static int accept_started;
static int recv_fd;
static int recv_started;
static int recvfrom_started;
static int recvmsg_started;
static int send_fd;
static int send_started;
static int sendmsg_started;
static int nanosleep_started;
static int clock_nanosleep_started;
static int pause_started;
static int lock_wait_fd;
static int lock_wait_started;
static int epoll_fd;
static int epoll_wait_started;
static int epoll_pwait_started;
static int epoll_pwait2_started;
static int poll_fd;
static int poll_started;
static int ppoll_started;
static int select_started;
static int pselect_started;
static pid_t wait_child;
static int wait_started_flag;
static int waitid_started_flag;
static sigset_t signal_wait_set;
static int sigwait_started_flag;
static int sigwaitinfo_started_flag;
static int sigtimedwait_started_flag;
static int sigsuspend_started_flag;
static mqd_t mq_descriptor;
static struct timespec mq_deadline;
static int mq_receive_started_flag;
static int mq_timedreceive_started_flag;
static int mq_send_started_flag;
static int mq_timedsend_started_flag;
static sem_t mq_thread_notification_sem;

static void mq_thread_notification_callback(union sigval value) {
  sem_post((sem_t *)value.sival_ptr);
}
static char fifo_path[128];
static int open_started;
static int sync_fd;
static int sync_started;
static pthread_mutex_t start_mutex = PTHREAD_MUTEX_INITIALIZER;
static pthread_cond_t start_condition = PTHREAD_COND_INITIALIZER;
static struct {
  int *flag;
  pid_t tid;
} started_threads[64];

extern pid_t gettid(void);

static void mark_started(int *started) {
  pid_t tid = gettid();
  pthread_mutex_lock(&start_mutex);
  size_t slot = 0;
  while (slot < sizeof(started_threads) / sizeof(started_threads[0]) &&
         started_threads[slot].flag != NULL &&
         started_threads[slot].flag != started)
    ++slot;
  if (slot < sizeof(started_threads) / sizeof(started_threads[0])) {
    started_threads[slot].flag = started;
    started_threads[slot].tid = tid;
  }
  *started = 1;
  pthread_cond_signal(&start_condition);
  pthread_mutex_unlock(&start_mutex);
}

static void wait_started(int *started) {
  pthread_mutex_lock(&start_mutex);
  while (!*started) pthread_cond_wait(&start_condition, &start_mutex);
  pthread_mutex_unlock(&start_mutex);
}

static void *blocked_reader(void *unused) {
  (void)unused;
  char byte;
  mark_started(&read_started);
  (void)read(read_fd, &byte, 1);
  return (void *)1;
}

static int wait_until_syscall_either(pid_t tid, long expected_syscall,
                                    long alternate_syscall) {
  char path[64];
  char contents[128];
  if (tid <= 0 || snprintf(path, sizeof(path), "/proc/self/task/%ld/syscall",
                           (long)tid) >= (int)sizeof(path))
    return EINVAL;
  const struct timespec pause = {0, 1000000};
  for (unsigned int attempt = 0; attempt < 5000U; ++attempt) {
    int fd = open(path, O_RDONLY | O_CLOEXEC);
    if (fd < 0) {
      if (errno == EACCES || errno == EPERM || errno == ENOENT) return ENOTSUP;
    } else {
      ssize_t amount = read(fd, contents, sizeof(contents) - 1);
      close(fd);
      if (amount > 0) {
        contents[amount] = '\0';
        char *end = NULL;
        long actual_syscall = strtol(contents, &end, 10);
        if (end != contents && (actual_syscall == expected_syscall ||
                                actual_syscall == alternate_syscall))
          return 0;
      }
    }
    if (nanosleep(&pause, NULL) != 0 && errno != EINTR) return errno;
  }
  return ETIMEDOUT;
}

static int wait_until_syscall(pid_t tid, long expected_syscall) {
  return wait_until_syscall_either(tid, expected_syscall, -1);
}

static int wait_until_started_syscall(int *started, long expected_syscall) {
  pthread_mutex_lock(&start_mutex);
  size_t slot = 0;
  while (slot < sizeof(started_threads) / sizeof(started_threads[0]) &&
         started_threads[slot].flag != started)
    ++slot;
  pid_t tid = slot < sizeof(started_threads) / sizeof(started_threads[0])
      ? started_threads[slot].tid : 0;
  pthread_mutex_unlock(&start_mutex);
  return wait_until_syscall(tid, expected_syscall);
}

static int wait_for_blocked_syscall(int *started, long expected_syscall,
                                    const struct timespec *fallback_delay) {
  int result = wait_until_started_syscall(started, expected_syscall);
  if (result == ENOTSUP)
    return nanosleep(fallback_delay, NULL) == 0 ? 0 : errno;
  return result;
}

static int wait_for_blocked_syscall_either(int *started, long expected_syscall,
                                           long alternate_syscall,
                                           const struct timespec *fallback_delay) {
  pthread_mutex_lock(&start_mutex);
  size_t slot = 0;
  while (slot < sizeof(started_threads) / sizeof(started_threads[0]) &&
         started_threads[slot].flag != started)
    ++slot;
  pid_t tid = slot < sizeof(started_threads) / sizeof(started_threads[0])
      ? started_threads[slot].tid : 0;
  pthread_mutex_unlock(&start_mutex);
  int result = wait_until_syscall_either(tid, expected_syscall, alternate_syscall);
  if (result == ENOTSUP)
    return nanosleep(fallback_delay, NULL) == 0 ? 0 : errno;
  return result;
}

static void *blocked_writer(void *unused) {
  (void)unused;
  char byte = 'x';
  mark_started(&write_started);
  (void)write(write_fd, &byte, 1);
  return (void *)1;
}

static void *blocked_readv_worker(void *unused) {
  (void)unused;
  char byte;
  struct iovec vector = {&byte, 1};
  mark_started(&readv_started);
  return (void *)(long)readv(readv_fd, &vector, 1);
}

static void *blocked_writev_worker(void *unused) {
  (void)unused;
  char byte = 'x';
  struct iovec vector = {&byte, 1};
  mark_started(&writev_started);
  return (void *)(long)writev(writev_fd, &vector, 1);
}

static void *blocked_acceptor(void *unused) {
  (void)unused;
  mark_started(&accept_started);
  return (void *)(long)accept(listen_fd, NULL, NULL);
}

static void *blocked_accept4(void *unused) {
  (void)unused;
  mark_started(&accept_started);
  return (void *)(long)accept4(listen_fd, NULL, NULL, SOCK_CLOEXEC);
}

static void *blocked_receiver(void *unused) {
  (void)unused;
  char byte;
  mark_started(&recv_started);
  return (void *)(long)recv(recv_fd, &byte, 1, 0);
}

static void *blocked_recvfrom_worker(void *unused) {
  (void)unused;
  char byte;
  mark_started(&recvfrom_started);
  return (void *)(long)recvfrom(recv_fd, &byte, 1, 0, NULL, NULL);
}

static void *blocked_recvmsg_worker(void *unused) {
  (void)unused;
  char byte;
  struct iovec vector = {&byte, 1};
  struct msghdr message = {0};
  message.msg_iov = &vector;
  message.msg_iovlen = 1;
  mark_started(&recvmsg_started);
  return (void *)(long)recvmsg(recv_fd, &message, 0);
}

static void *blocked_sender(void *unused) {
  (void)unused;
  char byte = 'x';
  mark_started(&send_started);
  return (void *)(long)send(send_fd, &byte, 1, MSG_NOSIGNAL);
}

static void *blocked_sendmsg_worker(void *unused) {
  (void)unused;
  char byte = 'x';
  struct iovec vector = {&byte, 1};
  struct msghdr message = {0};
  message.msg_iov = &vector;
  message.msg_iovlen = 1;
  mark_started(&sendmsg_started);
  return (void *)(long)sendmsg(send_fd, &message, MSG_NOSIGNAL);
}

static void *blocked_nanosleep_worker(void *unused) {
  (void)unused;
  struct timespec request = {3600, 0};
  mark_started(&nanosleep_started);
  return (void *)(long)nanosleep(&request, NULL);
}

static void *blocked_clock_nanosleep_worker(void *unused) {
  (void)unused;
  struct timespec request = {3600, 0};
  mark_started(&clock_nanosleep_started);
  return (void *)(long)clock_nanosleep(CLOCK_MONOTONIC, 0, &request, NULL);
}

static void *blocked_pauser(void *unused) {
  (void)unused;
  mark_started(&pause_started);
  return (void *)(long)pause();
}

static void *blocked_epoll_wait_worker(void *unused) {
  (void)unused;
  struct epoll_event event;
  mark_started(&epoll_wait_started);
  return (void *)(long)epoll_wait(epoll_fd, &event, 1, -1);
}

static void *blocked_epoll_pwait_worker(void *unused) {
  (void)unused;
  struct epoll_event event;
  mark_started(&epoll_pwait_started);
  return (void *)(long)epoll_pwait(epoll_fd, &event, 1, -1, NULL);
}

static void *blocked_epoll_pwait2_worker(void *unused) {
  (void)unused;
  struct epoll_event event;
  mark_started(&epoll_pwait2_started);
  return (void *)(long)epoll_pwait2(epoll_fd, &event, 1, NULL, NULL);
}

static void *blocked_record_locker(void *unused) {
  (void)unused;
  struct flock lock = {0};
  lock.l_type = F_WRLCK;
  lock.l_whence = SEEK_SET;
  mark_started(&lock_wait_started);
  return (void *)(long)fcntl(lock_wait_fd, F_SETLKW, &lock);
}

static int fill_socket_send_buffer(int fd) {
  int flags = fcntl(fd, F_GETFL);
  if (flags < 0 || fcntl(fd, F_SETFL, flags | O_NONBLOCK) != 0) return -1;
  char fill[4096] = {0};
  for (;;) {
    ssize_t count = send(fd, fill, sizeof fill, MSG_DONTWAIT | MSG_NOSIGNAL);
    if (count > 0) continue;
    if (errno == EAGAIN || errno == EWOULDBLOCK) break;
    return -1;
  }
  return fcntl(fd, F_SETFL, flags & ~O_NONBLOCK);
}

static void *blocked_poller(void *unused) {
  (void)unused;
  struct pollfd descriptor = {poll_fd, POLLIN, 0};
  mark_started(&poll_started);
  return (void *)(long)poll(&descriptor, 1, -1);
}

static void *blocked_ppoller(void *unused) {
  (void)unused;
  struct pollfd descriptor = {poll_fd, POLLIN, 0};
  mark_started(&ppoll_started);
  return (void *)(long)ppoll(&descriptor, 1, NULL, NULL);
}

static void *blocked_selector(void *unused) {
  (void)unused;
  fd_set read_set;
  FD_ZERO(&read_set);
  FD_SET(poll_fd, &read_set);
  mark_started(&select_started);
  return (void *)(long)select(poll_fd + 1, &read_set, NULL, NULL, NULL);
}

static void *blocked_pselector(void *unused) {
  (void)unused;
  fd_set read_set;
  FD_ZERO(&read_set);
  FD_SET(poll_fd, &read_set);
  mark_started(&pselect_started);
  return (void *)(long)pselect(poll_fd + 1, &read_set, NULL, NULL, NULL, NULL);
}

static void *blocked_waiter(void *unused) {
  (void)unused;
  mark_started(&wait_started_flag);
  return (void *)(long)waitpid(wait_child, NULL, 0);
}

static void *blocked_waitid_worker(void *unused) {
  (void)unused;
  siginfo_t info = {0};
  mark_started(&waitid_started_flag);
  return (void *)(long)waitid(P_PID, (id_t)wait_child, &info, WEXITED);
}

static void *blocked_sigwait_worker(void *unused) {
  (void)unused;
  int signal = 0;
  mark_started(&sigwait_started_flag);
  int result = sigwait(&signal_wait_set, &signal);
  return (void *)(long)(result == 0 ? signal : -result);
}

static void *blocked_sigwaitinfo_worker(void *unused) {
  (void)unused;
  siginfo_t info = {0};
  mark_started(&sigwaitinfo_started_flag);
  return (void *)(long)sigwaitinfo(&signal_wait_set, &info);
}

static void *blocked_sigtimedwait_worker(void *unused) {
  (void)unused;
  siginfo_t info = {0};
  struct timespec timeout = {3600, 0};
  mark_started(&sigtimedwait_started_flag);
  return (void *)(long)sigtimedwait(&signal_wait_set, &info, &timeout);
}

static void *blocked_sigsuspend_worker(void *unused) {
  (void)unused;
  sigset_t mask;
  sigemptyset(&mask);
  mark_started(&sigsuspend_started_flag);
  return (void *)(long)sigsuspend(&mask);
}

static void *self_cancel_readlink_worker(void *unused) {
  (void)unused;
  char target[4096];
  if (pthread_cancel(pthread_self()) != 0) return (void *)1;
  (void)readlink("/proc/self/exe", target, sizeof(target));
  return NULL;
}

static void *self_cancel_msync_worker(void *unused) {
  (void)unused;
  if (pthread_cancel(pthread_self()) != 0) return (void *)1;
  (void)msync(NULL, 0, MS_SYNC);
  return NULL;
}

static void *self_cancel_tcdrain_worker(void *unused) {
  (void)unused;
  if (pthread_cancel(pthread_self()) != 0) return (void *)1;
  (void)tcdrain(-1);
  return NULL;
}

static void *blocked_mq_receive_worker(void *unused) {
  (void)unused;
  char message[128];
  unsigned int priority;
  mark_started(&mq_receive_started_flag);
  return (void *)(long)mq_receive(mq_descriptor, message, sizeof(message), &priority);
}

static void *blocked_mq_timedreceive_worker(void *unused) {
  (void)unused;
  char message[128];
  unsigned int priority;
  mark_started(&mq_timedreceive_started_flag);
  return (void *)(long)mq_timedreceive(mq_descriptor, message, sizeof(message),
                                       &priority, &mq_deadline);
}

static void *blocked_mq_send_worker(void *unused) {
  (void)unused;
  mark_started(&mq_send_started_flag);
  return (void *)(long)mq_send(mq_descriptor, "blocked", 7, 0);
}

static void *blocked_mq_timedsend_worker(void *unused) {
  (void)unused;
  mark_started(&mq_timedsend_started_flag);
  return (void *)(long)mq_timedsend(mq_descriptor, "blocked", 7, 0, &mq_deadline);
}

static void *blocked_opener(void *unused) {
  (void)unused;
  mark_started(&open_started);
  return (void *)(long)open(fifo_path, O_RDONLY, 0);
}

static void *sync_worker(void *unused) {
  (void)unused;
  mark_started(&sync_started);
  for (;;) {
    if (fsync(sync_fd) != 0) return (void *)1;
  }
}

int main(void) {
  int pipe_fds[2];
  if (pipe(pipe_fds) != 0) return 1;
  read_fd = pipe_fds[0];
  pthread_t reader;
  if (pthread_create(&reader, NULL, blocked_reader, NULL) != 0) return 2;
  wait_started(&read_started);
  struct timespec delay = {0, 20000000};
  if (wait_for_blocked_syscall(&read_started, 0, &delay) != 0) return 108;
  alarm(5);
  if (pthread_cancel(reader) != 0) return 3;
  void *result = NULL;
  if (pthread_join(reader, &result) != 0 || result != PTHREAD_CANCELED) return 4;
  alarm(0);
  close(pipe_fds[0]);
  close(pipe_fds[1]);

  if (pipe(pipe_fds) != 0) return 66;
  readv_fd = pipe_fds[0];
  pthread_t readv_thread;
  if (pthread_create(&readv_thread, NULL, blocked_readv_worker, NULL) != 0) return 67;
  wait_started(&readv_started);
  if (wait_for_blocked_syscall(&readv_started, 19, &delay) != 0) return 68;
  alarm(5);
  if (pthread_cancel(readv_thread) != 0) return 68;
  result = NULL;
  if (pthread_join(readv_thread, &result) != 0 || result != PTHREAD_CANCELED) return 69;
  alarm(0);
  close(pipe_fds[0]);
  close(pipe_fds[1]);

  if (pipe(pipe_fds) != 0) return 5;
  int flags = fcntl(pipe_fds[1], F_GETFL);
  if (flags < 0 || fcntl(pipe_fds[1], F_SETFL, flags | O_NONBLOCK) != 0)
    return 6;
  char fill[4096] = {0};
  for (;;) {
    ssize_t count = write(pipe_fds[1], fill, sizeof fill);
    if (count > 0) continue;
    if (errno == EAGAIN || errno == EWOULDBLOCK) break;
    return 7;
  }
  if (fcntl(pipe_fds[1], F_SETFL, flags & ~O_NONBLOCK) != 0) return 8;
  write_fd = pipe_fds[1];
  pthread_t writer;
  if (pthread_create(&writer, NULL, blocked_writer, NULL) != 0) return 9;
  wait_started(&write_started);
  if (wait_for_blocked_syscall(&write_started, 1, &delay) != 0) return 10;
  alarm(5);
  if (pthread_cancel(writer) != 0) return 10;
  result = NULL;
  if (pthread_join(writer, &result) != 0 || result != PTHREAD_CANCELED) return 11;
  alarm(0);
  close(pipe_fds[0]);
  close(pipe_fds[1]);

  if (pipe(pipe_fds) != 0) return 70;
  flags = fcntl(pipe_fds[1], F_GETFL);
  if (flags < 0 || fcntl(pipe_fds[1], F_SETFL, flags | O_NONBLOCK) != 0)
    return 71;
  for (;;) {
    ssize_t count = write(pipe_fds[1], fill, sizeof fill);
    if (count > 0) continue;
    if (errno == EAGAIN || errno == EWOULDBLOCK) break;
    return 72;
  }
  if (fcntl(pipe_fds[1], F_SETFL, flags & ~O_NONBLOCK) != 0) return 73;
  writev_fd = pipe_fds[1];
  pthread_t writev_thread;
  if (pthread_create(&writev_thread, NULL, blocked_writev_worker, NULL) != 0) return 74;
  wait_started(&writev_started);
  if (wait_for_blocked_syscall(&writev_started, 20, &delay) != 0) return 75;
  alarm(5);
  if (pthread_cancel(writev_thread) != 0) return 75;
  result = NULL;
  if (pthread_join(writev_thread, &result) != 0 || result != PTHREAD_CANCELED) return 76;
  alarm(0);
  close(pipe_fds[0]);
  close(pipe_fds[1]);

  listen_fd = socket(AF_UNIX, SOCK_STREAM, 0);
  if (listen_fd < 0) return 12;
  struct sockaddr_un address = {0};
  address.sun_family = AF_UNIX;
  const char name[] = "nelibc-cancel-accept";
  memcpy(address.sun_path + 1, name, sizeof name);
  socklen_t address_length = (socklen_t)(offsetof(struct sockaddr_un, sun_path) + sizeof name);
  if (bind(listen_fd, (struct sockaddr *)&address, address_length) != 0 || listen(listen_fd, 1) != 0)
    return 13;
  pthread_t acceptor;
  if (pthread_create(&acceptor, NULL, blocked_acceptor, NULL) != 0) return 14;
  wait_started(&accept_started);
  if (wait_for_blocked_syscall(&accept_started, 43, &delay) != 0) return 15;
  alarm(5);
  if (pthread_cancel(acceptor) != 0) return 15;
  result = NULL;
  if (pthread_join(acceptor, &result) != 0 || result != PTHREAD_CANCELED) return 16;
  alarm(0);
  accept_started = 0;
  if (pthread_create(&acceptor, NULL, blocked_accept4, NULL) != 0) return 17;
  wait_started(&accept_started);
  if (wait_for_blocked_syscall(&accept_started, 288, &delay) != 0) return 18;
  alarm(5);
  if (pthread_cancel(acceptor) != 0) return 18;
  result = NULL;
  if (pthread_join(acceptor, &result) != 0 || result != PTHREAD_CANCELED) return 19;
  alarm(0);
  close(listen_fd);

  int sockets[2];
  if (socketpair(AF_UNIX, SOCK_STREAM, 0, sockets) != 0) return 20;
  recv_fd = sockets[0];
  pthread_t receiver;
  if (pthread_create(&receiver, NULL, blocked_receiver, NULL) != 0) return 21;
  wait_started(&recv_started);
  if (wait_for_blocked_syscall(&recv_started, 45, &delay) != 0) return 22;
  alarm(5);
  if (pthread_cancel(receiver) != 0) return 22;
  result = NULL;
  if (pthread_join(receiver, &result) != 0 || result != PTHREAD_CANCELED) return 23;
  alarm(0);
  close(sockets[0]);
  close(sockets[1]);

  if (socketpair(AF_UNIX, SOCK_STREAM, 0, sockets) != 0) return 24;
  poll_fd = sockets[0];
  pthread_t poller;
  if (pthread_create(&poller, NULL, blocked_poller, NULL) != 0) return 25;
  wait_started(&poll_started);
  if (wait_for_blocked_syscall(&poll_started, 7, &delay) != 0) return 26;
  alarm(5);
  if (pthread_cancel(poller) != 0) return 26;
  result = NULL;
  if (pthread_join(poller, &result) != 0 || result != PTHREAD_CANCELED) return 27;
  alarm(0);
  close(sockets[0]);
  close(sockets[1]);

  if (socketpair(AF_UNIX, SOCK_STREAM, 0, sockets) != 0) return 46;
  poll_fd = sockets[0];
  pthread_t ppoller;
  if (pthread_create(&ppoller, NULL, blocked_ppoller, NULL) != 0) return 47;
  wait_started(&ppoll_started);
  if (wait_for_blocked_syscall(&ppoll_started, 271, &delay) != 0) return 48;
  alarm(5);
  if (pthread_cancel(ppoller) != 0) return 48;
  result = NULL;
  if (pthread_join(ppoller, &result) != 0 || result != PTHREAD_CANCELED) return 49;
  alarm(0);
  close(sockets[0]);
  close(sockets[1]);

  if (socketpair(AF_UNIX, SOCK_STREAM, 0, sockets) != 0) return 50;
  poll_fd = sockets[0];
  pthread_t selector;
  if (pthread_create(&selector, NULL, blocked_selector, NULL) != 0) return 51;
  wait_started(&select_started);
  if (wait_for_blocked_syscall_either(&select_started, 23, 270, &delay) != 0)
    return 52;
  alarm(5);
  if (pthread_cancel(selector) != 0) return 52;
  result = NULL;
  if (pthread_join(selector, &result) != 0 || result != PTHREAD_CANCELED) return 53;
  alarm(0);
  close(sockets[0]);
  close(sockets[1]);

  if (socketpair(AF_UNIX, SOCK_STREAM, 0, sockets) != 0) return 54;
  poll_fd = sockets[0];
  pthread_t pselector;
  if (pthread_create(&pselector, NULL, blocked_pselector, NULL) != 0) return 55;
  wait_started(&pselect_started);
  if (wait_for_blocked_syscall(&pselect_started, 270, &delay) != 0) return 56;
  alarm(5);
  if (pthread_cancel(pselector) != 0) return 56;
  result = NULL;
  if (pthread_join(pselector, &result) != 0 || result != PTHREAD_CANCELED) return 57;
  alarm(0);
  close(sockets[0]);
  close(sockets[1]);

  if (socketpair(AF_UNIX, SOCK_STREAM, 0, sockets) != 0) return 58;
  recv_fd = sockets[0];
  pthread_t recvfrom_thread;
  if (pthread_create(&recvfrom_thread, NULL, blocked_recvfrom_worker, NULL) != 0) return 59;
  wait_started(&recvfrom_started);
  if (wait_for_blocked_syscall(&recvfrom_started, 45, &delay) != 0) return 60;
  alarm(5);
  if (pthread_cancel(recvfrom_thread) != 0) return 60;
  result = NULL;
  if (pthread_join(recvfrom_thread, &result) != 0 || result != PTHREAD_CANCELED) return 61;
  alarm(0);
  close(sockets[0]);
  close(sockets[1]);

  if (socketpair(AF_UNIX, SOCK_STREAM, 0, sockets) != 0) return 62;
  recv_fd = sockets[0];
  pthread_t recvmsg_thread;
  if (pthread_create(&recvmsg_thread, NULL, blocked_recvmsg_worker, NULL) != 0) return 63;
  wait_started(&recvmsg_started);
  if (wait_for_blocked_syscall(&recvmsg_started, 47, &delay) != 0) return 64;
  alarm(5);
  if (pthread_cancel(recvmsg_thread) != 0) return 64;
  result = NULL;
  if (pthread_join(recvmsg_thread, &result) != 0 || result != PTHREAD_CANCELED) return 65;
  alarm(0);
  close(sockets[0]);
  close(sockets[1]);

  if (socketpair(AF_UNIX, SOCK_STREAM, 0, sockets) != 0) return 77;
  send_fd = sockets[0];
  if (fill_socket_send_buffer(send_fd) != 0) return 78;
  pthread_t sender;
  if (pthread_create(&sender, NULL, blocked_sender, NULL) != 0) return 79;
  wait_started(&send_started);
  if (wait_for_blocked_syscall(&send_started, 44, &delay) != 0) return 80;
  alarm(5);
  if (pthread_cancel(sender) != 0) return 80;
  result = NULL;
  if (pthread_join(sender, &result) != 0 || result != PTHREAD_CANCELED) return 81;
  alarm(0);
  close(sockets[0]);
  close(sockets[1]);

  if (socketpair(AF_UNIX, SOCK_STREAM, 0, sockets) != 0) return 82;
  send_fd = sockets[0];
  if (fill_socket_send_buffer(send_fd) != 0) return 83;
  pthread_t sendmsg_thread;
  if (pthread_create(&sendmsg_thread, NULL, blocked_sendmsg_worker, NULL) != 0) return 84;
  wait_started(&sendmsg_started);
  if (wait_for_blocked_syscall(&sendmsg_started, 46, &delay) != 0) return 85;
  alarm(5);
  if (pthread_cancel(sendmsg_thread) != 0) return 85;
  result = NULL;
  if (pthread_join(sendmsg_thread, &result) != 0 || result != PTHREAD_CANCELED) return 86;
  alarm(0);
  close(sockets[0]);
  close(sockets[1]);

  pthread_t sleeper;
  if (pthread_create(&sleeper, NULL, blocked_nanosleep_worker, NULL) != 0) return 87;
  wait_started(&nanosleep_started);
  if (wait_for_blocked_syscall_either(&nanosleep_started, 35, 230, &delay) != 0)
    return 88;
  alarm(5);
  if (pthread_cancel(sleeper) != 0) return 88;
  result = NULL;
  if (pthread_join(sleeper, &result) != 0 || result != PTHREAD_CANCELED) return 89;
  alarm(0);

  pthread_t clock_sleeper;
  if (pthread_create(&clock_sleeper, NULL, blocked_clock_nanosleep_worker, NULL) != 0) return 90;
  wait_started(&clock_nanosleep_started);
  if (wait_for_blocked_syscall(&clock_nanosleep_started, 230, &delay) != 0) return 91;
  alarm(5);
  if (pthread_cancel(clock_sleeper) != 0) return 91;
  result = NULL;
  if (pthread_join(clock_sleeper, &result) != 0 || result != PTHREAD_CANCELED) return 92;
  alarm(0);

  pthread_t pauser;
  if (pthread_create(&pauser, NULL, blocked_pauser, NULL) != 0) return 191;
  wait_started(&pause_started);
  if (wait_for_blocked_syscall(&pause_started, 34, &delay) != 0) return 192;
  alarm(5);
  if (pthread_cancel(pauser) != 0) return 193;
  result = NULL;
  if (pthread_join(pauser, &result) != 0 || result != PTHREAD_CANCELED) return 194;
  alarm(0);

  epoll_fd = epoll_create1(EPOLL_CLOEXEC);
  if (epoll_fd < 0) return 93;
  pthread_t epoller;
  if (pthread_create(&epoller, NULL, blocked_epoll_wait_worker, NULL) != 0) return 94;
  wait_started(&epoll_wait_started);
  if (wait_for_blocked_syscall(&epoll_wait_started, 232, &delay) != 0) return 95;
  alarm(5);
  if (pthread_cancel(epoller) != 0) return 95;
  result = NULL;
  if (pthread_join(epoller, &result) != 0 || result != PTHREAD_CANCELED) return 96;
  alarm(0);

  pthread_t epoll_pwaiter;
  if (pthread_create(&epoll_pwaiter, NULL, blocked_epoll_pwait_worker, NULL) != 0) return 97;
  wait_started(&epoll_pwait_started);
  if (wait_for_blocked_syscall(&epoll_pwait_started, 281, &delay) != 0) return 98;
  alarm(5);
  if (pthread_cancel(epoll_pwaiter) != 0) return 98;
  result = NULL;
  if (pthread_join(epoll_pwaiter, &result) != 0 || result != PTHREAD_CANCELED) return 99;
  alarm(0);

  pthread_t epoll_pwait2er;
  if (pthread_create(&epoll_pwait2er, NULL, blocked_epoll_pwait2_worker, NULL) != 0) return 100;
  wait_started(&epoll_pwait2_started);
  if (wait_for_blocked_syscall(&epoll_pwait2_started, 441, &delay) != 0) return 101;
  alarm(5);
  if (pthread_cancel(epoll_pwait2er) != 0) return 101;
  result = NULL;
  if (pthread_join(epoll_pwait2er, &result) != 0 || result != PTHREAD_CANCELED) return 102;
  alarm(0);
  close(epoll_fd);

  if (snprintf(fifo_path, sizeof fifo_path, "/tmp/nelibc-cancel-lock-%ld", (long)getpid()) >=
      (int)sizeof fifo_path)
    return 103;
  int lock_fd = open(fifo_path, O_CREAT | O_EXCL | O_RDWR, 0600);
  if (lock_fd < 0) return 104;
  close(lock_fd);
  int ready_pipe[2];
  if (pipe(ready_pipe) != 0) return 105;
  pid_t lock_child = fork();
  if (lock_child < 0) return 106;
  if (lock_child == 0) {
    close(ready_pipe[0]);
    int child_fd = open(fifo_path, O_RDWR, 0);
    struct flock child_lock = {0};
    child_lock.l_type = F_WRLCK;
    child_lock.l_whence = SEEK_SET;
    char ready = child_fd >= 0 && fcntl(child_fd, F_SETLK, &child_lock) == 0 ? 'r' : 'e';
    (void)write(ready_pipe[1], &ready, 1);
    if (ready == 'e') _exit(1);
    for (;;) pause();
  }
  close(ready_pipe[1]);
  char ready;
  if (read(ready_pipe[0], &ready, 1) != 1 || ready != 'r') return 107;
  close(ready_pipe[0]);
  lock_wait_fd = open(fifo_path, O_RDWR, 0);
  if (lock_wait_fd < 0) return 108;
  pthread_t lock_waiter;
  if (pthread_create(&lock_waiter, NULL, blocked_record_locker, NULL) != 0) return 109;
  wait_started(&lock_wait_started);
  if (wait_for_blocked_syscall(&lock_wait_started, 72, &delay) != 0) return 110;
  alarm(5);
  if (pthread_cancel(lock_waiter) != 0) return 110;
  result = NULL;
  if (pthread_join(lock_waiter, &result) != 0 || result != PTHREAD_CANCELED) return 111;
  alarm(0);
  close(lock_wait_fd);
  if (kill(lock_child, SIGKILL) != 0 || waitpid(lock_child, NULL, 0) != lock_child) return 112;
  if (unlink(fifo_path) != 0) return 113;

  sigset_t previous_signal_mask;
  if (sigemptyset(&signal_wait_set) != 0 ||
      sigaddset(&signal_wait_set, SIGUSR1) != 0 ||
      pthread_sigmask(SIG_BLOCK, &signal_wait_set, &previous_signal_mask) != 0)
    return 121;
  struct timespec no_wait = {0, 0};
  siginfo_t timeout_info = {0};
  errno = 0;
  if (sigtimedwait(&signal_wait_set, &timeout_info, &no_wait) != -1 || errno != EAGAIN)
    return 138;
  siginfo_t received_info = {0};
  if (kill(getpid(), SIGUSR1) != 0 ||
      sigwaitinfo(&signal_wait_set, &received_info) != SIGUSR1 ||
      received_info.si_signo != SIGUSR1)
    return 139;
  sigwait_started_flag = 0;
  pthread_t sigwait_thread;
  if (pthread_create(&sigwait_thread, NULL, blocked_sigwait_worker, NULL) != 0) return 122;
  wait_started(&sigwait_started_flag);
  alarm(5);
  if (kill(getpid(), SIGUSR1) != 0) return 123;
  result = NULL;
  if (pthread_join(sigwait_thread, &result) != 0 ||
      result != (void *)(long)SIGUSR1)
    return 124;
  alarm(0);

  sigwait_started_flag = 0;
  if (pthread_create(&sigwait_thread, NULL, blocked_sigwait_worker, NULL) != 0) return 125;
  wait_started(&sigwait_started_flag);
  if (wait_for_blocked_syscall(&sigwait_started_flag, 128, &delay) != 0) return 126;
  alarm(5);
  if (pthread_cancel(sigwait_thread) != 0) return 126;
  result = NULL;
  if (pthread_join(sigwait_thread, &result) != 0 || result != PTHREAD_CANCELED) return 127;
  alarm(0);

  pthread_t sigwaitinfo_thread;
  if (pthread_create(&sigwaitinfo_thread, NULL, blocked_sigwaitinfo_worker, NULL) != 0) return 128;
  wait_started(&sigwaitinfo_started_flag);
  if (wait_for_blocked_syscall(&sigwaitinfo_started_flag, 128, &delay) != 0) return 129;
  alarm(5);
  if (pthread_cancel(sigwaitinfo_thread) != 0) return 129;
  result = NULL;
  if (pthread_join(sigwaitinfo_thread, &result) != 0 || result != PTHREAD_CANCELED) return 130;
  alarm(0);

  pthread_t sigtimedwait_thread;
  if (pthread_create(&sigtimedwait_thread, NULL, blocked_sigtimedwait_worker, NULL) != 0) return 131;
  wait_started(&sigtimedwait_started_flag);
  if (wait_for_blocked_syscall(&sigtimedwait_started_flag, 128, &delay) != 0) return 132;
  alarm(5);
  if (pthread_cancel(sigtimedwait_thread) != 0) return 132;
  result = NULL;
  if (pthread_join(sigtimedwait_thread, &result) != 0 || result != PTHREAD_CANCELED) return 133;
  alarm(0);

  pthread_t sigsuspend_thread;
  if (pthread_create(&sigsuspend_thread, NULL, blocked_sigsuspend_worker, NULL) != 0) return 134;
  wait_started(&sigsuspend_started_flag);
  if (wait_for_blocked_syscall(&sigsuspend_started_flag, 130, &delay) != 0) return 135;
  alarm(5);
  if (pthread_cancel(sigsuspend_thread) != 0) return 135;
  result = NULL;
  if (pthread_join(sigsuspend_thread, &result) != 0 || result != PTHREAD_CANCELED) return 136;
  alarm(0);
  if (pthread_sigmask(SIG_SETMASK, &previous_signal_mask, NULL) != 0) return 137;

  pthread_t readlink_thread;
  if (pthread_create(&readlink_thread, NULL, self_cancel_readlink_worker, NULL) != 0) return 140;
  result = NULL;
  if (pthread_join(readlink_thread, &result) != 0 || result != PTHREAD_CANCELED) return 141;

  pthread_t msync_thread;
  if (pthread_create(&msync_thread, NULL, self_cancel_msync_worker, NULL) != 0) return 142;
  result = NULL;
  if (pthread_join(msync_thread, &result) != 0 || result != PTHREAD_CANCELED) return 143;

  errno = 0;
  if (tcdrain(-1) != -1 || errno != EBADF) return 144;
  pthread_t tcdrain_thread;
  if (pthread_create(&tcdrain_thread, NULL, self_cancel_tcdrain_worker, NULL) != 0) return 145;
  result = NULL;
  if (pthread_join(tcdrain_thread, &result) != 0 || result != PTHREAD_CANCELED) return 146;

  char mq_name[64];
  if (snprintf(mq_name, sizeof(mq_name), "/nelibc-cancel-%ld", (long)getpid()) >=
      (int)sizeof(mq_name)) return 147;
  struct mq_attr mq_attributes = {0};
  mq_attributes.mq_maxmsg = 1;
  mq_attributes.mq_msgsize = 128;
  mq_descriptor = mq_open(mq_name, O_CREAT | O_EXCL | O_RDWR, 0600, &mq_attributes);
  if (mq_descriptor == (mqd_t)-1) {
    fprintf(stderr, "mq_open test setup failed: errno=%d\n", errno);
    return 148;
  }
  char mq_buffer[128] = {0};
  unsigned int mq_priority = 0;
  struct mq_attr observed_attributes = {0};
  if (mq_getattr(mq_descriptor, &observed_attributes) != 0 ||
      observed_attributes.mq_maxmsg != 1 || observed_attributes.mq_msgsize != 128 ||
      observed_attributes.mq_curmsgs != 0) return 169;
  struct mq_attr requested_attributes = {0};
  requested_attributes.mq_flags = O_NONBLOCK;
  if (mq_setattr(mq_descriptor, &requested_attributes, &observed_attributes) != 0 ||
      observed_attributes.mq_flags != 0) return 170;
  if (mq_getattr(mq_descriptor, &observed_attributes) != 0 ||
      observed_attributes.mq_flags != O_NONBLOCK) return 171;
  requested_attributes.mq_flags = 0;
  if (mq_setattr(mq_descriptor, &requested_attributes, NULL) != 0) return 172;
  struct sigevent mq_notification = {0};
  mq_notification.sigev_notify = SIGEV_NONE;
  if (mq_notify(mq_descriptor, &mq_notification) != 0) return 173;
  errno = 0;
  if (mq_notify(mq_descriptor, &mq_notification) != -1 || errno != EBUSY) return 174;
  if (mq_notify(mq_descriptor, NULL) != 0) return 175;
  sigset_t mq_signal_set, old_signal_set;
  sigemptyset(&mq_signal_set);
  sigaddset(&mq_signal_set, SIGUSR1);
  if (pthread_sigmask(SIG_BLOCK, &mq_signal_set, &old_signal_set) != 0) return 176;
  mq_notification.sigev_notify = SIGEV_SIGNAL;
  mq_notification.sigev_signo = SIGUSR1;
  if (mq_notify(mq_descriptor, &mq_notification) != 0) return 177;
  if (mq_send(mq_descriptor, "notice", 6, 0) != 0) return 178;
  struct timespec mq_signal_timeout = { .tv_sec = 2, .tv_nsec = 0 };
  siginfo_t mq_signal_info = {0};
  if (sigtimedwait(&mq_signal_set, &mq_signal_info, &mq_signal_timeout) != SIGUSR1)
    return 179;
  if (mq_receive(mq_descriptor, mq_buffer, sizeof(mq_buffer), &mq_priority) != 6 ||
      memcmp(mq_buffer, "notice", 6) != 0) return 180;
  if (pthread_sigmask(SIG_SETMASK, &old_signal_set, NULL) != 0) return 181;
  if (sem_init(&mq_thread_notification_sem, 0, 0) != 0) return 182;
  pthread_attr_t mq_thread_attributes;
  if (pthread_attr_init(&mq_thread_attributes) != 0 ||
      pthread_attr_setstacksize(&mq_thread_attributes, 1048576) != 0) return 183;
  mq_notification.sigev_notify = SIGEV_THREAD;
  mq_notification.sigev_notify_function = mq_thread_notification_callback;
  mq_notification.sigev_notify_attributes = &mq_thread_attributes;
  mq_notification.sigev_value.sival_ptr = &mq_thread_notification_sem;
  if (mq_notify(mq_descriptor, &mq_notification) != 0) {
    fprintf(stderr, "mq_notify SIGEV_THREAD failed: errno=%d\n", errno);
    return 184;
  }
  if (pthread_attr_destroy(&mq_thread_attributes) != 0) return 185;
  if (mq_send(mq_descriptor, "thread", 6, 0) != 0) return 186;
  if (clock_gettime(CLOCK_REALTIME, &mq_deadline) != 0) return 187;
  mq_deadline.tv_sec += 3;
  if (sem_timedwait(&mq_thread_notification_sem, &mq_deadline) != 0) return 188;
  if (mq_receive(mq_descriptor, mq_buffer, sizeof(mq_buffer), &mq_priority) != 6 ||
      memcmp(mq_buffer, "thread", 6) != 0) return 189;
  if (sem_destroy(&mq_thread_notification_sem) != 0) return 190;
  if (mq_send(mq_descriptor, "hello", 5, 7) != 0) return 149;
  if (mq_receive(mq_descriptor, mq_buffer, sizeof(mq_buffer), &mq_priority) != 5 ||
      memcmp(mq_buffer, "hello", 5) != 0 || mq_priority != 7) return 150;
  if (clock_gettime(CLOCK_REALTIME, &mq_deadline) != 0) return 151;
  mq_deadline.tv_sec -= 1;
  errno = 0;
  if (mq_timedreceive(mq_descriptor, mq_buffer, sizeof(mq_buffer), &mq_priority,
                      &mq_deadline) != -1 || errno != ETIMEDOUT) return 152;

  if (clock_gettime(CLOCK_REALTIME, &mq_deadline) != 0) return 153;
  mq_deadline.tv_sec += 3600;
  pthread_t mq_thread;
  if (pthread_create(&mq_thread, NULL, blocked_mq_receive_worker, NULL) != 0) return 154;
  wait_started(&mq_receive_started_flag);
  if (wait_for_blocked_syscall(&mq_receive_started_flag, 243, &delay) != 0) return 155;
  alarm(5);
  if (pthread_cancel(mq_thread) != 0) return 155;
  result = NULL;
  if (pthread_join(mq_thread, &result) != 0 || result != PTHREAD_CANCELED) return 156;
  alarm(0);

  if (pthread_create(&mq_thread, NULL, blocked_mq_timedreceive_worker, NULL) != 0) return 157;
  wait_started(&mq_timedreceive_started_flag);
  if (wait_for_blocked_syscall(&mq_timedreceive_started_flag, 243, &delay) != 0) return 158;
  alarm(5);
  if (pthread_cancel(mq_thread) != 0) return 158;
  result = NULL;
  if (pthread_join(mq_thread, &result) != 0 || result != PTHREAD_CANCELED) return 159;
  alarm(0);

  if (mq_send(mq_descriptor, "full", 4, 0) != 0) return 160;
  if (pthread_create(&mq_thread, NULL, blocked_mq_send_worker, NULL) != 0) return 161;
  wait_started(&mq_send_started_flag);
  if (wait_for_blocked_syscall(&mq_send_started_flag, 242, &delay) != 0) return 162;
  alarm(5);
  if (pthread_cancel(mq_thread) != 0) return 162;
  result = NULL;
  if (pthread_join(mq_thread, &result) != 0 || result != PTHREAD_CANCELED) return 163;
  alarm(0);

  if (pthread_create(&mq_thread, NULL, blocked_mq_timedsend_worker, NULL) != 0) return 164;
  wait_started(&mq_timedsend_started_flag);
  if (wait_for_blocked_syscall(&mq_timedsend_started_flag, 242, &delay) != 0) return 165;
  alarm(5);
  if (pthread_cancel(mq_thread) != 0) return 165;
  result = NULL;
  if (pthread_join(mq_thread, &result) != 0 || result != PTHREAD_CANCELED) return 166;
  alarm(0);
  if (mq_receive(mq_descriptor, mq_buffer, sizeof(mq_buffer), &mq_priority) != 4 ||
      memcmp(mq_buffer, "full", 4) != 0) return 167;
  if (mq_close(mq_descriptor) != 0 || mq_unlink(mq_name) != 0) return 168;

  pid_t exited_child = fork();
  if (exited_child < 0) return 114;
  if (exited_child == 0) _exit(42);
  siginfo_t exited_info = {0};
  if (waitid(P_PID, (id_t)exited_child, &exited_info, WEXITED) != 0 ||
      exited_info.si_pid != exited_child || exited_info.si_signo != SIGCHLD ||
      exited_info.si_code != CLD_EXITED || exited_info.si_status != 42)
    return 115;

  wait_child = fork();
  if (wait_child < 0) return 116;
  if (wait_child == 0) {
    for (;;) pause();
  }
  pthread_t waitid_thread;
  if (pthread_create(&waitid_thread, NULL, blocked_waitid_worker, NULL) != 0) return 117;
  wait_started(&waitid_started_flag);
  if (wait_for_blocked_syscall(&waitid_started_flag, 247, &delay) != 0) return 118;
  alarm(5);
  if (pthread_cancel(waitid_thread) != 0) return 118;
  result = NULL;
  if (pthread_join(waitid_thread, &result) != 0 || result != PTHREAD_CANCELED) return 119;
  alarm(0);
  if (kill(wait_child, SIGKILL) != 0 || waitpid(wait_child, NULL, 0) != wait_child)
    return 120;

  wait_child = fork();
  if (wait_child < 0) return 28;
  if (wait_child == 0) {
    for (;;) pause();
  }
  pthread_t waiter;
  if (pthread_create(&waiter, NULL, blocked_waiter, NULL) != 0) return 29;
  wait_started(&wait_started_flag);
  if (wait_for_blocked_syscall(&wait_started_flag, 61, &delay) != 0) return 30;
  alarm(5);
  if (pthread_cancel(waiter) != 0) return 30;
  result = NULL;
  if (pthread_join(waiter, &result) != 0 || result != PTHREAD_CANCELED) return 31;
  alarm(0);
  if (kill(wait_child, SIGKILL) != 0 || waitpid(wait_child, NULL, 0) != wait_child)
    return 32;

  if (snprintf(fifo_path, sizeof fifo_path, "/tmp/nelibc-cancel-open-%ld", (long)getpid()) >=
      (int)sizeof fifo_path)
    return 33;
  (void)unlink(fifo_path);
  if (mkfifo(fifo_path, 0600) != 0) return 34;
  pthread_t opener;
  if (pthread_create(&opener, NULL, blocked_opener, NULL) != 0) return 35;
  wait_started(&open_started);
  if (wait_for_blocked_syscall(&open_started, 257, &delay) != 0) return 36;
  alarm(5);
  if (pthread_cancel(opener) != 0) return 36;
  result = NULL;
  if (pthread_join(opener, &result) != 0 || result != PTHREAD_CANCELED) return 37;
  alarm(0);
  if (unlink(fifo_path) != 0) return 38;

  if (snprintf(fifo_path, sizeof fifo_path, "/tmp/nelibc-cancel-fsync-%ld", (long)getpid()) >=
      (int)sizeof fifo_path)
    return 39;
  sync_fd = open(fifo_path, O_CREAT | O_EXCL | O_RDWR, 0600);
  if (sync_fd < 0) return 40;
  char dirty = 'x';
  if (write(sync_fd, &dirty, 1) != 1) return 41;
  pthread_t sync_thread;
  if (pthread_create(&sync_thread, NULL, sync_worker, NULL) != 0) return 42;
  wait_started(&sync_started);
  nanosleep(&delay, NULL);
  alarm(5);
  if (pthread_cancel(sync_thread) != 0) return 43;
  result = NULL;
  if (pthread_join(sync_thread, &result) != 0 || result != PTHREAD_CANCELED) return 44;
  alarm(0);
  if (close(sync_fd) != 0 || unlink(fifo_path) != 0) return 45;
  return 0;
}
