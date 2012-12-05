Gtk Webkit Ruby
===============

Requires
--------
 * rubber-generate >= 0.0.17
 * rake-compiler, rake to build
 * ruby-gnome2 (including development files)
 * webkit (including development files)

Changes
-------
 * 0.0.4 - 16 Nov 2012
 * Use rubygems to find Ruby-GNOME2 installation (fixes #2, missing rbglib.h header file)
 * Wrap stored JS ref pointer with Data_Wrap_Struct (fixes GC bug)

Description
-----------
Gtk Webkit bindings for ruby.  Partial coverage sufficient to embed a webview in a ruby-gnome2 application.

Also experimental support for allowing ruby code to be called by javascript & executing javascript from ruby.

e.g

````ruby
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
````

License
-------
Copyright (c) 2011-2012 Geoff Youngs

This program is free software.
You can distribute/modify this program under the terms of
the GNU LESSER GENERAL PUBLIC LICENSE Version 2.1.

