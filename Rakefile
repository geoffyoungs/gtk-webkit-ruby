require 'rubygems'
gem 'rake-compiler'
require 'rake/extensiontask'
BASE_DIR = Dir.pwd
require 'rake/gempackagetask'

exts = []

FileList["ext/*/*.cr"].each do |cr|
	dir = File.dirname(cr)
	task(dir+"/extconf.rb") do
		sh 'rubber-generate', '--build-dir', dir, cr
	end
end

spec = Gem::Specification.new do |s|
	s.name = "gtk-webkit-ruby"
	s.author = "Geoff Youngs"
	s.email = "git@intersect-uk.co.uk"
	s.version = "0.0.2"
	s.homepage = "http://github.com/geoffyoungs/gtk-webkit-ruby"	
	s.summary = "Webkit bindings using rubber-generate"
	s.add_dependency("rubber-generate", ">= 0.0.12")
	s.platform = Gem::Platform::RUBY
	s.extensions = FileList["ext/*/extconf.rb"]
	s.files = Dir['ext/*/*.{c,cr,rd}']
s.description = <<-EOF
Gtk Webkit bindings for ruby.  Partial coverage sufficient to embed a webview in a ruby-gnome2 application.

Also initial/experimental support for allowing ruby code to be called by javascript & executing javascript
from ruby.

e.g
require 'gtk2'
require 'webkit'

v = WebKit::WebView.new
v.main_frame.setup_ruby
puts v.main_frame.exec_js("ruby_eval('RUBY_DESCRIPTION')")
puts v.main_frame.exec_js("document.root.innerHTML")
EOF
end
Rake::GemPackageTask.new(spec) do |pkg|
    pkg.need_tar = true
end
Rake::ExtensionTask.new("webkit")

namespace :gem do
end

task :prepare, exts
task :build_gem, ['prepare', 'package']

task :default, :compile

