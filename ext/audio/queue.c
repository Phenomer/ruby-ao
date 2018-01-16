#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <assert.h>
#include "cao.h"

int
enqueue(ao_struct *aos, void *value){
  queue *last;

  assert(pthread_mutex_lock(&aos->mutex) == 0);
  if (aos->queue == NULL){
    aos->queue = malloc(sizeof(queue));
    aos->queue->data = value;
    aos->queue->next = NULL;
    aos->qsize++;
    assert(pthread_cond_signal(&aos->cond) == 0);
    assert(pthread_mutex_unlock(&aos->mutex) == 0);
    return aos->qsize;
  }

  last = aos->queue;
  while (last->next != NULL){
    last = last->next;
  }
  last->next = malloc(sizeof(queue));
  last->next->data = value;
  last->next->next = NULL;
  aos->qsize++;
  assert(pthread_cond_signal(&aos->cond) == 0);
  assert(pthread_mutex_unlock(&aos->mutex) == 0);
  return aos->qsize;
}

void *
dequeue(ao_struct *aos){
  void *res;
  queue *trash;

  if (aos->queue == NULL){
    return NULL;
  }
  assert(pthread_mutex_lock(&aos->mutex) == 0);
  trash = aos->queue;
  res = aos->queue->data;
  aos->queue = aos->queue->next;
  aos->qsize--;
  free(trash);
  assert(pthread_mutex_unlock(&aos->mutex) == 0);
  return res;
}
