#define _POSIX_C_SOURCE 200809L
#include <errno.h>
#include <fcntl.h>
#include <pthread.h>
#include <poll.h>
#include <signal.h>
#include <stdio.h>
#include <stddef.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <sys/un.h>
#include <sys/wait.h>
#include <time.h>
#include <unistd.h>

static int read_fd;
static int read_started;
static int write_fd;
static int write_started;
static int listen_fd;
static int accept_started;
static int recv_fd;
static int recv_started;
static int poll_fd;
static int poll_started;
static pid_t wait_child;
static int wait_started_flag;
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

static void *blocked_poller(void *unused) {
  (void)unused;
  struct pollfd descriptor = {poll_fd, POLLIN, 0};
  mark_started(&poll_started);
  return (void *)(long)poll(&descriptor, 1, -1);
}

static void *blocked_waiter(void *unused) {
  (void)unused;
  mark_started(&wait_started_flag);
  return (void *)(long)waitpid(wait_child, NULL, 0);
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
