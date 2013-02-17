#ifndef CAO_H
#define CAO_H

#include <ruby.h>
#include <ao/ao.h>
#include <ao/os_types.h>

extern VALUE cAudio;
extern VALUE cAO;
extern VALUE cAO_cDeviceData;
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

typedef struct sample_buffer {
  int   bytes;
  char *samples;
  struct sample_buffer *next;
} sample_buffer;

typedef struct dev_data {
  ao_device        *device;
  ao_sample_format *format;
  ao_option        *option;
  sample_buffer    *buffer;
  pthread_t         thread;
  int               playing;
  struct dev_data  *next;
} dev_data;

dev_data * append_device(ao_device *dev,
			 ao_sample_format *format,
			 ao_option *option);
void close_device(dev_data *devdat);
void remove_device(dev_data *devdat);

void init_exception(void);
void init_constant(void);
void init_cao_device(void);
#endif
