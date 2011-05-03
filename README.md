Gtk Webkit Ruby
===============

Requires
--------
* rubber-generate >= 0.0.12
* ruby-gnome2 (including development files)
* webkit (including development files)

Description
-----------

Gtk Webkit bindings for ruby.  Partial coverage sufficient to embed a webview in a ruby-gnome2 application.

Also initial/experimental support for allowing ruby code to be called by javascript & executing javascript from ruby.

e.g

	require 'gtk2'
	require 'webkit'
	require 'digest/md5'

	v = WebKit::WebView.new
	v.main_frame.setup_ruby
	
	v.main_frame.add_js_api('md5') do |what|
		Digest::MD5.hexdigest(what)
	end
	
	puts v.main_frame.exec_js("ruby_eval('RUBY_DESCRIPTION')")
	puts v.main_frame.exec_js("md5('foo')")
	puts Digest::MD5.hexdigest('foo')

