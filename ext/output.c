#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include "cao.h"

VALUE cAudio;
VALUE cAO_Live;
VALUE cAO_File;
VALUE cAO_DeviceData;
VALUE cAO_eAOError;
VALUE cAO_eDeviceError;
VALUE cAO_eUnknownError;

VALUE cAO_eNoDriver;
VALUE cAO_eNotFile;
VALUE cAO_eNotLive;
VALUE cAO_eBadOption;
VALUE cAO_eDriverError;

VALUE cAO_eFileError;
VALUE cAO_eFileExists;
VALUE cAO_eBadFormat;

/*
 * call-seq: Audio::LiveOutput.new(driver_id, bits, rate, channels, byte_format, matrix, options)
 *
 * オーディオ出力デバイスを開く。
 * 引数に指定するmatrixとoptionについては以下を参照。
 * [matrix] http://xiph.org/ao/doc/ao_sample_format.html
 * [option] http://xiph.org/ao/doc/drivers.html
 * optionはKey-Valueを要素に含む2次元配列を設定する。
 * 特に設定が必要なければnilで構わない。
 * ex) ALSAドライバのデバイスをhw:1に設定する場合は [["dev", "hw:1"]] を渡す。
 *
 * [arg1] DriverID
 * [arg2] bits(Fixnum)
 * [arg3] rate(Fixnum)
 * [arg4] channels(fixnum)
 * [arg5] byte_format(fixnum)
 * [arg6] matrix(String or nil)
 * [arg7] options(Array or nil)
 * [return] self
 */
static VALUE
rao_open_live(VALUE obj,      VALUE driver_id,
	      VALUE bits, VALUE rate, VALUE channels,
	      VALUE byte_format, VALUE matrix, 
	      VALUE a_options)
{
  ao_device        *dev;
  ao_sample_format *format;
  ao_option        *option;
  dev_data         *devdat;
  VALUE             rdev;

  if (rb_iv_get(obj, "@device") != Qnil){
    Data_Get_Struct(rb_iv_get(obj, "@device"),
		    dev_data, devdat);
    rb_raise(cAO_eDeviceError,
	     "Device is already open.\n");
  }
  
  Check_Type(driver_id,   T_FIXNUM);
  format = set_format(bits, rate, channels, byte_format, matrix);
  option = set_option(a_options);
  dev    = ao_open_live(FIX2INT(driver_id), format, option);
  if (dev == NULL){
    free_format(format);
    ao_free_options(option);
    switch(errno){
    case AO_ENODRIVER:
      rb_raise(cAO_eNoDriver,
	       "No driver corresponds - %s",
	       strerror(errno));
      break;
    case AO_ENOTLIVE:
      rb_raise(cAO_eNotLive,
	       "This driver is not a live output device - %s",
	       strerror(errno));
      break;
    case AO_EBADOPTION:
      rb_raise(cAO_eBadOption,
	       "A valid option key has an invalid value - %s",
	       strerror(errno));
      break;
    case AO_EOPENDEVICE:
      rb_raise(cAO_eDeviceError,
	       "Cannot open the device - %s",
	       strerror(errno));
      break;
    case AO_EFAIL:
      rb_raise(cAO_eUnknownError,
	       "Any other cause of failure - %s",
	       strerror(errno));
      break;
    default:
      rb_raise(cAO_eUnknownError,
	       "Unknown error - %s",
	       strerror(errno));
    }
  }
  if ((devdat = append_device(dev, format, option)) == NULL){
    rb_raise(cAO_eUnknownError,
	     "memory allocation failure - %s",
	     strerror(errno));
  }

  rdev = Data_Wrap_Struct(cAO_DeviceData, 0,
			  remove_device, devdat);
  rb_iv_set(obj, "@device", rdev);  
  rb_ary_push(rb_cv_get(cAO_Live, "@@devices"), rdev);
  
  return obj;
}

/*
 * call-seq: Audio::FileOutput.new(driver_id, filepath, overwrite, bits, rate, channels, byte_format, matrix, options)
 *
 * ファイルを開く。引数に指定するmatrixとoptionについては以下を参照。
 * [matrix] http://xiph.org/ao/doc/ao_sample_format.html
 * [option] http://xiph.org/ao/doc/drivers.html
 * optionはKey-Valueを要素に含む2次元配列を設定する。
 * 特に設定が必要なければnilで構わない。
 * ex) RAWドライバの出力エンディアンをビッグエンディアンに設定する場合は [["byteirder", "big"]] を渡す。
 *
 * [arg1] DriverID(Fixnum)
 * [arg2] filepath(String)
 * [arg3] overwrite?(true or false)
 * [arg4] bits(Fixnum)
 * [arg5] rate(Fixnum)
 * [arg6] channels(fixnum)
 * [arg7] byte_format(fixnum)
 * [arg8] matrix(String or nil)
 * [arg9] options(Array or nil)
 * [return] self
 */
static VALUE
rao_open_file(VALUE obj,      VALUE driver_id,
	      VALUE filename, VALUE overwrite,
	      VALUE bits, VALUE rate, VALUE channels,
	      VALUE byte_format, VALUE matrix,
	      VALUE a_options)
{
  dev_data         *devdat;
  ao_device        *dev;
  ao_sample_format *format;
  ao_option        *option;
  int               overwrite_int = 0;
  VALUE             rdev;
  
  
  if (rb_iv_get(obj, "@device") != Qnil){
    Data_Get_Struct(rb_iv_get(obj, "@device"),
		    dev_data, devdat);
    rb_raise(cAO_eDeviceError,
	     "Device is already open.\n");
  }
  
  Check_Type(filename,  T_STRING);
  Check_Type(driver_id, T_FIXNUM);
  if (TYPE(overwrite) == T_TRUE){
    overwrite_int = 1;
  }
  format = set_format(bits, rate, channels, byte_format, matrix);
  option = set_option(a_options);
  dev = ao_open_file(FIX2INT(driver_id), StringValuePtr(filename), 
		     overwrite_int, format, option);
  if (dev == NULL){
    free_format(format);
    ao_free_options(option);
    switch(errno){
    case AO_ENODRIVER:
      rb_raise(cAO_eNoDriver,
	       "No driver corresponds - %s",
	       strerror(errno));
      break;
    case AO_ENOTFILE:
      rb_raise(cAO_eNotFile,
	       "This driver is not a file output device - %s",
	       strerror(errno));
      break;
    case AO_EBADOPTION:
      rb_raise(cAO_eBadOption,
	       "A valid option key has an invalid value - %s",
	       strerror(errno));
      break;
    case AO_EOPENFILE:
      rb_raise(cAO_eFileError,
	       "Cannot open the device - %s",
	       strerror(errno));
      break;
    case AO_EFILEEXISTS:
      rb_raise(cAO_eFileExists,
	       "The file already exists - %s",
	       strerror(errno));
    case AO_EFAIL:
      rb_raise(cAO_eUnknownError,
	       "Any other cause of failure - %s",
	       strerror(errno));
      break;
    default:
      rb_raise(cAO_eUnknownError,
	       "Unknown error - %s",
	       strerror(errno));
    }
  }  
  if ((devdat = append_device(dev, format, option)) == NULL){
    rb_raise(cAO_eUnknownError,
	     "memory allocation failure - %s",
	     strerror(errno));
  }

  rdev = Data_Wrap_Struct(cAO_DeviceData, 0,
			  remove_device, devdat);
  rb_iv_set(obj, "@device", rdev);
  rb_ary_push(rb_cv_get(cAO_File, "@@devices"), rdev);
    
  return obj;
}

static VALUE
rao_close(VALUE obj)
{
  dev_data *devdata;

  if (rb_iv_get(obj, "@device") == Qnil){
    return Qfalse;
  }
  
  Data_Get_Struct(rb_iv_get(obj, "@device"),
		  dev_data, devdata);
  close_device(devdata);
  rb_ary_delete(rb_cv_get(cAO_Live, "@@devices"), rb_intern("@device"));
  rb_iv_set(obj, "@device", Qnil);
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
  dev_data *devdata;
  
  rb_gc_start();
  while ((rdev = rb_ary_pop(rb_cv_get(cAO_Live, "@@devices"))) != Qnil){
    Data_Get_Struct(rdev, dev_data, devdata);
    close_device(devdata);
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

  /*
   * Document-class: Audio::FileOutput
   *
   * オーディオファイル出力機能をサポートするクラス。
   */
  cAO_File = rb_define_class_under(cAudio, "FileOutputC", cAO_Live);
  rb_define_private_method(cAO_File, "initialize", rao_open_file, 9);

  init_exception();
  init_info();
  init_constant();
  init_option();

  /* Shutdown */
  rb_set_end_proc(rao_shutdown, (VALUE)NULL);
}
