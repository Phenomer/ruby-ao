#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <assert.h>
#include <errno.h>
#include "cao.h"
#include "queue.h"

typedef struct aos_gvl {
  ao_struct *aos;
  sample_t   sample;
  int        res;
} aos_gvl;

void *nogvl_ao_play(void *dat){
  aos_gvl *aosg = (aos_gvl *)dat;

  aosg->res =
    ao_play(aosg->aos->device,
	    aosg->sample.buffer,
	    aosg->sample.bytes);
  return NULL;
}

ao_struct *
init_aos(ao_device *dev, ao_sample_format *format,
	 ao_option *option){
  ao_struct *aos;

  aos          = ALLOC(ao_struct);
  aos->device  = dev;
  aos->format  = format;
  aos->option  = option;
  aos->queue   = NULL;
  aos->status  = 1;
  aos->qsize   = 0;
  aos->tmode   = 0;
  return aos;
}

/*
 * call-seq: ao.play(output_samples)
 *
 * 受け取ったサンプルを再生する。
 * threadが有効の場合、受け取ったサンプルを
 * 再生キューに追加する。
 * (デバイスがファイル出力の場合はファイルに書き出す)
 * 一度に渡せる量はunsigned int(32bit)の範囲まで。
 *
 * [arg1] buffer(String)
 * [return] Fixnum
 */
VALUE
raodev_play(VALUE obj, VALUE output_samples)
{
  sample_t  *sample;
  ao_struct *aos;
  aos_gvl    aosg;

  Check_Type(output_samples, T_STRING);
  if (rb_ivar_get(obj, rb_intern("@device")) == Qnil){
    rb_raise(cAO_eAOError, "Device already closed.");
  }
  Data_Get_Struct(rb_ivar_get(obj, rb_intern("@device")),
		  ao_struct, aos);
  if ((sample = malloc(sizeof(sample_t))) == NULL){
    rb_raise(cAO_eAOError, "Memory allocation failure.");
  }
  sample->bytes = RSTRING_LENINT(output_samples);
  if ((sample->buffer = malloc(sample->bytes)) == NULL){
    rb_raise(cAO_eAOError, "Memory allocation failure.");
  }
  memcpy(sample->buffer, StringValuePtr(output_samples), sample->bytes);

  if (aos->tmode == 1){
    enqueue(aos, sample);
  } else {
    aosg.aos = aos;
    aosg.sample = *sample;
#ifdef HAVE_RB_THREAD_CALL_WITHOUT_GVL
    rb_thread_call_without_gvl2(nogvl_ao_play, &aosg, NULL, NULL);
#else
    nogvl_ao_play(&aosg);
#endif
    free(sample->buffer);
  }
  return obj;
}

/*
 * ao_struct構造体の持つデバイスを全て閉じ、
 * またオプションとフォーマットの情報も削除する。
 * メンバ変数にはNULLを設定する。
 * ao_struct構造体自体は開放しない。
 * (rubyのGCによって実行されるremove_device関数に任せるため)
 */
void
close_device(ao_struct *aos){
  sample_t *sample;

  if (aos->tmode == 1){
    assert(pthread_mutex_lock(&aos->mutex) == 0);
    if (aos->status > 0){
      aos->status = 0;
    }
    assert(pthread_cond_signal(&aos->cond) == 0);
    assert(pthread_mutex_unlock(&aos->mutex) == 0);

    while ((sample = dequeue(aos)) != NULL){
      free(sample->buffer);
      free(sample);
    }
    pthread_join(aos->thread, NULL);
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
  return;
}

/*
 * GCが実行された時にデバイスを閉じ、構造体を開放する。
 * この関数はopen_live()とopen_file()にてao_struct構造体をWrapする時に
 * 構造体free用関数として渡す(rubyのGCにて呼ばれるよう設定する)
 * Data_Wrap_Struct(cAO_DeviceData, 0, remove_device, aos);
 */
void
remove_device(ao_struct *aos)
{
  close_device(aos);
  free(aos);
  return;
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
  ao_struct *aos;
  Data_Get_Struct(rb_ivar_get(obj, rb_intern("@device")),
		  ao_struct, aos);
  if (aos->device == NULL){
    return Qtrue;
  }
  return Qfalse;
}

/*
 * call-seq: ao.playing?
 *
 * デバイスがオーディオを出力中か確認する。
 * (threadを有効にしていない場合、常にfalse)
 * [return] true or false
 */
VALUE
raodev_playing(VALUE obj)
{
  ao_struct *aos;
  Data_Get_Struct(rb_ivar_get(obj, rb_intern("@device")),
		  ao_struct, aos);
  if (aos->thread == 1 && aos->status == 2){
    return Qtrue;
  }
  return Qfalse;
}

/*
 * call-seq: ao.waiting
 *
 * 再生キューの待ち数を返す。
 * (threadを有効にしていない場合、常に0)
 * [return] Integer
 */
VALUE
raodev_waiting(VALUE obj)
{
  ao_struct *aos;
  Data_Get_Struct(rb_ivar_get(obj, rb_intern("@device")),
		  ao_struct, aos);
  return INT2FIX(aos->qsize);
}
