Gem::Specification.new do |s|
  s.name              = "ao"
  s.version           = "0.0.2"
  s.summary           = "simple & cross platform audio output library"
  s.description       = "simple & cross platform audio output library(libao binding http://http://xiph.org/ao/)"
  s.author            = "Akito Miura"
  s.email             = "phenomer@g.hachune.net"
  s.homepage          = "http://bitbucket.org/phenomer/ruby-ao"
  s.has_rdoc          = true
  s.rdoc_options      = "--main README.rdoc"
  s.files             = ["LICENSE", "README.rdoc", "CHANGELOG.rdoc", 
                         "ext/cao.h", "ext/cao_output.c", 
                         "ext/cao_dev_list.c", "ext/cao_device.c",
                         "ext/cao_constant.c", "ext/cao_exception.c",
                         "ext/extconf.rb", "lib/ao.rb",
                         "example/beeplay.rb", "example/drivers.rb",
                         "example/raw2wav.rb", "example/rawplay.rb"]
  s.extra_rdoc_files  = ["README.rdoc", "CHANGELOG.rdoc",
                         "ext/cao.h", "ext/cao_output.c",
                         "ext/cao_dev_list.c", "ext/cao_device.c",
                         "ext/cao_exception.c", "ext/cao_constant.c"]
  s.extensions        = ["ext/extconf.rb"]
  s.require_paths     = ["lib"]
  s.required_ruby_version = Gem::Requirement.new(">=1.9.1")
end

