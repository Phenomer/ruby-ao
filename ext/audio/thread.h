#ifndef THREAD_H
#define THREAD_H
void *thread_player(void *device);
ao_struct *create_thread(ao_device *dev, ao_sample_format *format, ao_option *option);
#endif
