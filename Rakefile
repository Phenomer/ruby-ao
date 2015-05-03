#-*- coding: utf-8 -*-

require 'rake/extensiontask'

Rake::ExtensionTask.new('audio') do |ext|
  ext.name = 'outputc'
  ext.lib_dir  ='lib/audio'
  ext.cross_compile = true
  ext.cross_platform = ENV['CROSS_COMPILE'] || 'armv6j-linux-gnueabi'
  ext.cross_compiling do |gem_spec|
    gem_spec.post_install_message = "You installed the binary version of this gem!"
  end
end

