#!/usr/bin/env ruby
#-*- coding: utf-8 -*-

#
# RAW to WAV converter
# (bit: 16bit, rate: 44100Hz, channel: 2ch,
#  endian: native, matrix: default, option: default)
#

require 'audio/output'

unless ARGV[1]
  puts('ex) ./raw2wav.rb input.raw output.wav')
  exit(1)
end

input  = ARGV[0]
output = ARGV[1]
ao = 
raise unless File.file?(input)
Audio::FileOutput.new(driver_id: Audio::Info.driver_id('wav'),
                      filename: output){|ao|
  File.open(input){|f|
    while buffer = f.read(4096)
      ao.play(buffer)
    end
  }
}
