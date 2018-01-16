#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include "cao.h"
#include "thread.h"

ao_device *
rao_open_live_dev(int driver_id,
		  ao_sample_format *format,
		  ao_option *option){
  ao_device *dev;

  dev = ao_open_live(driver_id, format, option);
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
  return dev;
}

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
VALUE
rao_open_live(VALUE obj,      VALUE driver_id,
	      VALUE bits, VALUE rate, VALUE channels,
	      VALUE byte_format, VALUE matrix,
	      VALUE a_options)
{
  ao_struct        *aos;
  ao_device        *dev;
  ao_sample_format *format;
  ao_option        *option;
  VALUE             rdev;

  if (rb_iv_get(obj, "@device") != Qnil){
    Data_Get_Struct(rb_iv_get(obj, "@device"),
		    ao_struct, aos);
    rb_raise(cAO_eDeviceError,
	     "Device is already open.\n");
  }
  Check_Type(driver_id,   T_FIXNUM);
  format = set_format(bits, rate, channels, byte_format, matrix);
  option = set_option(a_options);

  dev = rao_open_live_dev(FIX2INT(driver_id), format, option);
  if ((aos = create_thread(dev, format, option)) == NULL){
    rb_raise(cAO_eUnknownError,
	     "memory allocation failure - %s",
	     strerror(errno));
  }
  rdev = Data_Wrap_Struct(cAO_DeviceData, 0,
			  remove_device, aos);
  rb_iv_set(obj, "@device", rdev);  
  rb_ary_push(rb_cv_get(cAO_Live, "@@devices"), rdev);
  return obj;
}
