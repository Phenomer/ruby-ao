#!/usr/bin/env ruby
#-*- coding: utf-8 -*-

require 'cao'

class Audio::Output < Audio::BasicOutput
  @@init = false
  def initialize
    unless @@init
      @@init = true
      super
      at_exit{ shutdown }
    end
  end

  # オーディオ出力デバイスを開く。
  # 引数に指定するmatrixとoptionについては以下を参照。
  # [matrix] http://xiph.org/ao/doc/ao_sample_format.html
  # [option] http://xiph.org/ao/doc/drivers.html
  # optionはKey-Valueを要素に含む多次元配列を設定する。
  # 特に設定が必要なければnilで構わない。
  # ex) ALSAドライバのデバイスをhw:1に設定する場合は [["dev", "hw:1"]] を渡す。
  #
  # [arg1] DriverID(fixnum, default: default_driver_id)
  # [arg2] bit(fixnum, default: 16)
  # [arg3] rate(fixnum, default: 44100)
  # [arg4] channel(fixnum, default: 2)
  # [arg5] endian(fixnum, default: FMT_NATIVE)
  # [arg6] matrix(String or nil, default: nil)
  # [arg7] option(Array or nil, default: nil)
  # [return] AO::Device
  def open_live(driver_id=default_driver_id,
                bit=16, rate=44100, ch=2, endian=FMT_NATIVE,
                matrix=nil, option=nil)
    if block_given?
      device = BasicDevice.new(super(driver_id, bit, rate, ch, endian, matrix, option))
      begin
        yield device
      ensure
        device.close
      end
      return nil
    else
      return BasicDevice.new(super(driver_id, bit, rate, ch, endian, matrix, option))
    end
  end
  alias :open :open_live

  # ファイルを開く。
  # 引数に指定するmatrixとoptionについては以下を参照。
  # [matrix] http://xiph.org/ao/doc/ao_sample_format.html
  # [option] http://xiph.org/ao/doc/drivers.html
  # optionはKey-Valueを要素に含む多次元配列を設定する。
  # 特に設定が必要なければnilで構わない。
  # ex) RAWドライバの出力エンディアンをビッグエンディアンに設定する場合は [["byteirder", "big"]] を渡す。
  #
  # [arg1] DriverID(fixnum)
  # [arg2] filepath(String)
  # [arg3] overwrite?(true of false, default: false)
  # [arg4] bit(fixnum, default: 16)
  # [arg5] rate(fixnum, default: 44100)
  # [arg6] channel(fixnum, default: 2)
  # [arg7] endian(fixnum, default: FMT_NATIVE)
  # [arg8] matrix(String or nil, default: nil)
  # [arg9] option(Array or nil, default: nil)
  # [return] AO::Device
  def open_file(driver_id, file, ow=false,
                bit=16, rate=44100, ch=2, endian=FMT_NATIVE,
                matrix=nil, option=nil)
    if block_given?
      device = BasicDevice.new(super(driver_id, file, ow,
                                     bit, rate, ch, endian, matrix, option))
      begin
        yield device
      ensure
        device.close
      end
    else
      return BasicDevice.new(super(driver_id, file, ow,
                                   bit, rate, ch, endian, matrix, option))
    end
  end

  # デフォルトドライバの情報をハッシュで返す。
  # ハッシュに含まれる情報は以下の通り。
  # [:type]        Fixnum(ドライバの種類)
  # [:name]        String(ドライバ名)
  # [:shortname]   String(ドライバの短縮名)
  # [:author]      String(ドライバの作成者名)
  # [:comment]     String(コメント)
  # [:byte_format] Fixnum(ドライバのバイトフォーマット)
  # [:priority]    Fixnum(優先度)
  # [:options]     Fixnum(オプションの数)
  # [:option]      Array(オプションのkeyとなる文字列の配列)
  #
  # デフォルトのドライバが存在しなかった場合nilを返す。
  #
  # [return] Hash
  def default_driver_info
    ddid = default_driver_id
    return nil unless ddid
    return info2hash(driver_info(ddid))
  end

  # システムのlibaoでサポートされているドライバ情報のリストをArrayで返す。
  # [return] Array
  def driver_info_list
    return super.collect{|info|
      info2hash(info)
    }
  end

  private
  def info2hash(info)
    return {
      :type        => info[0],
      :name        => info[1],
      :shortname   => info[2],
      :author      => info[3],
      :comment     => info[4],
      :byte_format => info[5],
      :priority    => info[6],
      :options     => info[7],
      :option      => info[8]
    }
  end
end
