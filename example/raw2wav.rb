#!/usr/bin/env ruby
#-*- coding: utf-8 -*-

#
# RAW to WAV converter
# (bit: 16bit, rate: 44100Hz, channel: 2ch,
#  endian: Little endian, matrix: default, option: default)
#

require 'ao'

unless ARGV[1]
  puts('ex) ./raw2wav.rb input.raw output.wav')
  exit(1)
end

input  = ARGV[0]
output = ARGV[1]
ao = AO.new
if File.file?(input)
    ao.open_file(ao.driver_id('wav'), output){|dev|
      File.open(input){|f|
        while buffer = f.read(4096)
          dev.play(buffer)
        end
      }
    }
end

