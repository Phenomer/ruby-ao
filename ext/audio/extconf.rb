#!/usr/bin/env ruby
#-*- coding: utf-8 -*-

require 'mkmf'

RbConfig::MAKEFILE_CONFIG['CC'] = ENV['CC'] if ENV['CC']

dir_config('audio')
have_header('pthread.h')
have_header('ao/ao.h')
have_header('ao/os_types.h')
have_header('ao/plugin.h')
have_library('ao')
have_library('pthread')
have_func('rb_thread_call_without_gvl')
create_makefile('audio/outputc')
