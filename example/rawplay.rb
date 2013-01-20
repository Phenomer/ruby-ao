#!/usr/bin/env ruby
#-*- coding: utf-8 -*-

#
# Simple RAW Audio Player
# (bit: 16bit, rate: 44100Hz, channel: 2ch,
#  endian: Little endian, matrix: "L,R")
#

require 'ao'

AO.initialize
at_exit{
  AO.shutdown
  puts('done.')
}

dev = AO.open_live(AO.default_driver_id, 16, 44100, 2,
                   AO::FMT_LITTLE, "L,R", nil)
trap(:INT){AO.close(dev); exit(0)}
ARGV.each{|file|
  next unless File.file?(file)
  printf("play: %s\n", file)
  File.open(file){|f|
    while buffer = f.read(8192)
      AO.play(dev, buffer)
    end
  }
}
AO.close(dev)
