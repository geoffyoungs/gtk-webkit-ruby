require 'mkmf'

begin
  require 'mkmf-gnome2'
rescue LoadError
  require 'rubygems'
  gem 'glib2'
  require 'mkmf-gnome2'
  %w[rbglib.h rbgtk.h rbpango.h rbatk.h].each do |header|
  	Gem.find_files(header).each do |f|
		$CFLAGS += " '-I#{File.dirname(f)}'"
	end
  end
end

have_func("rb_errinfo")
PKGConfig.have_package("gtk+-2.0") or exit(-1)
PKGConfig.have_package("webkit-1.0") or exit(-1)
have_header("dlfcn.h") or exit(-1)
have_header("errno.h") or exit(-1)
have_header("ffi.h") or exit(-1)
have_header("rbgobject.h") or exit(-1)
have_header("sys/mman.h") or exit(-1)
have_header("intern.h") or exit(-1)
have_header("webkit/webkit.h") or exit(-1)
have_header("webkit/webkitenumtypes.h") or exit(-1)
have_header("JavaScriptCore/JavaScript.h") or exit(-1)
have_library("dl") or exit(-1)
$LIBS += " -ldl"
have_library("ffi") or exit(-1)
$LIBS += " -lffi"

STDOUT.print("checking for new allocation framework... ") # for ruby-1.7
if Object.respond_to? :allocate
  STDOUT.print "yes
"
  $defs << "-DHAVE_OBJECT_ALLOCATE"
else
  STDOUT.print "no
"
end

top = File.expand_path(File.dirname(__FILE__) + '/..') # XXX
$CFLAGS += " " + ['glib/src'].map{|d|
  "-I" + File.join(top, d)
}.join(" ")

have_func("rb_define_alloc_func") # for ruby-1.8

#set_output_lib('libruby-webkit.a')
if /cygwin|mingw/ =~ RUBY_PLATFORM
  top = "../.."
  [
    ["glib/src", "ruby-glib2"],
  ].each{|d,l|
    $LDFLAGS << sprintf(" -L%s/%s", top, d)
    $libs << sprintf(" -l%s", l)
  }
end
begin
  srcdir = File.expand_path(File.dirname($0))

  begin

    obj_ext = "."+$OBJEXT

    $libs = $libs.split(/ /).uniq.join(' ')
    $source_files = Dir.glob(sprintf("%s/*.c", srcdir)).map{|fname|
      fname[0, srcdir.length+1] = ''
      fname
    }
    $objs = $source_files.collect do |item|
      item.gsub(/.c$/, obj_ext)
    end

    #
    # create Makefile
    #
    $defs << "-DRUBY_WEBKIT_COMPILATION"
    # $CFLAGS << $defs.join(' ')
    create_makefile("webkit", srcdir)
    raise Interrupt if not FileTest.exist? "Makefile"

    File.open("Makefile", "a") do |mfile|
      $source_files.each do |e|
        mfile.print sprintf("%s: %s
", e.gsub(/.c$/, obj_ext), e)
      end
    end
  ensure
    #Dir.chdir ".."
  end

  #create_top_makefile()
rescue Interrupt
  print "  [error] " + $!.to_s + "
"
end

