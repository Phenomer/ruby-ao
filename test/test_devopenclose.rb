#!/usr/bin/env ruby
#-*- coding: utf-8 -*-

#
# Device open/close parallel loop test
#

require 'ao'

list = Array.new
ao   = Audio::Output.new
loop{
  Thread.start{
    list.push(ao.open_live(ao.driver_id('null')))
    list.delete_at(rand(list.length)) if list.length > 5
  }
}
