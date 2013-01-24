require 'rubygems'
gem 'rake-compiler'
require 'rake/extensiontask'
BASE_DIR = Dir.pwd
require 'rubygems/package_task'
require 'rake/testtask'

$: << './lib'

require 'webkit/version'
exts = []

namespace :prepare do
FileList["ext/*/*.cr"].each do |cr|
	dir = File.dirname(cr)
	name = File.basename(dir)
	desc "Generate source for #{name}"
	task(name.intern) do
		sh 'rubber-generate', '--build-dir', dir, cr
	end
end
end

spec = Gem::Specification.new do |s|
	s.name = "gtk-webkit-ruby"
	s.author = "Geoff Youngs"
	s.email = "git@intersect-uk.co.uk"
	s.version = WebKit::BINDINGS_VERSION
	s.homepage = "http://github.com/geoffyoungs/gtk-webkit-ruby"
	s.summary = "Webkit bindings using rubber-generate"
	s.add_dependency("rubber-generate", ">= 0.0.17")
	s.platform = Gem::Platform::RUBY
	s.extensions = FileList["ext/*/extconf.rb"]
	s.files = FileList['ext/*/*.{c,h,cr,rd}'] + ['Rakefile', 'README.md'] + FileList['lib/**/*.rb']
s.description = <<-EOF
Gtk Webkit bindings for ruby.  Partial coverage sufficient to embed a webview in a Ruby-GNOME2 application.

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
Gem::PackageTask.new(spec) do |pkg|
    pkg.need_tar = true
end
Rake::ExtensionTask.new("webkit", spec)

Rake::TestTask.new do |t|
	t.test_files = FileList['test/*_test.rb']
end

task :default, :compile

