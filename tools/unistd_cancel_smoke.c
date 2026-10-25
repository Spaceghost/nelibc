#define _GNU_SOURCE
#include <errno.h>
#include <fcntl.h>
#include <pthread.h>
#include <poll.h>
#include <signal.h>
#include <stdio.h>
#include <stddef.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/epoll.h>
#include <sys/stat.h>
#include <sys/un.h>
#include <sys/uio.h>
#include <sys/wait.h>
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
static char fifo_path[128];
static int open_started;
static int sync_fd;
static int sync_started;
static pthread_mutex_t start_mutex = PTHREAD_MUTEX_INITIALIZER;
static pthread_cond_t start_condition = PTHREAD_COND_INITIALIZER;

static void mark_started(int *started) {
  pthread_mutex_lock(&start_mutex);
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
  nanosleep(&delay, NULL);
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
  nanosleep(&delay, NULL);
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
  nanosleep(&delay, NULL);
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
  nanosleep(&delay, NULL);
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
  nanosleep(&delay, NULL);
  alarm(5);
  if (pthread_cancel(acceptor) != 0) return 15;
  result = NULL;
  if (pthread_join(acceptor, &result) != 0 || result != PTHREAD_CANCELED) return 16;
  alarm(0);
  accept_started = 0;
  if (pthread_create(&acceptor, NULL, blocked_accept4, NULL) != 0) return 17;
  wait_started(&accept_started);
  nanosleep(&delay, NULL);
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
  nanosleep(&delay, NULL);
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
  nanosleep(&delay, NULL);
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
  nanosleep(&delay, NULL);
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
  nanosleep(&delay, NULL);
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
  nanosleep(&delay, NULL);
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
  nanosleep(&delay, NULL);
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
  nanosleep(&delay, NULL);
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
  nanosleep(&delay, NULL);
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
  nanosleep(&delay, NULL);
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
  nanosleep(&delay, NULL);
  alarm(5);
  if (pthread_cancel(sleeper) != 0) return 88;
  result = NULL;
  if (pthread_join(sleeper, &result) != 0 || result != PTHREAD_CANCELED) return 89;
  alarm(0);

  pthread_t clock_sleeper;
  if (pthread_create(&clock_sleeper, NULL, blocked_clock_nanosleep_worker, NULL) != 0) return 90;
  wait_started(&clock_nanosleep_started);
  nanosleep(&delay, NULL);
  alarm(5);
  if (pthread_cancel(clock_sleeper) != 0) return 91;
  result = NULL;
  if (pthread_join(clock_sleeper, &result) != 0 || result != PTHREAD_CANCELED) return 92;
  alarm(0);

  epoll_fd = epoll_create1(EPOLL_CLOEXEC);
  if (epoll_fd < 0) return 93;
  pthread_t epoller;
  if (pthread_create(&epoller, NULL, blocked_epoll_wait_worker, NULL) != 0) return 94;
  wait_started(&epoll_wait_started);
  nanosleep(&delay, NULL);
  alarm(5);
  if (pthread_cancel(epoller) != 0) return 95;
  result = NULL;
  if (pthread_join(epoller, &result) != 0 || result != PTHREAD_CANCELED) return 96;
  alarm(0);

  pthread_t epoll_pwaiter;
  if (pthread_create(&epoll_pwaiter, NULL, blocked_epoll_pwait_worker, NULL) != 0) return 97;
  wait_started(&epoll_pwait_started);
  nanosleep(&delay, NULL);
  alarm(5);
  if (pthread_cancel(epoll_pwaiter) != 0) return 98;
  result = NULL;
  if (pthread_join(epoll_pwaiter, &result) != 0 || result != PTHREAD_CANCELED) return 99;
  alarm(0);

  pthread_t epoll_pwait2er;
  if (pthread_create(&epoll_pwait2er, NULL, blocked_epoll_pwait2_worker, NULL) != 0) return 100;
  wait_started(&epoll_pwait2_started);
  nanosleep(&delay, NULL);
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
  nanosleep(&delay, NULL);
  alarm(5);
  if (pthread_cancel(lock_waiter) != 0) return 110;
  result = NULL;
  if (pthread_join(lock_waiter, &result) != 0 || result != PTHREAD_CANCELED) return 111;
  alarm(0);
  close(lock_wait_fd);
  if (kill(lock_child, SIGKILL) != 0 || waitpid(lock_child, NULL, 0) != lock_child) return 112;
  if (unlink(fifo_path) != 0) return 113;

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
  nanosleep(&delay, NULL);
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
  nanosleep(&delay, NULL);
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
  nanosleep(&delay, NULL);
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
