/* Includes */
#include <ruby.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "webkit/webkit.h"
#include "JavaScriptCore/JavaScript.h"

/* Setup types */
/* Try not to clash with other definitions of bool... */
typedef int rubber_bool;
#define bool rubber_bool

/* Prototypes */
#include "rbglib.h"

#include "rbgtk.h"

#if defined(G_PLATFORM_WIN32) && !defined(RUBY_GTK2_STATIC_COMPILATION)
#  ifdef RUBY_GTK2_COMPILATION
#    define RUBY_GTK2_VAR __declspec(dllexport)
#  else
#    define RUBY_GTK2_VAR extern __declspec(dllimport)
#  endif
#else
#  define RUBY_GTK2_VAR extern
#endif

RUBY_GTK2_VAR VALUE mGtk;
RUBY_GTK2_VAR VALUE mGdk;

#define RBGTK_INITIALIZE(obj,gtkobj) (rbgtk_initialize_gtkobject(obj, GTK_OBJECT(gtkobj)))
static VALUE mWebKit;
static VALUE
WebKit_CLASS_set_web_database_path(VALUE self, VALUE __v_path);
static VALUE
WebKit_CLASS_remove_all_web_databases(VALUE self);
static VALUE
WebKit_CLASS_set_default_web_database_quota(VALUE self, VALUE __v_quota);
static VALUE cJavascriptError;
static VALUE cWebSettings;
static VALUE
WebSettings_initialize(VALUE self);
static VALUE cWebFrame;
static VALUE
WebFrame_exec_js(VALUE self, VALUE __v_js);
static VALUE
WebFrame_add_ruby_eval(VALUE self);
static VALUE
WebFrame_add_js_api(VALUE self, VALUE __v_name);
static VALUE
WebFrame_load_string(VALUE self, VALUE __v_content, VALUE __v_mime_type, VALUE __v_encoding, VALUE __v_base_uri);
static VALUE cWebView;
static VALUE
WebView_initialize(VALUE self);
static VALUE
WebView_open(VALUE self, VALUE __v_uri);
static VALUE
WebView_execute_script(VALUE self, VALUE __v_script);
static VALUE
WebView_set_settings(VALUE self, VALUE __v_settings);
static VALUE
WebView_load_string(VALUE self, VALUE __v_content, VALUE __v_mime_type, VALUE __v_encoding, VALUE __v_base_uri);
static VALUE
WebView_load_uri(VALUE self, VALUE __v_uri);
static VALUE
WebView_main_frame(VALUE self);
static VALUE
WebView_focused_frame(VALUE self);
static VALUE
WebView_progress(VALUE self);
static VALUE
WebView_title(VALUE self);
static VALUE
WebView_uri(VALUE self);
static VALUE
WebView_reload(VALUE self);
static VALUE
WebView_reload_bypass_cache(VALUE self);
static VALUE
WebView_set_custom_encoding(VALUE self, VALUE __v_encoding);
static VALUE
WebView_stop_loading(VALUE self);
static VALUE cWebResource;
static VALUE
WebResource_encoding(VALUE self);
static VALUE
WebResource_frame_name(VALUE self);
static VALUE
WebResource_mime_type(VALUE self);
static VALUE
WebResource_uri(VALUE self);
static VALUE
WebResource_data(VALUE self);
static VALUE
WebResource_data_equals(VALUE self, VALUE data);
static VALUE cWebNetworkRequest;
static VALUE
WebNetworkRequest_uri(VALUE self);
static VALUE
WebNetworkRequest_uri_equals(VALUE self, VALUE __v_uri);
static VALUE cWebNetworkResponse;
static VALUE
WebNetworkResponse_uri(VALUE self);
static VALUE
WebNetworkResponse_uri_equals(VALUE self, VALUE __v_uri);
static VALUE _gcpool_RubyFunc = Qnil;
static void __gcpool_RubyFunc_add(VALUE val);
static void __gcpool_RubyFunc_del(VALUE val);
#define RUBYFUNC_ADD(val) __gcpool_RubyFunc_add(val)
#define RUBYFUNC_DEL(val) __gcpool_RubyFunc_del(val)

/* Inline C code */

#include <intern.h>

#include "javascript.h"

static inline VALUE strOrNil(const char *str) {
	if (str) {
		return rb_str_new2(str);
	} else {
		return Qnil;
	}
}


/* Code */
static VALUE
WebKit_CLASS_set_web_database_path(VALUE self, VALUE __v_path)
{
  char * path; char * __orig_path;
  __orig_path = path = ( NIL_P(__v_path) ? NULL : StringValuePtr(__v_path) );

#line 31 "/home/geoff/Projects/gtk-webkit-ruby/ext/webkit/webkit.cr"
  webkit_set_web_database_directory_path(path);
 
  return self;
}

static VALUE
WebKit_CLASS_remove_all_web_databases(VALUE self)
{

#line 34 "/home/geoff/Projects/gtk-webkit-ruby/ext/webkit/webkit.cr"
  webkit_remove_all_web_databases();
 
  return Qnil;
}

static VALUE
WebKit_CLASS_set_default_web_database_quota(VALUE self, VALUE __v_quota)
{
  guint64 quota; guint64 __orig_quota;
  __orig_quota = quota = rb_num2ull(__v_quota);

#line 37 "/home/geoff/Projects/gtk-webkit-ruby/ext/webkit/webkit.cr"
  webkit_set_default_web_database_quota(quota);
 
  return self;
}

static VALUE
WebSettings_initialize(VALUE self)
{

#line 43 "/home/geoff/Projects/gtk-webkit-ruby/ext/webkit/webkit.cr"
  G_INITIALIZE(self, webkit_web_settings_new());
 
  return Qnil;
}

static VALUE
WebFrame_exec_js(VALUE self, VALUE __v_js)
{
  VALUE __p_retval = Qnil;
  char * js; char * __orig_js;
  WebKitWebFrame *_self = ((WebKitWebFrame*)RVAL2GOBJ(self));
  __orig_js = js = ( NIL_P(__v_js) ? NULL : StringValuePtr(__v_js) );

#line 50 "/home/geoff/Projects/gtk-webkit-ruby/ext/webkit/webkit.cr"
  do { __p_retval = javascript_exec(webkit_web_frame_get_global_context(_self), js); goto out; } while(0);
out:
  return __p_retval;
}

static VALUE
WebFrame_add_ruby_eval(VALUE self)
{
  WebKitWebFrame *_self = ((WebKitWebFrame*)RVAL2GOBJ(self));

#line 53 "/home/geoff/Projects/gtk-webkit-ruby/ext/webkit/webkit.cr"
  javascript_add_ruby_eval(webkit_web_frame_get_global_context(_self));
 
  return Qnil;
}

static VALUE
WebFrame_add_js_api(VALUE self, VALUE __v_name)
{
  char * name; char * __orig_name;
  WebKitWebFrame *_self = ((WebKitWebFrame*)RVAL2GOBJ(self));
  __orig_name = name = ( NIL_P(__v_name) ? NULL : StringValuePtr(__v_name) );
  VALUE block = rb_block_proc();

#line 56 "/home/geoff/Projects/gtk-webkit-ruby/ext/webkit/webkit.cr"
  javascript_add_ruby_fn(webkit_web_frame_get_global_context(_self), name, block);
 
  return Qnil;
}

static VALUE
WebFrame_load_string(VALUE self, VALUE __v_content, VALUE __v_mime_type, VALUE __v_encoding, VALUE __v_base_uri)
{
  char * content; char * __orig_content;
  char * mime_type; char * __orig_mime_type;
  char * encoding; char * __orig_encoding;
  char * base_uri; char * __orig_base_uri;
  WebKitWebFrame *_self = ((WebKitWebFrame*)RVAL2GOBJ(self));
  __orig_content = content = ( NIL_P(__v_content) ? NULL : StringValuePtr(__v_content) );
  __orig_mime_type = mime_type = ( NIL_P(__v_mime_type) ? NULL : StringValuePtr(__v_mime_type) );
  __orig_encoding = encoding = ( NIL_P(__v_encoding) ? NULL : StringValuePtr(__v_encoding) );
  __orig_base_uri = base_uri = ( NIL_P(__v_base_uri) ? NULL : StringValuePtr(__v_base_uri) );

#line 59 "/home/geoff/Projects/gtk-webkit-ruby/ext/webkit/webkit.cr"
  webkit_web_frame_load_string(_self, content, mime_type, encoding, base_uri);
 
  return Qnil;
}

static VALUE
WebView_initialize(VALUE self)
{

#line 65 "/home/geoff/Projects/gtk-webkit-ruby/ext/webkit/webkit.cr"
  RBGTK_INITIALIZE(self, webkit_web_view_new());
 
  return Qnil;
}

static VALUE
WebView_open(VALUE self, VALUE __v_uri)
{
  char * uri; char * __orig_uri;
  WebKitWebView *_self = ((WebKitWebView*)RVAL2GOBJ(self));
  __orig_uri = uri = ( NIL_P(__v_uri) ? NULL : StringValuePtr(__v_uri) );

#line 68 "/home/geoff/Projects/gtk-webkit-ruby/ext/webkit/webkit.cr"
  webkit_web_view_open(_self, uri);
 
  return Qnil;
}

static VALUE
WebView_execute_script(VALUE self, VALUE __v_script)
{
  char * script; char * __orig_script;
  WebKitWebView *_self = ((WebKitWebView*)RVAL2GOBJ(self));
  __orig_script = script = ( NIL_P(__v_script) ? NULL : StringValuePtr(__v_script) );

#line 71 "/home/geoff/Projects/gtk-webkit-ruby/ext/webkit/webkit.cr"
  webkit_web_view_execute_script(_self, script);
 
  return Qnil;
}

static VALUE
WebView_set_settings(VALUE self, VALUE __v_settings)
{
  WebKitWebSettings * settings; WebKitWebSettings * __orig_settings;
  WebKitWebView *_self = ((WebKitWebView*)RVAL2GOBJ(self));
  __orig_settings = settings = RVAL2GOBJ(__v_settings);

#line 74 "/home/geoff/Projects/gtk-webkit-ruby/ext/webkit/webkit.cr"
  webkit_web_view_set_settings(_self, settings);
 
  return self;
}

static VALUE
WebView_load_string(VALUE self, VALUE __v_content, VALUE __v_mime_type, VALUE __v_encoding, VALUE __v_base_uri)
{
  char * content; char * __orig_content;
  char * mime_type; char * __orig_mime_type;
  char * encoding; char * __orig_encoding;
  char * base_uri; char * __orig_base_uri;
  WebKitWebView *_self = ((WebKitWebView*)RVAL2GOBJ(self));
  __orig_content = content = ( NIL_P(__v_content) ? NULL : StringValuePtr(__v_content) );
  __orig_mime_type = mime_type = ( NIL_P(__v_mime_type) ? NULL : StringValuePtr(__v_mime_type) );
  __orig_encoding = encoding = ( NIL_P(__v_encoding) ? NULL : StringValuePtr(__v_encoding) );
  __orig_base_uri = base_uri = ( NIL_P(__v_base_uri) ? NULL : StringValuePtr(__v_base_uri) );

#line 77 "/home/geoff/Projects/gtk-webkit-ruby/ext/webkit/webkit.cr"
  webkit_web_view_load_string(_self, content, mime_type, encoding, base_uri);
 
  return Qnil;
}

static VALUE
WebView_load_uri(VALUE self, VALUE __v_uri)
{
  char * uri; char * __orig_uri;
  WebKitWebView *_self = ((WebKitWebView*)RVAL2GOBJ(self));
  __orig_uri = uri = ( NIL_P(__v_uri) ? NULL : StringValuePtr(__v_uri) );

#line 80 "/home/geoff/Projects/gtk-webkit-ruby/ext/webkit/webkit.cr"
  webkit_web_view_load_uri(_self, uri);
 
  return Qnil;
}

static VALUE
WebView_main_frame(VALUE self)
{
  VALUE __p_retval = Qnil;
  WebKitWebView *_self = ((WebKitWebView*)RVAL2GOBJ(self));

#line 83 "/home/geoff/Projects/gtk-webkit-ruby/ext/webkit/webkit.cr"
  do { __p_retval = GOBJ2RVAL(webkit_web_view_get_main_frame(_self)); goto out; } while(0);
out:
  return __p_retval;
}

static VALUE
WebView_focused_frame(VALUE self)
{
  VALUE __p_retval = Qnil;
  WebKitWebView *_self = ((WebKitWebView*)RVAL2GOBJ(self));

#line 86 "/home/geoff/Projects/gtk-webkit-ruby/ext/webkit/webkit.cr"
  do { __p_retval = GOBJ2RVAL(webkit_web_view_get_focused_frame(_self)); goto out; } while(0);
out:
  return __p_retval;
}

static VALUE
WebView_progress(VALUE self)
{
  VALUE __p_retval = Qnil;
  WebKitWebView *_self = ((WebKitWebView*)RVAL2GOBJ(self));

#line 89 "/home/geoff/Projects/gtk-webkit-ruby/ext/webkit/webkit.cr"
  do { __p_retval =  rb_float_new(webkit_web_view_get_progress(_self)); goto out; } while(0);
out:
  return __p_retval;
}

static VALUE
WebView_title(VALUE self)
{
  VALUE __p_retval = Qnil;
  WebKitWebView *_self = ((WebKitWebView*)RVAL2GOBJ(self));

#line 92 "/home/geoff/Projects/gtk-webkit-ruby/ext/webkit/webkit.cr"
  do { __p_retval =  rb_str_new2(webkit_web_view_get_title(_self)); goto out; } while(0);
out:
  return __p_retval;
}

static VALUE
WebView_uri(VALUE self)
{
  VALUE __p_retval = Qnil;
  WebKitWebView *_self = ((WebKitWebView*)RVAL2GOBJ(self));

#line 95 "/home/geoff/Projects/gtk-webkit-ruby/ext/webkit/webkit.cr"
  do { __p_retval =  rb_str_new2(webkit_web_view_get_uri(_self)); goto out; } while(0);
out:
  return __p_retval;
}

static VALUE
WebView_reload(VALUE self)
{
  WebKitWebView *_self = ((WebKitWebView*)RVAL2GOBJ(self));

#line 98 "/home/geoff/Projects/gtk-webkit-ruby/ext/webkit/webkit.cr"
  webkit_web_view_reload(_self);
 
  return Qnil;
}

static VALUE
WebView_reload_bypass_cache(VALUE self)
{
  WebKitWebView *_self = ((WebKitWebView*)RVAL2GOBJ(self));

#line 101 "/home/geoff/Projects/gtk-webkit-ruby/ext/webkit/webkit.cr"
  webkit_web_view_reload_bypass_cache(_self);
 
  return Qnil;
}

static VALUE
WebView_set_custom_encoding(VALUE self, VALUE __v_encoding)
{
  char * encoding; char * __orig_encoding;
  WebKitWebView *_self = ((WebKitWebView*)RVAL2GOBJ(self));
  __orig_encoding = encoding = ( NIL_P(__v_encoding) ? NULL : StringValuePtr(__v_encoding) );

#line 104 "/home/geoff/Projects/gtk-webkit-ruby/ext/webkit/webkit.cr"
  webkit_web_view_set_custom_encoding(_self, encoding);
 
  return self;
}

static VALUE
WebView_stop_loading(VALUE self)
{
  WebKitWebView *_self = ((WebKitWebView*)RVAL2GOBJ(self));

#line 107 "/home/geoff/Projects/gtk-webkit-ruby/ext/webkit/webkit.cr"
  webkit_web_view_stop_loading(_self);
 
  return Qnil;
}

static VALUE
WebResource_encoding(VALUE self)
{
  VALUE __p_retval = Qnil;
  WebKitWebResource *_self = ((WebKitWebResource*)RVAL2GOBJ(self));

#line 114 "/home/geoff/Projects/gtk-webkit-ruby/ext/webkit/webkit.cr"
  do { __p_retval = strOrNil(webkit_web_resource_get_encoding(_self)); goto out; } while(0);
out:
  return __p_retval;
}

static VALUE
WebResource_frame_name(VALUE self)
{
  VALUE __p_retval = Qnil;
  WebKitWebResource *_self = ((WebKitWebResource*)RVAL2GOBJ(self));

#line 117 "/home/geoff/Projects/gtk-webkit-ruby/ext/webkit/webkit.cr"
  do { __p_retval = strOrNil(webkit_web_resource_get_frame_name(_self)); goto out; } while(0);
out:
  return __p_retval;
}

static VALUE
WebResource_mime_type(VALUE self)
{
  VALUE __p_retval = Qnil;
  WebKitWebResource *_self = ((WebKitWebResource*)RVAL2GOBJ(self));

#line 120 "/home/geoff/Projects/gtk-webkit-ruby/ext/webkit/webkit.cr"
  do { __p_retval = strOrNil(webkit_web_resource_get_mime_type(_self)); goto out; } while(0);
out:
  return __p_retval;
}

static VALUE
WebResource_uri(VALUE self)
{
  VALUE __p_retval = Qnil;
  WebKitWebResource *_self = ((WebKitWebResource*)RVAL2GOBJ(self));

#line 123 "/home/geoff/Projects/gtk-webkit-ruby/ext/webkit/webkit.cr"
  do { __p_retval = strOrNil(webkit_web_resource_get_mime_type(_self)); goto out; } while(0);
out:
  return __p_retval;
}

static VALUE
WebResource_data(VALUE self)
{
  VALUE __p_retval = Qnil;
  WebKitWebResource *_self = ((WebKitWebResource*)RVAL2GOBJ(self));

#line 126 "/home/geoff/Projects/gtk-webkit-ruby/ext/webkit/webkit.cr"

  do {
  GString * data  =
 webkit_web_resource_get_data(_self);
  do { __p_retval = rb_str_new(data->str, data->len); goto out; } while(0);

  } while(0);

out:
  return __p_retval;
}

static VALUE
WebResource_data_equals(VALUE self, VALUE data)
{
  VALUE __p_retval = data;
  WebKitWebResource *_self = ((WebKitWebResource*)RVAL2GOBJ(self));
  Check_Type(data, T_STRING);

#line 130 "/home/geoff/Projects/gtk-webkit-ruby/ext/webkit/webkit.cr"

  do {
  GString * odata  =
 webkit_web_resource_get_data(_self);
  g_free(odata->str);
  odata->str = g_malloc(RSTRING_LEN(data)+1);
  odata->len = odata->allocated_len = RSTRING_LEN(data);
  memcpy(RSTRING_PTR(data), odata->str, RSTRING_LEN(data));
  do { __p_retval = data; goto out; } while(0);

  } while(0);

out:
  return __p_retval;
}

static VALUE
WebNetworkRequest_uri(VALUE self)
{
  VALUE __p_retval = Qnil;
  WebKitNetworkRequest *_self = ((WebKitNetworkRequest*)RVAL2GOBJ(self));

#line 142 "/home/geoff/Projects/gtk-webkit-ruby/ext/webkit/webkit.cr"
  do { __p_retval =  rb_str_new2(webkit_network_request_get_uri(_self)); goto out; } while(0);
out:
  return __p_retval;
}

static VALUE
WebNetworkRequest_uri_equals(VALUE self, VALUE __v_uri)
{
  char * uri; char * __orig_uri;
  WebKitNetworkRequest *_self = ((WebKitNetworkRequest*)RVAL2GOBJ(self));
  __orig_uri = uri = ( NIL_P(__v_uri) ? NULL : StringValuePtr(__v_uri) );

#line 145 "/home/geoff/Projects/gtk-webkit-ruby/ext/webkit/webkit.cr"
  webkit_network_request_set_uri(_self, uri);
 
  return __v_uri;
}

static VALUE
WebNetworkResponse_uri(VALUE self)
{
  VALUE __p_retval = Qnil;
  WebKitNetworkResponse *_self = ((WebKitNetworkResponse*)RVAL2GOBJ(self));

#line 152 "/home/geoff/Projects/gtk-webkit-ruby/ext/webkit/webkit.cr"
  do { __p_retval =  rb_str_new2(webkit_network_response_get_uri(_self)); goto out; } while(0);
out:
  return __p_retval;
}

static VALUE
WebNetworkResponse_uri_equals(VALUE self, VALUE __v_uri)
{
  char * uri; char * __orig_uri;
  WebKitNetworkResponse *_self = ((WebKitNetworkResponse*)RVAL2GOBJ(self));
  __orig_uri = uri = ( NIL_P(__v_uri) ? NULL : StringValuePtr(__v_uri) );

#line 155 "/home/geoff/Projects/gtk-webkit-ruby/ext/webkit/webkit.cr"
  webkit_network_response_set_uri(_self, uri);
 
  return __v_uri;
}

static void __gcpool_RubyFunc_add(VALUE val)
    {
      if (_gcpool_RubyFunc == Qnil)
      {
        _gcpool_RubyFunc = rb_ary_new3(1, val);
      }
      else
      {
        rb_ary_push(_gcpool_RubyFunc, val);
      }
    }
    
    static void __gcpool_RubyFunc_del(VALUE val)
    {
      if (_gcpool_RubyFunc == Qnil)
      {
        rb_warn("Trying to remove object from empty GC queue RubyFunc");
        return;
      }
      rb_ary_delete(_gcpool_RubyFunc, val);
      // If nothing is referenced, don't keep an empty array in the pool...
      if (RARRAY(_gcpool_RubyFunc)->len == 0)
        _gcpool_RubyFunc = Qnil;
    }
    
/* Init */
void
Init_webkit(void)
{
  mWebKit = rb_define_module("WebKit");
  rb_define_singleton_method(mWebKit, "set_web_database_path", WebKit_CLASS_set_web_database_path, 1);
  rb_define_singleton_method(mWebKit, "remove_all_web_databases", WebKit_CLASS_remove_all_web_databases, 0);
  rb_define_singleton_method(mWebKit, "set_default_web_database_quota", WebKit_CLASS_set_default_web_database_quota, 1);
  cJavascriptError = rb_define_class_under(mWebKit, "JavascriptError", rb_eStandardError);
  cWebSettings = G_DEF_CLASS(WEBKIT_TYPE_WEB_SETTINGS, "WebSettings", mWebKit);
  rb_define_method(cWebSettings, "initialize", WebSettings_initialize, 0);
  cWebFrame = G_DEF_CLASS(WEBKIT_TYPE_WEB_FRAME, "WebFrame", mWebKit);
  rb_define_method(cWebFrame, "exec_js", WebFrame_exec_js, 1);
  rb_define_method(cWebFrame, "add_ruby_eval", WebFrame_add_ruby_eval, 0);
  rb_define_method(cWebFrame, "add_js_api", WebFrame_add_js_api, 1);
  rb_define_method(cWebFrame, "load_string", WebFrame_load_string, 4);
  cWebView = G_DEF_CLASS(WEBKIT_TYPE_WEB_VIEW, "WebView", mWebKit);
  rb_define_method(cWebView, "initialize", WebView_initialize, 0);
  rb_define_method(cWebView, "open", WebView_open, 1);
  rb_define_method(cWebView, "execute_script", WebView_execute_script, 1);
  rb_define_method(cWebView, "set_settings", WebView_set_settings, 1);
  rb_define_method(cWebView, "load_string", WebView_load_string, 4);
  rb_define_method(cWebView, "load_uri", WebView_load_uri, 1);
  rb_define_method(cWebView, "main_frame", WebView_main_frame, 0);
  rb_define_method(cWebView, "focused_frame", WebView_focused_frame, 0);
  rb_define_method(cWebView, "progress", WebView_progress, 0);
  rb_define_method(cWebView, "title", WebView_title, 0);
  rb_define_method(cWebView, "uri", WebView_uri, 0);
  rb_define_method(cWebView, "reload", WebView_reload, 0);
  rb_define_method(cWebView, "reload_bypass_cache", WebView_reload_bypass_cache, 0);
  rb_define_method(cWebView, "set_custom_encoding", WebView_set_custom_encoding, 1);
  rb_define_method(cWebView, "stop_loading", WebView_stop_loading, 0);
  cWebResource = G_DEF_CLASS(WEBKIT_TYPE_WEB_RESOURCE, "WebResource", mWebKit);
  rb_define_method(cWebResource, "encoding", WebResource_encoding, 0);
  rb_define_method(cWebResource, "frame_name", WebResource_frame_name, 0);
  rb_define_method(cWebResource, "mime_type", WebResource_mime_type, 0);
  rb_define_method(cWebResource, "uri", WebResource_uri, 0);
  rb_define_method(cWebResource, "data", WebResource_data, 0);
  rb_define_method(cWebResource, "data=", WebResource_data_equals, 1);
  cWebNetworkRequest = G_DEF_CLASS(WEBKIT_TYPE_NETWORK_REQUEST, "WebNetworkRequest", mWebKit);
  rb_define_method(cWebNetworkRequest, "uri", WebNetworkRequest_uri, 0);
  rb_define_method(cWebNetworkRequest, "uri=", WebNetworkRequest_uri_equals, 1);
  cWebNetworkResponse = G_DEF_CLASS(WEBKIT_TYPE_NETWORK_RESPONSE, "WebNetworkResponse", mWebKit);
  rb_define_method(cWebNetworkResponse, "uri", WebNetworkResponse_uri, 0);
  rb_define_method(cWebNetworkResponse, "uri=", WebNetworkResponse_uri_equals, 1);
rb_gc_register_address(&_gcpool_RubyFunc);
}
