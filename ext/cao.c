#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include<errno.h>
#include<ruby.h>
#include<ao/ao.h>
#include<ao/os_types.h>
#include<ao/plugin.h>

static VALUE cAO;
static VALUE cAO_eAOError;
static VALUE cAO_eNoDriver;
static VALUE cAO_eNotFile;
static VALUE cAO_eNotLive;
static VALUE cAO_eBadOption;
static VALUE cAO_eDriverError;
static VALUE cAO_eDeviceError;
static VALUE cAO_eFileError;
static VALUE cAO_eFileExists;
static VALUE cAO_eBadFormat;
static VALUE cAO_eUnknownError;

typedef struct dev_list {
  int               id;
  ao_device        *device;
  ao_sample_format *format;
  ao_option        *option;
  struct dev_list  *next;
} dev_list;

static dev_list *devices = NULL;

/*
  デバイス一覧にデバイスを追加する。
  追加する際、一意なIDを付与し、付与したID(>=0)を返す。
  割当に失敗した場合は-1を返す。
*/
int
append_device(ao_device *dev, ao_sample_format *format,
	      ao_option *option)
{
  int       id      = 0;
  dev_list *behind  = NULL;
  dev_list *current = devices;

  while (current != NULL){
    id      = current->id + 1;
    if (id < 0){id = 0;};
    behind  = current;
    current = current->next;
  }
  if ((current = malloc(sizeof(dev_list))) == NULL){
    return -1;
  }
  current->id     = id;
  current->device = dev;
  current->format = format;
  current->option = option;
  current->next   = NULL;
  if (behind != NULL){
    behind->next    = current;
  } else {
    devices = current;
  }

  return id;
}

/*
  デバイス一覧からデバイスIDを元にデバイスを検索し返す。
  デバイスが見つからない場合NULLを返す。
*/
ao_device *
search_device(int id)
{
  dev_list  *tmp    = devices;
  ao_device *result = NULL;
  while (tmp != NULL){
    if (tmp->id == id){
      result = tmp->device;
      break;
    }
    tmp = devices->next;
  }
  return result;
}

/* 
   デバイス一覧からデバイスIDを元にデバイスを削除する。
   成功した場合デバイスID(>=0)を、失敗した場合-1を返す。
*/
int
remove_device(int id)
{
  dev_list *behind  = NULL;
  dev_list *current = devices;
  
  while (current != NULL){
    if (current->id == id){
      if (current->next != NULL && behind != NULL){
	behind->next = current->next;
      }
      free(current->format);
      ao_free_options(current->option);
      free(current);
      return id;
    }
  }
  return -1;
}

/*
  全てのデバイスを閉じリストから削除する。
*/
/* VALUE */
/* rao_close_all_device() */
/* { */
/*   dev_list *current = devices; */
/*   dev_list *behind  = NULL; */
/*   int result; */

/*   while (current != NULL){ */
/*     behind  = current; */
/*     current = current->next; */
/*     result = ao_close(behind->device); */
/*     if (result == 0){ */
/*       fprintf(stderr, */
/* 	      "Indicates an error while the device was being closed."); */
/*     } */
/*     free(behind->format); */
/*     ao_free_options(behind->option); */
/*     free(behind); */
/*   } */
  
/*   return Qtrue; */
/* } */


/*
  引数に設定されたサンプルフォーマットをao_sample_format構造体に設定する。
  成功したらao_sample_format構造体へのポインタを返す。
*/
static ao_sample_format *
set_format(VALUE bits, VALUE rate, VALUE channels,
	   VALUE byte_format, VALUE matrix)
{
  ao_sample_format *format;
  if ((format = malloc(sizeof(ao_sample_format))) == NULL){
    rb_raise(cAO_eUnknownError, "memory allocation failure.");
  }
  Check_Type(bits,        T_FIXNUM);
  Check_Type(rate,        T_FIXNUM);
  Check_Type(channels,    T_FIXNUM);
  Check_Type(byte_format, T_FIXNUM);
  format->bits        = FIX2INT(bits);
  format->rate        = FIX2INT(rate);
  format->channels    = FIX2INT(channels);
  format->byte_format = FIX2INT(byte_format);
  switch(TYPE(matrix)){
  case T_STRING:
    format->matrix = StringValuePtr(matrix);
    break;
  case T_NIL:
    format->matrix = NULL;
    break;
  default:
    Check_Type(matrix, T_STRING);
    break;
  }
  return format;
}

/*
  Rubyの配列からデバイスオプションを読み込む。
  配列ではなくnilであった場合はNULLを返す。
*/
static ao_option *
set_option(VALUE a_options)
{
  ao_option *option = NULL;
  int   pos, result;
  VALUE element, key, value;
  switch(TYPE(a_options)){
  case T_ARRAY:
    for (pos=0; pos<RARRAY_LEN(a_options); pos++){
      element  = rb_ary_entry(a_options, pos);
      Check_Type(element, T_ARRAY);
      key      = rb_ary_entry(element, 0);
      value    = rb_ary_entry(element, 1);
      Check_Type(key,   T_STRING);
      Check_Type(value, T_STRING);
      printf("%s: %s\n", StringValuePtr(key),
	     StringValuePtr(value));

      result = ao_append_option(&option,
				StringValuePtr(key),
				StringValuePtr(value));
      if (result == 0){
	rb_raise(cAO_eUnknownError, "memory allocation failure.");
      }
    }
    break;
  case T_NIL:
    break;
  default:
    rb_raise(cAO_eBadOption, "Unsupported option type.");
    break;
  }
  return option;
}

/*
  ao_info構造体をRubyの配列に変換し返す。
*/
static VALUE
ao_info2array(ao_info *info)
{
  int opt_count;
  VALUE iary, optary;

  iary   = rb_ary_new();
  optary = rb_ary_new();
  rb_ary_push(iary, INT2FIX(info->type));
  rb_ary_push(iary, rb_str_new2(info->name));
  rb_ary_push(iary, rb_str_new2(info->short_name));
  rb_ary_push(iary, rb_str_new2(info->author));
  rb_ary_push(iary, rb_str_new2(info->comment));
  rb_ary_push(iary, INT2FIX(info->preferred_byte_format));
  rb_ary_push(iary, INT2FIX(info->priority));
  rb_ary_push(iary, INT2FIX(info->option_count));
  for(opt_count=0; opt_count<info->option_count; opt_count++){
    rb_ary_push(optary, rb_str_new2(info->options[opt_count]));
  }
  rb_ary_push(iary, optary);
  return iary;
}

/* #define AO_TYPE_LIVE 1 */
/* #define AO_TYPE_FILE 2 */

/* #define AO_ENODRIVER   1 */
/* #define AO_ENOTFILE    2 */
/* #define AO_ENOTLIVE    3 */
/* #define AO_EBADOPTION  4 */
/* #define AO_EOPENDEVICE 5 */
/* #define AO_EOPENFILE   6 */
/* #define AO_EFILEEXISTS 7 */
/* #define AO_EBADFORMAT  8 */

/* #define AO_EFAIL       100 */

/* #define AO_FMT_LITTLE 1 */
/* #define AO_FMT_BIG    2 */
/* #define AO_FMT_NATIVE 4 */

/* /\* --- Structures --- *\/ */

/* typedef struct ao_info { */
/*   int  type; /\* live output or file output? *\/ */
/*   char *name; /\* full name of driver *\/ */
/*   char *short_name; /\* short name of driver *\/ */
/*   char *author; /\* driver author *\/ */
/*   char *comment; /\* driver comment *\/ */
/*   int  preferred_byte_format; */
/*   int  priority; */
/*   char **options; */
/*   int  option_count; */
/* } ao_info; */

/* typedef struct ao_functions ao_functions; */
/* typedef struct ao_device ao_device; */

/* typedef struct ao_sample_format { */
/*   int  bits; /\* bits per sample *\/ */
/*   int  rate; /\* samples per second (in a single channel) *\/ */
/*   int  channels; /\* number of audio channels *\/ */
/*   int  byte_format; /\* Byte ordering in sample, see constants below *\/ */
/*   char *matrix; /\* input channel location/ordering *\/ */
/* } ao_sample_format; */

/* typedef struct ao_option { */
/*   char *key; */
/*   char *value; */
/*   struct ao_option *next; */
/* } ao_option; */

/* library setup/teardown */
/* void ao_initialize(void); */
/* void ao_shutdown(void); */

/*
  call-seq: AO.new

  libaoを初期化する。これを実行してからshutdownを実行するまでの間のみ
  CAOクラスのメソッドを利用できる。
 */
static VALUE
rao_initialize(VALUE obj)
{
  ao_initialize();
  return Qnil;
}

/*
  call-seq: ao.shutdown

  libaoを終了する。shutdownの前に開いている全てのデバイスを
  closeしておかなければならない。
  
  [return] nil
 */
static VALUE
rao_shutdown(VALUE obj)
{
  ao_shutdown();
  return Qnil;
}

/* device setup/playback/teardown */
/* int   ao_append_global_option(const char *key, */
/*                               const char *value); */
/* int          ao_append_option(ao_option **options, */
/*                               const char *key, */
/*                               const char *value); */
/* void          ao_free_options(ao_option *options); */
/* ao_device*       ao_open_live(int driver_id, */
/*                               ao_sample_format *format, */
/*                               ao_option *option); */
/* ao_device*       ao_open_file(int driver_id, */
/*                               const char *filename, */
/*                               int overwrite, */
/*                               ao_sample_format *format, */
/*                               ao_option *option); */

/* int                   ao_play(ao_device *device, */
/*                               char *output_samples, */
/*                               uint_32 num_bytes); */
/* int                  ao_close(ao_device *device); */

/*
  call-seq: ao.append_global_option(key, value)

  libao全体で参照されるオプションを設定する。
  オプションはKey-Value形式で設定する。
  各ドライバ毎のオプションについては下記を参照。
  http://xiph.org/ao/doc/drivers.html

  [param1] key(String)
  [param2] value(String)
  [return] true
 */
static VALUE
rao_append_global_option(VALUE obj,
			 VALUE key, VALUE value)
{
  int result;
  Check_Type(key, T_STRING);
  Check_Type(value, T_STRING);
  result = ao_append_global_option(StringValuePtr(key),
				   StringValuePtr(value));
  if (result == 0){
    rb_raise(cAO_eUnknownError, "memory allocation failure.");
  }
  return Qtrue;
}

/*
  call-seq: ao.open_live(driver_id, bits, rate, channels, byte_format, matrix, options)

  LiveDeviceを開く。引数に指定するmatrixとoptionについては以下を参照。
  [matrix] http://xiph.org/ao/doc/ao_sample_format.html
  [option] http://xiph.org/ao/doc/drivers.html
  optionはKey-Valueを要素に含む多次元配列を設定する。
  特に設定が必要なければnilで構わない。
  ex) ALSAドライバのデバイスをhw:1に設定する場合は [["dev", "hw:1"]] を渡す。

  [param1] DriverID
  [param2] bits(Fixnum)
  [param3] rate(Fixnum)
  [param4] channels(fixnum)
  [param5] byte_format(fixnum)
  [param6] matrix(String or nil)
  [param7] option(Array or nil)
*/
static VALUE
rao_open_live(VALUE obj,      VALUE driver_id,
	      VALUE bits, VALUE rate, VALUE channels,
	      VALUE byte_format, VALUE matrix, 
	      VALUE a_options)
{
  int               id;
  ao_device        *dev;
  ao_sample_format *format;
  ao_option        *option;

  Check_Type(driver_id,   T_FIXNUM);
  format = set_format(bits, rate, channels, byte_format, matrix);
  option = set_option(a_options);
  dev    = ao_open_live(FIX2INT(driver_id), format, option);
  if (dev == NULL){
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
  if (id = append_device(dev, format, option) < 0){
    rb_raise(cAO_eUnknownError,
	     "Unknown error - %s",
	     strerror(errno));
  }
  return INT2FIX(id);
}

/*
  call-seq: ao.open_file(driver_id, filepath, overwrite, bits, rate, channels, byte_format, matrix, options)

  Fileを開く。引数に指定するmatrixとoptionについては以下を参照。
  [matrix] http://xiph.org/ao/doc/ao_sample_format.html
  [option] http://xiph.org/ao/doc/drivers.html
  optionはKey-Valueを要素に含む多次元配列を設定する。
  特に設定が必要なければnilで構わない。
  ex) RAWドライバの出力エンディアンをビッグエンディアンに設定する場合は [["byteirder", "big"]] を渡す。

  [param1] DriverID
  [param2] filepath(String)
  [param3] overwrite?(true or false)
  [param4] bits(Fixnum)
  [param5] rate(Fixnum)
  [param6] channels(fixnum)
  [param7] byte_format(fixnum)
  [param8] matrix(String or nil)
  [param9] option(Array or nil)
*/
static VALUE
rao_open_file(VALUE obj,      VALUE driver_id,
	      VALUE filename, VALUE overwrite,
	      VALUE bits, VALUE rate, VALUE channels,
	      VALUE byte_format, VALUE matrix,
	      VALUE a_options)
{
  unsigned int      id;
  ao_device        *dev;
  ao_sample_format *format;
  ao_option        *option;
  int               overwrite_int = 0;

  Check_Type(filename,    T_STRING);
  Check_Type(driver_id,   T_FIXNUM);
  if (TYPE(overwrite) == T_TRUE){
    overwrite_int = 1;
  }
  format = set_format(bits, rate, channels, byte_format, matrix);
  option = set_option(a_options);
  dev = ao_open_file(FIX2INT(driver_id), StringValuePtr(filename), 
		     overwrite_int, format, option);
  if (dev == NULL){
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
  if (id = append_device(dev, format, option) < 0){
    rb_raise(cAO_eUnknownError,
	     "Unknown error - %s",
	     strerror(errno));
  }
  return INT2FIX(id);
}

/*
  call-seq: ao.play(device_id, output_samples)

  受け取ったサンプルを再生する。
  (デバイスがファイル出力の場合はファイルに書き出す)
  一度に渡せる量はunsigned int(32bit)の範囲まで。

  [param1] DeviceID
  [param2] buffer(String)
  [return] Fixnum
*/
static VALUE
rao_play(VALUE obj,            VALUE device_id,
	 VALUE output_samples)
{
  ao_device *dev;
  int        result;
  uint32_t   bytes;

  Check_Type(output_samples, T_STRING);
  Check_Type(device_id, T_FIXNUM);
  bytes = RSTRING_LENINT(output_samples);
  if ((dev = search_device(FIX2INT(device_id))) == NULL){
    rb_raise(cAO_eDeviceError, "Cannot find device from device ID.");
  }
  result = ao_play(dev, StringValuePtr(output_samples), bytes);
  if (result == 0){
    rb_raise(cAO_eDeviceError, "Device should be closed.");
  }
  return INT2FIX(result);
}

/*
  call-seq: ao.close(device_id)

  デバイスを閉じる。
  [param1] DeviceID
  [return] true
*/
static VALUE
rao_close(VALUE obj, VALUE device_id)
{
  ao_device *dev;
  int result;
  int devid = FIX2INT(device_id);
  Check_Type(device_id, T_FIXNUM);
  if ((dev = search_device(devid)) == NULL){
    rb_raise(cAO_eDeviceError, "Cannot find device from device ID.");
  }
  result = ao_close(dev);
  remove_device(devid);
  if (result == 0){
    rb_raise(cAO_eDeviceError,
	     "Indicates an error while the device was being closed.");
  }
  return Qtrue;
}

/*
  call-seq: ao.closed?(device_id)

  デバイスが閉じられているか否かを調査する。
  閉じられている場合true, 閉じられていない場合falseを返す。
  
  [param1] DeviceID
  [return] true or false
*/
static VALUE
rao_closed(VALUE obj, VALUE device_id)
{
  ao_device *dev;
  int result;
  Check_Type(device_id, T_FIXNUM);
  if ((dev = search_device(FIX2INT(device_id))) == NULL){
    return Qtrue;
  }
  return Qfalse;
}

/* driver information */
/* int              ao_driver_id(const char *short_name); */
/* int      ao_default_driver_id(void); */
/* ao_info       *ao_driver_info(int driver_id); */
/* ao_info **ao_driver_info_list(int *driver_count); */
/* char       *ao_file_extension(int driver_id); */

/*
  call-seq: ao.driver_id(short_name)

  short_nameを元にDriverIDを検索する。
  見つからなかった場合はnilを返す。

  [param1] short_name(String)
  [return] DriverID or nil
*/
static VALUE
rao_driver_id(VALUE obj, VALUE short_name)
{
  int driver_id;
  Check_Type(short_name, T_STRING);
  driver_id = ao_driver_id(StringValuePtr(short_name));
  if (driver_id < 0){
    return Qnil;
    /* rb_raise(cAO_eNoDriver, */
    /* 	     "No driver by that name exists."); */
  }
  return INT2FIX(driver_id);
}

/*
  call-seq: ao.default_driver_id

  デフォルトのDriverIDを返す。
  [return] DriverID
*/
static VALUE
rao_default_driver_id(VALUE obj)
{
  int id;
  id = ao_default_driver_id();
  if (id < 0){
    rb_raise(cAO_eNoDriver,
	     "Failure to find a usable audio output device.");
  }
  return INT2FIX(id);
}

/*
  call-seq: ao.driver_info(driver_id)

  ドライバの情報を確認する。
  ドライバ情報の要素順は以下の通り。
  [name(String), short_name(String), author(String),
  comment(String), preferred_byte_format(Fixnum),
  priority(Fixnum), option_count(Fixnum), options(Array)]

  [param1] DriverID
  [return] Driver Information(Array)
*/
static VALUE
rao_driver_info(VALUE obj, VALUE driver_id)
{
  ao_info *info;

  Check_Type(driver_id, T_FIXNUM);
  info = ao_driver_info(FIX2INT(driver_id));
  if (info == NULL){
    rb_raise(cAO_eNoDriver,
	     "Does not correspond to an actual driver.");
  }
  return ao_info2array(info);;
}

/*
  call-seq: ao.driver_info_list

  システムのlibaoがサポートしているドライバ一覧を配列で返す。
  [return] ドライバ一覧の配列
*/
static VALUE
rao_driver_info_list(VALUE obj)
{
  ao_info **info;
  int info_count, driver_count;
  VALUE iary;

  iary = rb_ary_new();
  info = ao_driver_info_list(&driver_count);
  for (info_count=0; info_count<driver_count;
       info_count++){
    rb_ary_push(iary, ao_info2array(info[info_count]));
  }
  return iary;
}

/*
  call-seq: ao.file_extension(driver_id)

  ファイル出力時の拡張子の標準を確認する。
  [param1] DriverID
  [return] ext name(String)
*/
static VALUE
rao_file_extension(VALUE obj, VALUE driver_id)
{
  char *ext;

  Check_Type(driver_id, T_FIXNUM);
  ext = ao_file_extension(FIX2INT(driver_id));
  if (ext == NULL){
    rb_raise(cAO_eDriverError,
	     "This driver has no file extension associated with it or if this driver does not exist.");
  }
  return rb_str_new2(ext);
}

/* miscellaneous */
/* int          ao_is_big_endian(void); */

/*
  call-seq: ao.bigendian?

  ホストの環境がビッグエンディアンであるか否かを調査する。
  [return] ビッグエンディアンであればtrue、リトルエンディアンであればfalse
*/
static VALUE
rao_is_big_endian(VALUE obj)
{
  if (ao_is_big_endian() == 1){
    return Qtrue;
  } else {
    return Qfalse;
  }
}


/*
  Ruby-AOの基礎となるクラス。通常はこれを直接利用するのではなく、
  利用しやすい形にしたAOクラスを用いる。
*/

void
Init_cao(void)
{
  cAO = rb_define_class("CAO", rb_cObject);

  /* exceptions */
  cAO_eAOError =
    rb_define_class_under(cAO, "AOError", rb_eStandardError);
  cAO_eNoDriver =
    rb_define_class_under(cAO, "NoDriver",     cAO_eAOError);
  cAO_eNotFile =
    rb_define_class_under(cAO, "NotFile",      cAO_eAOError);
  cAO_eNotLive =
    rb_define_class_under(cAO, "NotLive",      cAO_eAOError);
  cAO_eBadOption =
    rb_define_class_under(cAO, "BadOption",    cAO_eAOError);
  cAO_eDriverError =
    rb_define_class_under(cAO, "DriverError",  cAO_eAOError);
  cAO_eDeviceError =
    rb_define_class_under(cAO, "DeviceError",  cAO_eAOError);
  cAO_eFileError =
    rb_define_class_under(cAO, "FileError",    cAO_eAOError);
  cAO_eFileExists =
    rb_define_class_under(cAO, "FileExists",   cAO_eAOError);
  cAO_eBadFormat =
    rb_define_class_under(cAO, "BadFormat",    cAO_eAOError);
  cAO_eUnknownError =
    rb_define_class_under(cAO, "UnknownError", cAO_eAOError);

  /* constants */
  /* ドライバがLive出力用であることを示す。 */
  rb_define_const(cAO, "TYPE_LIVE",  INT2FIX(AO_TYPE_LIVE));
  /* ドライバがファイル出力用であることを示す。 */
  rb_define_const(cAO, "TYPE_FILE",  INT2FIX(AO_TYPE_FILE));
  /* データのエンディアンがリトルエンディアンであることを示す。 */
  rb_define_const(cAO, "FMT_LITTLE", INT2FIX(AO_FMT_LITTLE));
  /* データのエンディアンがビッグエンディアンであることを示す。 */
  rb_define_const(cAO, "FMT_BIG",    INT2FIX(AO_FMT_BIG));
  /* データのエンディアンがホストのネイティブ形式であることを示す。 */
  rb_define_const(cAO, "FMT_NATIVE", INT2FIX(AO_FMT_NATIVE));

  /* library setup/shutdown */
  rb_define_private_method(cAO, "initialize", rao_initialize, 0);
  rb_define_method(cAO, "shutdown",           rao_shutdown, 0);

  /* device setup/playback/teardown */
  rb_define_method(cAO, "append_global_option", rao_append_global_option, 2);
  rb_define_method(cAO, "open_live",            rao_open_live, 7);
  rb_define_method(cAO, "open_file",            rao_open_file, 9);
  rb_define_method(cAO, "play",                 rao_play, 2);
  rb_define_method(cAO, "close",                rao_close, 1);
  /*rb_define_method(cAO, "close_all_device"      rao_close_all_device, 1);*/
  rb_define_method(cAO, "closed?",              rao_closed, 1);

  /* driver information */
  rb_define_method(cAO, "driver_id",         rao_driver_id, 1);
  rb_define_method(cAO, "default_driver_id", rao_default_driver_id, 0);
  rb_define_method(cAO, "driver_info",       rao_driver_info, 1);
  rb_define_method(cAO, "driver_info_list",  rao_driver_info_list, 0);
  rb_define_method(cAO, "file_extension",    rao_file_extension, 1);

  /* miscellaneous */
  rb_define_method(cAO, "bigendian?", rao_is_big_endian, 0);
}
