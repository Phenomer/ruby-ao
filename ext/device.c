#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include "cao.h"

static VALUE cAO_cDevice;

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
  rb_ivar_set(obj, rb_intern("@device"), rdevdata);
  return Qnil;
}

/*
 * call-seq: dev.play(output_samples)
 *
 * 受け取ったサンプルを再生する。
 * (デバイスがファイル出力の場合はファイルに書き出す)
 * 一度に渡せる量はunsigned int(32bit)の範囲まで。
 *
 * [arg1] BasicOutput::BasicDeviceData
 * [arg2] buffer(String)
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
 * call-seq: dev.close
 *
 * デバイスを閉じる。
 *
 * [arg1] BasicOutput::BasicDeviceData
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
 * [arg1] BasicOutput::BasicDeviceData
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
  rb_define_method(cAO_cDevice, "play",    raodev_play,   1);
  rb_define_method(cAO_cDevice, "close",   raodev_close,  0);
  rb_define_method(cAO_cDevice, "closed?", raodev_closed, 0);
}
