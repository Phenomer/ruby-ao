#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include "cao.h"

/*
  引数に設定されたサンプルフォーマットをao_sample_format構造体に設定する。
  成功したらao_sample_format構造体へのポインタを返す。
*/
ao_sample_format *
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
    if ((format->matrix = strndup(StringValuePtr(matrix),
				  RSTRING_LENINT(matrix))) == NULL){
	rb_raise(cAO_eAOError, "Memory allocation failure.");
    }
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
 */
void
free_format(ao_sample_format *format)
{
  if (format == NULL){ return; }
  if (format->matrix != NULL){
    free(format->matrix);
  }
  free(format);
  return;
}

/*
  Rubyの配列からデバイスオプションを読み込む。
  配列ではなくnilであった場合はNULLを返す。
*/
ao_option *
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
 * call-seq: ao.append_global_option(key, value)
 *
 * libao全体で参照されるオプションを設定する。
 * オプションはKey-Value形式で設定する。
 * 各ドライバ毎のオプションについては下記を参照。
 * http://xiph.org/ao/doc/drivers.html
 *
 * [arg1] key(String)
 * [arg2] value(String)
 * [return] true
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

void
init_option(void)
{
  rb_define_method(cAO_Info, "append_global_option", rao_append_global_option, 2);
}
