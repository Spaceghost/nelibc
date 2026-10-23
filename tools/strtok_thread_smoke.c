#define _GNU_SOURCE
#include "string.h"
#include <pthread.h>
#include <stdio.h>
#include <string.h>

struct worker_state {
  pthread_barrier_t *barrier;
  char input[16];
  char first[8];
  char second[8];
};

static void *run_worker(void *argument) {
  struct worker_state *state = argument;
  char *token = strtok(state->input, ",");
  if (token != NULL) strcpy(state->first, token);
  (void)pthread_barrier_wait(state->barrier);
  token = strtok(NULL, ",");
  if (token != NULL) strcpy(state->second, token);
  return NULL;
}

int main(void) {
  pthread_barrier_t barrier;
  pthread_t threads[2];
  struct worker_state states[2] = {
      {.barrier = &barrier, .input = "a1,a2"},
      {.barrier = &barrier, .input = "b1,b2"},
  };
  if (pthread_barrier_init(&barrier, NULL, 2) != 0) return 1;
  if (pthread_create(&threads[0], NULL, run_worker, &states[0]) != 0 ||
      pthread_create(&threads[1], NULL, run_worker, &states[1]) != 0)
    return 2;
  if (pthread_join(threads[0], NULL) != 0 || pthread_join(threads[1], NULL) != 0)
    return 3;
  (void)pthread_barrier_destroy(&barrier);
  if (strcmp(states[0].first, "a1") != 0 || strcmp(states[0].second, "a2") != 0 ||
      strcmp(states[1].first, "b1") != 0 || strcmp(states[1].second, "b2") != 0)
    return 4;
  puts("Nelua strtok state is independent between threads");
  return 0;
}
