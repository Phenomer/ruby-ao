#include "cao.h"

VALUE cAudio;
VALUE cAO;
VALUE cAO_eAOError;
VALUE cAO_eNoDriver;
VALUE cAO_eNotFile;
VALUE cAO_eNotLive;
VALUE cAO_eBadOption;
VALUE cAO_eDeviceError;
VALUE cAO_eDriverError;
VALUE cAO_eFileError;
VALUE cAO_eFileExists;
VALUE cAO_eBadFormat;
VALUE cAO_eUnknownError;

void init_exception(void)
{
  /*
   * Document-class: Audio::BasicOutput::Error
   *
   * Audio::BasicOutputに関するエラーは
   * Audio::BasicOutput::Errorを継承する。
   */
  cAO_eAOError =
    rb_define_class_under(cAO, "Error", rb_eStandardError);

  /*
   * Document-class: Audio::BasicOutput::NoDriver
   *
   * ドライバが見つからない時に発生する。
   */
  cAO_eNoDriver =
    rb_define_class_under(cAO, "NoDriver",     cAO_eAOError);

  /*
   * Document-class: Audio::BasicOutput::NotFile
   *
   * ドライバがファイル出力用のものではない時に発生する。
   */
  cAO_eNotFile =
    rb_define_class_under(cAO, "NotFile",      cAO_eAOError);

  /*
   * Document-class: Audio::BasicOutput::NotLive
   *
   * ドライバがデバイス出力用のものではない時に発生する。
   */
  cAO_eNotLive =
    rb_define_class_under(cAO, "NotLive",      cAO_eAOError);

  /*
   * Document-class: Audio::BasicOutput::BadOption
   *
   * ドライバに設定したオプションの値が不正である時に発生する。
   */
  cAO_eBadOption =
    rb_define_class_under(cAO, "BadOption",    cAO_eAOError);

  /*
   * Document-class: Audio::BasicOutput::DriverError
   *
   * ドライバに関するエラー。
   */
  cAO_eDriverError =
    rb_define_class_under(cAO, "DriverError",  cAO_eAOError);

  /* 
   * Document-class: Audio::BasicOutput::DeviceError
   *
   * デバイスに関するエラー。
   */
  cAO_eDeviceError =
    rb_define_class_under(cAO, "DeviceError",  cAO_eAOError);

  /*
   * Document-class: Audio::BasicOutput::FileError
   *
   * ファイル出力に関するエラー。
   */
  cAO_eFileError =
    rb_define_class_under(cAO, "FileError",    cAO_eAOError);

  /* 
   * Document-class: Audio::BasicOutput::FileExists
   *
   * ファイル出力時の上書き禁止が設定されている時に
   * 既にファイルが存在する場合に発生する。
   */
  cAO_eFileExists =
    rb_define_class_under(cAO, "FileExists",   cAO_eAOError);

  /*
   * Document-class: Audio::BasicOutput::Badformat
   *
   * オーディオ出力フォーマットが不正である場合発生する。
   */
  cAO_eBadFormat =
    rb_define_class_under(cAO, "BadFormat",    cAO_eAOError);

  /* 
   * Document-class: Audio::BasicOutput::UnknownError
   *
   * 不明なエラー。
   */
  cAO_eUnknownError =
    rb_define_class_under(cAO, "UnknownError", cAO_eAOError);
  return;
}
