#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <assert.h>
#include <pthread.h>
#include <time.h>
#include <signal.h>
#include "cao.h"
#include "queue.h"
#include "thread.h"

void *
thread_player(void *val){
  queue res;
  ao_struct *aos = val;
  sample_t *sample;
  struct timespec tout = {0, 1000000};

  pthread_setcancelstate(PTHREAD_CANCEL_DISABLE, NULL);
  /*assert(pthread_detach(pthread_self()) == 0);*/

  while (1){
    assert(pthread_mutex_lock(&aos->mutex) == 0);
    while (aos->status == 1 && aos->queue == NULL){
      pthread_cond_timedwait(&aos->cond, &aos->mutex, &tout);
    }
    if (aos->status != 1){
      assert(pthread_mutex_unlock(&aos->mutex) == 0);
      break;
    }
    aos->status = 2;
    assert(pthread_mutex_unlock(&aos->mutex) == 0);

    while ((sample = dequeue(aos)) != NULL){
      ao_play(aos->device, sample->buffer,
	      sample->bytes);
      free(sample->buffer);
      free(sample);
      assert(pthread_mutex_lock(&aos->mutex) == 0);
      if (aos->status == 1){
	assert(pthread_mutex_unlock(&aos->mutex) == 0);
	break;
      }
      assert(pthread_mutex_unlock(&aos->mutex) == 0);
    }

    assert(pthread_mutex_lock(&aos->mutex) == 0);
    if (aos->status == 2){
      aos->status = 1;
      assert(pthread_mutex_unlock(&aos->mutex) == 0);
      break;
    }
    assert(pthread_mutex_unlock(&aos->mutex) == 0);
  }

  while ((sample = dequeue(aos)) != NULL){
    free(sample->buffer);
    free(sample);
  }
  if (aos->device != NULL){
    ao_close(aos->device);
  }
  if (aos->option != NULL){
    ao_free_options(aos->option);
  }
  if (aos->format != NULL){
    free_format(aos->format);
  }
  aos->device = NULL;
  aos->option = NULL;
  aos->format = NULL;
  aos->queue  = NULL;
  assert(pthread_mutex_lock(&aos->mutex) == 0);
  aos->status = -1;
  assert(pthread_mutex_unlock(&aos->mutex) == 0);
  pthread_exit(NULL);
}

ao_struct *
create_thread(ao_device *dev, ao_sample_format *format,
	      ao_option *option){
  ao_struct *aos;

  aos          = ALLOC(ao_struct);
  aos->device  = dev;
  aos->format  = format;
  aos->option  = option;
  aos->queue   = NULL;
  aos->status  = 1;
  aos->qsize   = 0;
  assert(pthread_mutex_init(&aos->mutex, NULL) == 0);
  assert(pthread_cond_init(&aos->cond, NULL) == 0);
  assert(pthread_create(&aos->thread, NULL,
			thread_player, aos) == 0);
  return aos;
}
