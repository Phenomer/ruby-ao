#!/usr/bin/env ruby
#-*- coding: utf-8 -*-

#
# View driver infomation list
#

require 'ao'

def print_info(info)
  printf("  TYPE:        %d\n", info[0]);
  printf("  NAME:        %s\n", info[1]);
  printf("  SHORTNAME:   %s\n", info[2]);
  printf("  AUTHOR:      %s\n", info[3]);
  printf("  COMMENT:     %s\n", info[4]);
  printf("  BYTE_FORMAT: %d\n", info[5]);
  printf("  PRIORITY:    %d\n", info[6]);
  printf("  OPTIONS:     %d\n", info[7]);
  info[8].each{|opt|
    printf("    OPTION:      %s\n", opt);
  }
  puts()
end



AO.initialize 
at_exit{ AO.shutdown; puts('done.') }

puts("Default Driver:")
info = AO.driver_info(AO.default_driver_id)
print_info(info)

AO.driver_info_list.each_with_index{|info, index|
  printf("DriverID: %d\n", index)
  print_info(info)
}

