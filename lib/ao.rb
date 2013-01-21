#!/usr/bin/env ruby
#-*- coding: utf-8 -*-

require 'cao'

class AO < CAO
  class Device < CAO
    def initialize(dev_id)
      @dev_id = dev_id
    end
    undef :open_live, :open_file
    
    def play(buffer)
      super(@dev_id, buffer)
    end

    def close
      super(@dev_id)
    end

    def closed?
      super(@dev_id)
    end
  end

  @@init = false
  def initialize
    unless @@init
      @@init = true
      super
      at_exit{ shutdown }
    end
  end

  def open_live(driver_id=default_driver_id,
                bit=16, rate=44100, ch=2, endian=FMT_LITTLE,
                matrix=nil, option=nil)
    if block_given?
      device = Device.new(super(driver_id, bit, rate, ch, endian, matrix, option))
      begin
        yield device
      ensure
        device.close
      end
    else
      return Device.new(super(driver_id, bit, rate, ch, endian, matrix, option))
    end
  end

  def open_file(driver_id, file, ow=false,
                bit=16, rate=44100, ch=2, endian=FMT_LITTLE,
                matrix=nil, option=nil)
    if block_given?
      device = Device.new(super(driver_id, file, ow,
                                bit, rate, ch, endian, matrix, option))
      begin
        yield device
      ensure
        device.close
      end
    else
      return Device.new(super(driver_id, file, ow,
                              bit, rate, ch, endian, matrix, option))
    end
  end

  def close(device)
    device.close
  end

  def default_driver_info
    return info2hash(driver_info(default_driver_id))
  end

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
