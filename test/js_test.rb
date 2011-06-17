require 'test/unit'
require 'gtk2'
$:.unshift 'ext/webkit'
$:.unshift 'lib'
$:.unshift 'x86_64-linux'
require 'webkit'

class JsTest < Test::Unit::TestCase
	def setup
		@webview = WebKit::WebView.new
		@webview.load_string("<html><head><title>Foo</title></head><body></body></html>", 
							"text/html", "UTF-8", "file://home")
	end

	def test_exec_js
		assert_equal 6, @webview.main_frame.exec_js("3 + 3")
		assert_equal 'foobar', @webview.main_frame.exec_js("'foo' + 'bar'")
		assert_equal false, @webview.main_frame.exec_js("3 == 2")
	end

	def test_api
		@webview.main_frame.add_js_api('test_add') { |a,b| a + b }

		assert_equal 250, @webview.main_frame.exec_js("test_add(100, 150)")
		assert_equal 'foobarr', @webview.main_frame.exec_js("test_add('foo','barr')")

		@webview.main_frame.add_js_api('test_nil') { nil }
		@webview.main_frame.add_js_api('test_true') { true }
		@webview.main_frame.add_js_api('test_false') { false }

		assert_equal nil, @webview.main_frame.exec_js("test_nil()")
		assert_equal true, @webview.main_frame.exec_js("test_true()")
		assert_equal false, @webview.main_frame.exec_js("test_false()")
	end
end

