#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include "cao.h"

VALUE cAudio;
VALUE cAO_Live;
VALUE cAO_Info;
VALUE cAO_eDriverError;
VALUE cAO_eBadFormat;

/*
  ao_info構造体をRubyのHashに変換し返す。
*/
static VALUE
ao_info2hash(ao_info *info)
{
  int opt_count;
  VALUE ihash, optary;

  ihash  = rb_hash_new();
  optary = rb_ary_new();
  rb_hash_aset(ihash, ID2SYM(rb_intern("type")), INT2FIX(info->type));
  rb_hash_aset(ihash, ID2SYM(rb_intern("name")), rb_str_new2(info->name));
  rb_hash_aset(ihash, ID2SYM(rb_intern("short_name")), rb_str_new2(info->short_name));
  rb_hash_aset(ihash, ID2SYM(rb_intern("author")), rb_str_new2(info->author));
  rb_hash_aset(ihash, ID2SYM(rb_intern("comment")), rb_str_new2(info->comment));
  rb_hash_aset(ihash, ID2SYM(rb_intern("byte_format")), INT2FIX(info->preferred_byte_format));
  rb_hash_aset(ihash, ID2SYM(rb_intern("priority")), INT2FIX(info->priority));
  rb_hash_aset(ihash, ID2SYM(rb_intern("opt_count")), INT2FIX(info->option_count));
  for(opt_count=0; opt_count<info->option_count; opt_count++){
    rb_ary_push(optary, rb_str_new2(info->options[opt_count]));
  }
  rb_hash_aset(ihash, ID2SYM(rb_intern("options")), optary);
  return ihash;
}

/* driver information */
/*
 * call-seq: Audio::Info.driver_id(short_name)
 *
 * short_nameを元にDriverIDを検索する。
 * 見つからなかった場合はnilを返す。
 *
 * [arg1] short_name(String)
 * [return] DriverID or nil
 */
static VALUE
rao_driver_id(VALUE obj, VALUE short_name)
{
  int driver_id;

  Check_Type(short_name, T_STRING);
  driver_id = ao_driver_id(StringValuePtr(short_name));
  if (driver_id < 0){
    return Qnil;
  }
  return INT2FIX(driver_id);
}

/*
 * call-seq: Audio::Info.default_driver_id
 *
 * デフォルトのDriverIDを返す。
 * [return] DriverID
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
 * call-seq: Audio::Info.driver_info(driver_id)
 *
 * ドライバの情報を確認する。
 * ドライバ情報の要素順は以下の通り。
 * [name(String), short_name(String), author(String),
 * comment(String), preferred_byte_format(Fixnum),
 * priority(Fixnum), option_count(Fixnum), options(Array)]
 *
 * [arg1] DriverID
 * [return] Driver Information(Array)
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
  return ao_info2hash(info);;
}

/*
 * call-seq: Audio::Info.driver_info_list
 *
 * システムのlibaoがサポートしているドライバ一覧を配列で返す。
 * [return] ドライバ一覧の配列
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
    rb_ary_push(iary, ao_info2hash(info[info_count]));
  }
  return iary;
}

/*
 * call-seq: Audio::Info.file_extension(driver_id)
 *
 * ファイル出力時の拡張子の標準を確認する。
 *
 * [arg1] DriverID
 * [return] ext name(String)
 */
static VALUE
rao_file_extension(VALUE obj, VALUE driver_id)
{
  char *ext;

  Check_Type(driver_id, T_FIXNUM);
  ext = (char *)ao_file_extension(FIX2INT(driver_id));
  if (ext == NULL){
    rb_raise(cAO_eDriverError,
	     "This driver has no file extension associated with it or if this driver does not exist.");
  }
  return rb_str_new2(ext);
}

/* miscellaneous */
/*
 * call-seq: Audio::Info.bigendian?
 *
 * ホストの環境がビッグエンディアンであるか否かを調査する。
 * [return] ビッグエンディアンであればtrue、リトルエンディアンであればfalse
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

void
init_info(void)
{
  /*
   * Document-class: Audio::Info
   *
   * オーディオ出力デバイスに関する情報を取り扱うモジュール。
   */
  cAO_Info = rb_define_module_under(cAudio, "Info");

  /* driver information */
  rb_define_module_function(cAO_Info, "driver_id",         rao_driver_id, 1);
  rb_define_module_function(cAO_Info, "default_driver_id", rao_default_driver_id, 0);
  rb_define_module_function(cAO_Info, "driver_info",       rao_driver_info, 1);
  rb_define_module_function(cAO_Info, "driver_info_list",  rao_driver_info_list, 0);
  rb_define_module_function(cAO_Info, "file_extension",    rao_file_extension, 1);

  /* miscellaneous */
  rb_define_module_function(cAO_Info, "bigendian?", rao_is_big_endian, 0);
}
