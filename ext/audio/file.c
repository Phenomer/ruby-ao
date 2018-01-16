#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include "cao.h"
#include "thread.h"

ao_device *
rao_open_file_dev(int driver_id, char *filename,
		  int overwrite, ao_sample_format *format,
		  ao_option *option){
  ao_device *dev;
  
  dev = ao_open_file(driver_id, filename,
		     overwrite, format, option);
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
  return dev;
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
VALUE
rao_open_file(VALUE obj,      VALUE driver_id,
	      VALUE filename, VALUE overwrite,
	      VALUE bits, VALUE rate, VALUE channels,
	      VALUE byte_format, VALUE matrix,
	      VALUE a_options)
{
  ao_struct        *aos;
  ao_device        *dev;
  ao_sample_format *format;
  ao_option        *option;
  int               overwrite_int = 0;
  VALUE             rdev;

  if (rb_iv_get(obj, "@device") != Qnil){
    Data_Get_Struct(rb_iv_get(obj, "@device"),
		    ao_struct, aos);
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
  dev = rao_open_file_dev(FIX2INT(driver_id), StringValuePtr(filename), overwrite_int, format, option);
  if ((aos = create_thread(dev, format, option)) == NULL){
    rb_raise(cAO_eUnknownError,
	     "memory allocation failure - %s",
	     strerror(errno));
  }
  rdev = Data_Wrap_Struct(cAO_DeviceData, 0,
			  remove_device, aos);
  rb_iv_set(obj, "@device", rdev);
  rb_ary_push(rb_cv_get(cAO_File, "@@devices"), rdev);
    
  return obj;
}
