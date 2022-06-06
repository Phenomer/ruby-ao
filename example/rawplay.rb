#!/usr/bin/env ruby
#-*- coding: utf-8 -*-

#
# Simple RAW Audio Player
# (bits: 16bit, rate: 44100Hz, channels: 2ch,
#  byte_format: Little endian,
#  matrix: default, option: default)
#

require 'audio/output'

ARGV.each{|file|
  if File.file?(file)
    Audio::LiveOutput.new(driver_id: Audio::Info.default_driver_id, bits: 16, rate: 44100, channels: 2,
                          byte_format: Audio::Info::FMT_LITTLE){|ao|
      File.open(file){|f|
        while buffer = f.read(4096)
          ao.play(buffer)
        end
      }
    }
  end
}
