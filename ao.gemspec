Gem::Specification.new do |s|
  s.name              = "ao"
  s.version           = "0.1.5"
  s.summary           = "simple & cross platform audio output library"
  s.description       = "simple & cross platform audio output library(libao binding http://http://xiph.org/ao/)"
  s.author            = "Akito Miura"
  s.email             = "phenomer@g.hachune.net"
  s.homepage          = "http://bitbucket.org/phenomer/ruby-ao"
  s.rdoc_options      = "-m README.rdoc"
  s.files             = ["LICENSE", "README.rdoc", "CHANGELOG.rdoc", "Rakefile",
                         "ext/audio/queue.h", "ext/audio/queue.c",
                         "ext/audio/thread.h", "ext/audio/thread.c",
                         "ext/audio/cao.h", "ext/audio/output.c", 
                         "ext/audio/live.c", "ext/audio/file.c", "ext/audio/mixer.c",
                         "ext/audio/device.c", "ext/audio/info.c", "ext/audio/option.c",
                         "ext/audio/constant.c", "ext/audio/exception.c",
                         "ext/audio/extconf.rb", "lib/audio/output.rb",
                         "lib/ao.rb",
                         "example/beeplay.rb", "example/drivers.rb",
                         "example/raw2wav.rb", "example/rawplay.rb"]
  s.extra_rdoc_files  = ["README.rdoc", "CHANGELOG.rdoc",
                         "ext/audio/cao.h", "ext/audio/output.c",
                         "ext/audio/thread.c", "ext/audio/queue.c", "ext/audio/mixer.c",
                         "ext/audio/device.c", "ext/audio/info.c", "ext/audio/option.c",
                         "ext/audio/exception.c", "ext/audio/constant.c", "lib/audio/output.rb"]
  s.extensions        = ["ext/audio/extconf.rb"]
  s.require_paths     = ["lib"]
  s.required_ruby_version = Gem::Requirement.new(">=2.7.0")
  s.licenses          = ["MIT"]
end

