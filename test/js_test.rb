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


	class Foo
		class << self
			attr_reader :objects
		end
		@objects = []

		def initialize()
			@init = true
			puts "Making a new object..."
			Foo.objects << self
		end
	
		def do_something
			puts "Do something!"
		end

		def toString
			"#{self.class.name}:#{object_id}"
		end
	end

	class Iter
		def initialize()
			@num = 0
		end
    def id
      object_id
    end
		def nextNumber
			@num += 1
		end
		def value
			@num
		end
	end


	def test_class
		@webview.main_frame.add_ruby_class('Foo', Foo)
		@webview.main_frame.exec_js <<-EOF
var s = new Foo();
//console.log(s);
//console.log(s.do_something);
s.do_something();
EOF
		assert_equal 1, Foo.objects.size
		@webview.main_frame.add_ruby_class('Iter', Iter)

    js_id = false
		@webview.main_frame.add_js_api('id_is') { |a| js_id = a }

		assert @webview.main_frame.exec_js(<<-EOF)
var i = new Iter();
var j = new Iter();

i.nextNumber()
i.nextNumber()
j.nextNumber()

id_is(i.id())

if (j.value() < i.value()) {
	true;
} else {
	false;
}
EOF

    ids = []
    ObjectSpace.each_object(Iter) { |iter| ids << iter.id }
    assert ids.include?(js_id)

    GC.enable
    GC.start
    # Check GC doesn't crash out
    assert true
  end
end

