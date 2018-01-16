#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <assert.h>
#include <errno.h>
#include "cao.h"
#include "queue.h"

/*
 * call-seq: ao.play(output_samples)
 *
 * 受け取ったサンプルを再生キューに追加する。
 * (デバイスがファイル出力の場合はファイルに書き出す)
 * 一度に渡せる量はunsigned int(32bit)の範囲まで。
 *
 * [arg1] buffer(String)
 * [return] Fixnum
*/
VALUE
raodev_play(VALUE obj, VALUE output_samples)
{
  sample_t *sample;
  ao_struct *aos;

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
  enqueue(aos, sample);
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
  int i;
  sample_t *sample;

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
 * [return] true or false
*/
VALUE
raodev_playing(VALUE obj)
{
  ao_struct *aos;
  Data_Get_Struct(rb_ivar_get(obj, rb_intern("@device")),
		  ao_struct, aos);
  if (aos->status == 2){
    return Qtrue;
  }
  return Qfalse;
}

/*
 * call-seq: ao.waiting
 *
 * 再生キューの待ち数を返す。
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
