#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include "cao.h"

VALUE cAudio;
VALUE cAO_Live;
VALUE cAO_File;
VALUE cAO_Info;
VALUE cAO_Mixer;
VALUE cAO_DeviceData;
VALUE cAO_eAOError;
VALUE cAO_eDeviceError;
VALUE cAO_eUnknownError;

VALUE cAO_eNoDriver;
VALUE cAO_eNotFile;
VALUE cAO_eNotLive;
VALUE cAO_eBadOption;
VALUE cAO_eDriverError;

VALUE
rao_close(VALUE obj)
{
  ao_struct *aos;

  if (rb_iv_get(obj, "@device") == Qnil){
    return Qfalse;
  }

  Data_Get_Struct(rb_iv_get(obj, "@device"),
		  ao_struct, aos);
  close_device(aos);
  rb_ary_delete(rb_cv_get(cAO_Live, "@@devices"), rb_intern("@device"));
  /*rb_iv_set(obj, "@device", Qnil);*/
  return Qtrue;
}

/*
 * libaoを終了する。shutdownの前に開いている全てのデバイスを
 * closeしておかなければならない。
 *
 * [return] nil
 */
void
rao_shutdown(VALUE obj){
  VALUE rdev;
  ao_struct *aos;

  rb_gc_start();
  while ((rdev = rb_ary_pop(rb_cv_get(cAO_Live, "@@devices"))) != Qnil){
    Data_Get_Struct(rdev, ao_struct, aos);
    close_device(aos);
  }
  ao_shutdown();
  return;
}

void
Init_outputc(void)
{
  ao_initialize();
  /*
   * Document-class: Audio
   *
   * Ruby-AOの基礎となるクラス。
   */
  cAudio = rb_define_class("Audio", rb_cObject);

  /*
   * Document-class: Audio::LiveOutput
   *
   * オーディオデバイス出力機能をサポートするクラス。
   */
  cAO_Live = rb_define_class_under(cAudio, "LiveOutputC", rb_cObject);
  rb_cv_set(cAO_Live, "@@devices", rb_ary_new());

  /*
   * Document-class: Audio::BasicOutput::DeviceData
   *
   * 開いたデバイスに関する基本的な情報を保持するクラス。
   * ruby側から操作はしない。
   */
  cAO_DeviceData = rb_define_class_under(cAO_Live, "DeviceData", rb_cData);

  /* library initialize & shutdown */
  rb_define_private_method(cAO_Live, "initialize", rao_open_live, 7);

  /* device setup */
  rb_define_method(cAO_Live, "close",   rao_close,     0);
  rb_define_method(cAO_Live, "play",    raodev_play,   1);
  rb_define_method(cAO_Live, "closed?", raodev_closed, 0);
  rb_define_method(cAO_Live, "playing?", raodev_playing, 0);
  rb_define_method(cAO_Live, "waiting", raodev_waiting, 0);

  /*
   * Document-class: Audio::FileOutput
   *
   * オーディオファイル出力機能をサポートするクラス。
   */
  cAO_File = rb_define_class_under(cAudio, "FileOutputC", cAO_Live);
  rb_define_private_method(cAO_File, "initialize", rao_open_file, 9);

  init_exception();
  init_info();
  init_mixer();
  init_constant();
  init_option();

  /* Shutdown */
  rb_set_end_proc(rao_shutdown, (VALUE)NULL);
}
