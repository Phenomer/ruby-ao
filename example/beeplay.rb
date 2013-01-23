#!/usr/bin/env ruby
#-*- coding: utf-8 -*-

#
# Beep Keyboard
#

require 'ao'
require 'io/console'

def gen_sample(freq, volume=0.5, sec=0.1, rate=44100)
  buffer = Array.new
  0.upto(rate * sec){|i|
    sample = volume * 32768.0 * Math.sin(2 * Math::PI * freq * (i.to_f/rate))
    buffer.push(sample)
    buffer.push(sample)
  }
  return buffer.pack('s*')
end

freqlist = {
  'c' => 261.63, 'd' => 293.66, 'e' => 329.63,
  'f' => 349.23, 'g' => 392.00, 'a' => 440.00,
  'b' => 493.88, 'C' => 523.25, 'D' => 587.33,
  'E' => 659.26, 'F' => 698.46, 'G' => 783.99,
  'A' => 880.00, 'B' => 987.77
}

ao = AO.new
ao.open_live{|dev|
  dev.play(gen_sample(freqlist['B'])+gen_sample(freqlist['E']))
  IO.console.noecho{|io|
    while freq = freqlist[io.getch]
      dev.play(gen_sample(freq))
    end
  }
}
