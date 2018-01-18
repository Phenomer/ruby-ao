#-*- coding: utf-8 -*-

require 'audio/outputc'

class Audio::LiveOutput < Audio::LiveOutputC
  def initialize(
        driver_id:
          Audio::Info.default_driver_id,
        bits: 16, rate: 44100, channels: 2,
        byte_format:
          Audio::Info::FMT_NATIVE,
        matrix: nil, options: nil, thread: nil)
    ao = super(driver_id, bits, rate, channels,
               byte_format, matrix, options, thread)
    if block_given?
      begin
        yield ao
      ensure
        ao.close
      end
    end
  end
end

class Audio::FileOutput < Audio::FileOutputC
  def initialize(
        driver_id:
          Audio::Info.driver_id('wav'),
        filename:, overwrite: false,
        bits: 16, rate: 44100, channels: 2,
        byte_format:
          Audio::Info::FMT_NATIVE,
        matrix: nil, options: nil, thread: nil)
    ao = super(driver_id, filename, overwrite,
               bits, rate, channels, byte_format,
               matrix, options, thread)
    if block_given?
      begin
        yield ao
      ensure
        ao.close
      end
    end
  end
end
