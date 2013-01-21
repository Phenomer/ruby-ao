#!/usr/bin/env ruby
#-*- coding: utf-8 -*-

#
# View driver infomation list
#

require 'ao'

def print_info(info)
  printf("  TYPE:        %d\n", info[:type]);
  printf("  NAME:        %s\n", info[:name]);
  printf("  SHORTNAME:   %s\n", info[:shortname]);
  printf("  AUTHOR:      %s\n", info[:author]);
  printf("  COMMENT:     %s\n", info[:comment]);
  printf("  BYTE_FORMAT: %d\n", info[:byte_format]);
  printf("  PRIORITY:    %d\n", info[:priority]);
  printf("  OPTIONS:     %d\n", info[:options]);
  info[:option].each{|opt|
    printf("    OPTION:      %s\n", opt);
  }
  puts()
end


ao = AO.new
puts("Default Driver:")
print_info(ao.default_driver_info)

ao.driver_info_list.each_with_index{|info, index|
  printf("DriverID: %d\n", index)
  print_info(info)
}

