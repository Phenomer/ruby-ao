#!/usr/bin/env ruby
#-*- coding: utf-8 -*-

require 'mkmf'

dir_config('ao')
have_header('ao/ao.h')
have_header('ao/os_types.h')
have_header('ao/plugin.h')
have_library('ao')
have_func('rb_thread_call_without_gvl')
create_makefile('cao')
