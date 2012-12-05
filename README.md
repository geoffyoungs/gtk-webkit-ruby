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
Copyright (C) 2011-2012 Geoff Youngs

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

