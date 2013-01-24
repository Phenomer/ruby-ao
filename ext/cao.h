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

typedef struct dev_data {
  ao_device        *device;
  ao_sample_format *format;
  ao_option        *option;
  struct dev_data  *next;
} dev_data;

dev_data * append_device(ao_device *dev, ao_sample_format *format,
			 ao_option *option);
void close_device(dev_data *devdat);
void remove_device(dev_data *devdat);
#endif
