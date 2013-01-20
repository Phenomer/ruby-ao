#!/usr/bin/env ruby
#-*- coding: utf-8 -*-

require 'ao'

class RubyAO
  @@init = false
  def initialize
    unless @@init
      AO.initialize
      at_exit{ AO.shutdown }
    end
  end

  def open_live(driver_id=AO.default_driver_id,
                bit=16, rate=44100, ch=2, endian=AO::FMT_LITTLE,
                matrix=nil, option=nil)
    if block_given?
      dev = AO.open_live(driver_id, bit, rate, ch, endian, matrix, option)
      while yield buffer
        play(dev, buffer)
      end
      close(dev)
    else
      return AO.open_live(driver_id, bit, rate, ch, endian, matrix, option)
    end
  end

  def open_file(driver_id, file, ow=false,
                bit=16, rate=44100, ch=2, endian=AO::FMT_LITTLE,
                matrix=nil, option=nil)
    if block_given?
      dev = AO.open_file(driver_id, file, ow,
                         bit, rate, ch, endian, matrix, option)
      while yield buffer
        play(dev, buffer)
      end
      close(dev)
    else
      return AO.open_file(driver_id, file, ow,
                          bit, rate, ch, endian, matrix, option)
    end
  end

  def play(dev, buffer)
    return AO.play(dev, buffer, buffer.bytesize)
  end

  def close(dev)
    return AO.close(dev)
  end

  def default_driver_id
    return AO.default_driver_id
  end

  def default_driver_info
    return info2hash(AO.default_driver_id)
  end

  def driver_info_list
    return AO.driver_info_list.collect{|info|
      info2hash(info)
    }
  end

  def bigendian?
    return AO.bigendian?
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
