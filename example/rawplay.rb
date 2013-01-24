#!/usr/bin/env ruby
#-*- coding: utf-8 -*-

#
# Simple RAW Audio Player
# (bit: 16bit, rate: 44100Hz, channel: 2ch,
#  endian: Little endian, matrix: default, option: default)
#

require 'ao'

ao = Audio::Output.new
ARGV.each{|file|
  if File.file?(file)
    ao.open_live{|dev|
      File.open(file){|f|
        while buffer = f.read(4096)
          dev.play(buffer)
        end
      }
    }
  end
}
