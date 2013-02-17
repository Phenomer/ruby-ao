#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include<pthread.h>
#include<unistd.h>
#include "cao.h"

static VALUE cAO_cDevice;

static void *
play_thread(dev_data *devdata)
{
  sample_buffer *currentbuf;
  int result;

  while(1){
    while ((currentbuf = devdata->buffer) == NULL){
      if (devdata->playing < 0){
	return NULL;
      }
      usleep(100000);
    }
    result = ao_play(devdata->device, currentbuf->samples, currentbuf->bytes);
    if (result == 0){
      rb_raise(cAO_eDeviceError, "Device should be closed.");
    }
    devdata->buffer = devdata->buffer->next;
    free(currentbuf->samples);
    free(currentbuf);
    devdata->playing--;
  }
  return NULL;
}


/*
 * call-seq: dev = Audio::BasicDevice.new(rdevdata)
 *
 * デバイス操作クラスのインスタンスを生成する。
 *
 * [arg1] Audio::BasicDeviceData
*/
static VALUE
raodev_initialize(VALUE obj, VALUE rdevdata)
{
  dev_data *devdata;

  rb_ivar_set(obj, rb_intern("@device"), rdevdata);
  Data_Get_Struct(rdevdata, dev_data, devdata);
  pthread_create(&devdata->thread, NULL, (void *)play_thread, devdata);
  return Qnil;
}

/*
 * call-seq: dev.play(output_samples)
 *
 * 受け取ったサンプルを再生する。
 * (デバイスがファイル出力の場合はファイルに書き出す)
 * 一度に渡せる量はunsigned int(32bit)の範囲まで。
 *
 * [arg1] buffer(String)
 * [return] Fixnum
*/
static VALUE
raodev_play(VALUE obj, VALUE output_samples)
{
  int        result;
  uint32_t   bytes;
  dev_data  *devdata;

  Check_Type(output_samples, T_STRING);
  Data_Get_Struct(rb_ivar_get(obj, rb_intern("@device")),
		  dev_data, devdata);
  bytes = RSTRING_LENINT(output_samples);
  result = 
    ao_play(devdata->device,
	    StringValuePtr(output_samples), bytes);
  if (result == 0){
    rb_raise(cAO_eDeviceError, "Device should be closed.");
  }
  return INT2FIX(result);
}

/*
 * call-seq: dev.play_bg(output_samples)
 *
 * 受け取ったサンプルをバックグラウンドで再生する。
 * バックグラウンドで再生されているか否かを確認するには
 * dev.playing?を利用する。
 * (デバイスがファイル出力の場合はファイルに書き出す)
 * 一度に渡せる量はunsigned int(32bit)の範囲まで。
 *
 * [arg1] buffer(String)
 * [return] true or false
*/
static VALUE
raodev_play_bg(VALUE obj, VALUE samples)
{
  dev_data      *devdata;
  sample_buffer *new_buffer;
  sample_buffer *behind_buffer  = NULL;
  sample_buffer *current_buffer = NULL;

  if ((new_buffer = malloc(sizeof(sample_buffer))) == NULL){
    rb_raise(cAO_eUnknownError, "memory allocation failure.");
  }
  new_buffer->next = NULL;
  Check_Type(samples, T_STRING);
  Data_Get_Struct(rb_ivar_get(obj, rb_intern("@device")),
		  dev_data, devdata);
  new_buffer->bytes = RSTRING_LENINT(samples);
  if ((new_buffer->samples = malloc(new_buffer->bytes)) == NULL){
    rb_raise(cAO_eUnknownError, "memory allocation failure.");
  }
  memcpy(new_buffer->samples, StringValuePtr(samples), new_buffer->bytes);
  current_buffer = devdata->buffer;
  while (current_buffer != NULL){
    behind_buffer  = current_buffer;
    current_buffer = current_buffer->next;
  }
  if(behind_buffer == NULL){
    devdata->buffer = new_buffer;
  } else {
    behind_buffer->next = new_buffer;
  }
  devdata->playing++;
  return Qtrue;
}

/*
 * call-seq: dev.buffers
 *
 * バックグラウンド再生バッファの長さを返す。
 * 非再生中の場合はnilを返す。
 * [return] FixNum or nil
*/
static VALUE
raodev_buffers(VALUE obj)
{
  dev_data *devdata;

  Data_Get_Struct(rb_ivar_get(obj, rb_intern("@device")),
		  dev_data, devdata);
  if (devdata->playing < 0){
    return Qnil;
  }
  return INT2FIX(devdata->playing);
}

/*
 * call-seq: dev.playing?
 *
 * バックグラウンドで再生中であるか否かを確認する。
 *
 * [return] true or false
*/
static VALUE
raodev_playing(VALUE obj)
{
  dev_data *devdata;

  Data_Get_Struct(rb_ivar_get(obj, rb_intern("@device")),
		  dev_data, devdata);
  if (devdata->playing > 0){
    return Qtrue;
  } else {
    return Qfalse;
  }
}

/*
 * call-seq: dev.close
 *
 * デバイスを閉じる。
 *
 * [return] true
*/
static VALUE
raodev_close(VALUE obj)
{
  dev_data  *devdata;
  Data_Get_Struct(rb_ivar_get(obj, rb_intern("@device")),
		  dev_data, devdata);
  close_device(devdata);
  return Qtrue;
}

/*
 * call-seq: dev.closed?
 *
 * デバイスが既に閉じられているか確認する。
 * [return] true or false
*/
static VALUE
raodev_closed(VALUE obj)
{
  dev_data  *devdata;
  Data_Get_Struct(rb_ivar_get(obj, rb_intern("@device")),
		  dev_data, devdata);
  if (devdata->device == NULL){
    return Qtrue;
  }
  return Qfalse;
}

void init_cao_device(void){
/* 
 * 開いたデバイスを操作するクラス。
 */
  cAO_cDevice = rb_define_class_under(cAO, "BasicDevice", rb_cObject);
  rb_define_private_method(cAO_cDevice, "initialize", raodev_initialize, 1);
  rb_define_method(cAO_cDevice, "play",     raodev_play,    1);
  rb_define_method(cAO_cDevice, "play_bg",  raodev_play_bg, 1);
  rb_define_method(cAO_cDevice, "buffers",  raodev_buffers, 0);
  rb_define_method(cAO_cDevice, "playing?", raodev_playing, 0);
  rb_define_method(cAO_cDevice, "close",    raodev_close,   0);
  rb_define_method(cAO_cDevice, "closed?",  raodev_closed,  0);
}
