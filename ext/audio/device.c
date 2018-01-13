#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include "cao.h"

typedef struct play_data {
  ao_device *device;
  char *samples;
  uint_32 bytes;
  int result;
} play_data;

void *nogvl_ao_play(void *playdata){
  play_data *pd = playdata;
  pd->result =
    ao_play(pd->device, pd->samples, pd->bytes);
  return NULL;
}

/*
 * デバイス一覧にデバイスを追加する。
 * 追加した構造体へのポインタを返す。
 * 割当に失敗した場合は-1を返す。
 */
dev_data *
append_device(ao_device *dev, ao_sample_format *format,
	      ao_option *option)
{
  dev_data *newdev;
  newdev          = ALLOC(dev_data);
  newdev->device  = dev;
  newdev->format  = format;
  newdev->option  = option;
  newdev->buffer  = NULL;
  return newdev;
}

/*
 * devdata構造体の持つデバイスを全て閉じ、
 * またオプションとフォーマットの情報も削除する。
 * メンバ変数にはNULLを設定する。
 * devdata構造体自体は開放しない。
 * (rubyのGCによって実行されるremove_device関数に任せるため)
 */
void
close_device(dev_data *devdat){
  if (devdat->device != NULL){
    ao_close(devdat->device);
  }
  if (devdat->option != NULL){
    ao_free_options(devdat->option);
  }
  if (devdat->format != NULL){
    free_format(devdat->format);
  }
  devdat->device = NULL;
  devdat->option = NULL;
  devdat->format = NULL;
  devdat->buffer = NULL;
  return;
}

/* 
 * GCが実行された時にデバイスを閉じ、構造体を開放する。
 * この関数はopen_live()とopen_file()にてdev_data構造体をWrapする時に
 * 構造体free用関数として渡す(rubyのGCにて呼ばれるよう設定する)
 * Data_Wrap_Struct(cAO_DeviceData, 0, remove_device, devdat);
 */
void
remove_device(dev_data *devdat)
{
  close_device(devdat);
  free(devdat);
  return;
}

/*
 * call-seq: ao.play(output_samples)
 *
 * 受け取ったサンプルを再生する。
 * (デバイスがファイル出力の場合はファイルに書き出す)
 * 一度に渡せる量はunsigned int(32bit)の範囲まで。
 *
 * [arg1] buffer(String)
 * [return] Fixnum
*/
VALUE
raodev_play(VALUE obj, VALUE output_samples)
{
  dev_data  *devdata;
  play_data  playdata;

  Check_Type(output_samples, T_STRING);
  Data_Get_Struct(rb_ivar_get(obj, rb_intern("@device")),
		  dev_data, devdata);
  playdata.device  = devdata->device;
  playdata.bytes   = RSTRING_LENINT(output_samples);
  if ((playdata.samples = malloc(playdata.bytes)) == NULL){
    rb_raise(cAO_eAOError, "Memory allocation failure.");
  }
  memcpy(playdata.samples, StringValuePtr(output_samples), playdata.bytes);
#ifdef HAVE_RB_THREAD_CALL_WITHOUT_GVL
  rb_thread_call_without_gvl2(nogvl_ao_play, &playdata, NULL, NULL);
  /*rb_thread_call_without_gvl2((rb_blocking_function_t *)nogvl_ao_play, &playdata, NULL, NULL);*/
#else
  nogvl_ao_play(&playdata);
#endif
  free(playdata.samples);
  if (playdata.result == 0){
    rb_raise(cAO_eDeviceError, "Device should be closed.");
  }
  return INT2FIX(playdata.result);
}

/*
 * call-seq: ao.closed?
 *
 * デバイスが既に閉じられているか確認する。
 * [return] true or false
*/
VALUE
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
