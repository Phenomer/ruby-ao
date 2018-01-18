#ifndef CAO_H
#define CAO_H

#include <ruby.h>
#include <ruby/thread.h>
#include <ao/ao.h>
#include <ao/os_types.h>
#include <pthread.h>

extern VALUE cAudio;
extern VALUE cAO_Live;
extern VALUE cAO_File;
extern VALUE cAO_Info;
extern VALUE cAO_Mixer;
extern VALUE cAO_DeviceData;
extern VALUE cAO_eAOError;
extern VALUE cAO_eDeviceError;
extern VALUE cAO_eUnknownError;

extern VALUE cAO_eNoDriver;
extern VALUE cAO_eNotFile;
extern VALUE cAO_eNotLive;
extern VALUE cAO_eBadOption;
extern VALUE cAO_eDriverError;

extern VALUE cAO_eFileError;
extern VALUE cAO_eFileExists;
extern VALUE cAO_eBadFormat;

#define cAO_STATE_EXIT -1
#define cAO_STATE_STOP 0
#define cAO_STATE_ACTIVE 1
#define cAO_STATE_PLAYING 2

typedef struct queue {
  void *data;
  struct queue *next;
} queue;

typedef struct sample_t {
  uint_32 bytes;
  char *buffer;
} sample_t;

typedef struct ao_struct {
  ao_device        *device;
  ao_sample_format *format;
  ao_option        *option;
  queue            *queue;
  int               qsize;
  pthread_t         thread;
  pthread_mutex_t   mutex;
  pthread_cond_t    cond;
  int               status;
  int               tmode;
} ao_struct;


void init_info(void);
void init_option(void);
void init_mixer(void);
ao_struct *init_aos(ao_device *dev,
		    ao_sample_format *format,
		    ao_option *option);
void close_device(ao_struct *aos);
void remove_device(ao_struct *aos);

void init_exception(void);
void init_constant(void);
ao_sample_format *
set_format(VALUE bits, VALUE rate, VALUE channels,
	   VALUE byte_format, VALUE matrix);
void free_format(ao_sample_format *format);
ao_option * set_option(VALUE a_options);

VALUE
raodev_play(VALUE obj, VALUE output_samples);

VALUE raodev_close(VALUE obj);
VALUE raodev_closed(VALUE obj);
VALUE raodev_playing(VALUE obj);
VALUE raodev_waiting(VALUE obj);

VALUE
rao_open_live(VALUE obj,      VALUE driver_id,
	      VALUE bits, VALUE rate, VALUE channels,
	      VALUE byte_format, VALUE matrix,
	      VALUE a_options, VALUE thread);
VALUE
rao_open_file(VALUE obj,      VALUE driver_id,
	      VALUE filename, VALUE overwrite,
	      VALUE bits, VALUE rate, VALUE channels,
	      VALUE byte_format, VALUE matrix,
	      VALUE a_options, VALUE thread);
#endif
