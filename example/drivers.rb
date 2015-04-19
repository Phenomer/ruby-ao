#!/usr/bin/env ruby
#-*- coding: utf-8 -*-

#
# View driver infomation list
#

require 'audio/output'

def print_info(info)
  printf("  TYPE:        %d\n", info[:type]);
  printf("  NAME:        %s\n", info[:name]);
  printf("  SHORTNAME:   %s\n", info[:short_name]);
  printf("  AUTHOR:      %s\n", info[:author]);
  printf("  COMMENT:     %s\n", info[:comment]);
  printf("  BYTE_FORMAT: %d\n", info[:byte_format]);
  printf("  PRIORITY:    %d\n", info[:priority]);
  printf("  OPT_COUNT:   %d\n", info[:opt_count]);
  info[:options].each{|opt|
    printf("    OPTIONS:      %s\n", opt);
  }
  puts()
end

Audio::Info.driver_info_list.each_with_index{|info, index|
  printf("DriverID: %d\n", index)
  print_info(info)
}

puts("Default Driver:")
print_info(Audio::Info.driver_info(Audio::Info.default_driver_id))
