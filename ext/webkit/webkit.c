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
WebFrame_setup_ruby(VALUE self);
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

/* Inline C code */

#include <intern.h>

#define STRINGIZE(s) #s

#define JS_fn(f) _JS_fn(ctx, STRINGIZE(f), f)
JSObjectRef _JS_fn(JSContextRef ctx, const char *name, JSObjectCallAsFunctionCallback f)
{
    JSStringRef nameStr = JSStringCreateWithUTF8CString(name);
    JSObjectRef value = JSObjectMakeFunctionWithCallback(ctx, nameStr, f);
    JSStringRelease(nameStr);
    return value;
}

#define SET_VALUE(object, name, value) \
    {JSStringRef nameStr = JSStringCreateWithUTF8CString(name); \
    JSObjectSetProperty(ctx, object, nameStr, value, kJSPropertyAttributeNone, _exception); \
    JSStringRelease(nameStr); \
    if (*_exception) { return NULL; }}

static char *js2utf8(JSContextRef ctx, JSValueRef val, size_t* out_len) {
	JSStringRef str = JSValueToStringCopy(ctx, val, NULL);
	size_t max = 0, len;
	char *buf;

	max = JSStringGetMaximumUTF8CStringSize(str);
	buf = malloc(max);
	len = JSStringGetUTF8CString(str, buf, max);

	if (out_len) {
		*out_len = len;
	}

	JSStringRelease(str);

	return buf;
}

static VALUE js2rb(JSContextRef ctx, JSValueRef val) {
	VALUE output = Qnil;
	JSStringRef str = JSValueToStringCopy(ctx, val, NULL);

	size_t len = 0;
	size_t max = 0;
	char *buf;

	max = JSStringGetMaximumUTF8CStringSize(str);
	buf = malloc(max);
	len = JSStringGetUTF8CString(str, buf, max);

	output = rb_str_new(buf, len);
	free(buf);

	JSStringRelease(str);
	return output;
}

static JSValueRef str2jsval(JSContextRef ctx, char *str) {
	JSStringRef js_s_ref  = JSStringCreateWithUTF8CString(str);
	JSValueRef js_val_ref = JSValueMakeString(ctx, js_s_ref);
	JSStringRelease(js_s_ref);
	return js_val_ref;
}

static JSValueRef rb2js(JSContextRef ctx, VALUE val) {
	
	if (TYPE(val) != T_STRING) {
		val = rb_funcall(val, rb_intern("to_s"), 0);
	}

	{
		JSStringRef js_s_ref  = JSStringCreateWithUTF8CString(RSTRING_PTR(val));
		JSValueRef js_val_ref = JSValueMakeString(ctx, js_s_ref);
		JSStringRelease(js_s_ref);
		return js_val_ref;
	}
}

static JSValueRef evaluate_ruby (JSContextRef ctx, JSObjectRef function, JSObjectRef thisObject, size_t argumentCount, const JSValueRef arguments[], JSValueRef* exception)
{
	JSValueRef value = NULL;

	if (argumentCount == 1) {
		char *script = js2utf8(ctx, arguments[0], NULL);
		VALUE val    = rb_eval_string(script);

		value = rb2js(ctx, val);

		free(script);
	} else {
		*exception = str2jsval(ctx, "Wrong number of arguments");
	}

	return value;
}

static GHashTable *ruby_fns = NULL;

static JSValueRef js_ruby_fn(JSContextRef ctx, JSObjectRef function, JSObjectRef thisObject, 
				size_t argumentCount, const JSValueRef arguments[], JSValueRef* exception)
{
	size_t i;
	VALUE *args = ALLOC_N(VALUE, argumentCount);
	VALUE ruby_fn = (VALUE) JSObjectGetPrivate(function);
	JSValueRef retval;

	if ((!ruby_fn) && ruby_fns) {
		ruby_fn = (VALUE)g_hash_table_lookup(ruby_fns, (gpointer)function);
	}
	
	//printf("Hmm.  Found: %p\n", function);

	for (i = 0; i < argumentCount; i++) {
		args[i] = js2rb(ctx, arguments[i]);
	}

	retval = rb2js(ctx, rb_funcall2(ruby_fn, rb_intern("call"), argumentCount, args));

	free(args);

	return retval;
}

static void *javascript_add_ruby_fn(JSGlobalContextRef ctx, char *name, VALUE ruby_fn)
{
	JSValueRef *_exception = NULL;
	JSObjectRef fn = NULL;
	
	if (!ruby_fns) {
		ruby_fns = g_hash_table_new(NULL, NULL);
	}

	JSObjectRef global = JSContextGetGlobalObject(ctx);

	fn = JS_fn(js_ruby_fn);

	//printf("Hmm. Made: %p\n", fn);

	if (!JSObjectSetPrivate(fn, (void*)ruby_fn)) {
		g_hash_table_insert(ruby_fns, (gpointer)fn, (gpointer)ruby_fn);
		//printf("Fail to set fn!! :(\n");
	}

	SET_VALUE(global, name, fn);

	return NULL;
}

static void *javascript_setup_ruby(JSGlobalContextRef ctx)
{
	JSValueRef *_exception = NULL;

	JSObjectRef global = JSContextGetGlobalObject(ctx);

	SET_VALUE(global, "ruby_eval", JS_fn(evaluate_ruby));

	return NULL;
}

VALUE javascript_exec(JSGlobalContextRef ctx, char *script) 
{
	VALUE output = Qnil;

	JSStringRef js_script = JSStringCreateWithUTF8CString(script);

	JSStringRef url = JSStringCreateWithUTF8CString("ruby:string");
	JSValueRef err = JSValueMakeUndefined(ctx);
	JSValueRef retval = NULL;

	/* Execute script */
	//printf("Executing: `%s'\n", script);
	
	retval = JSEvaluateScript(ctx, js_script, NULL, url, 0, &err);
	//printf("Retval: %p, err %p\n", retval, err);

	JSStringRelease(url);
	JSStringRelease(js_script);

	/* Convert output to string */
	if (retval) {
		output = js2rb(ctx, retval);
	} else {
		JSStringRef str = JSValueToStringCopy(ctx, err, NULL);
		size_t len = 0;
		size_t max = 0;
		char *buf;

		max = JSStringGetMaximumUTF8CStringSize(str);
		buf = malloc(max);
		len = JSStringGetUTF8CString(str, buf, max);
		fprintf(stderr, "Exception: %s\n", buf);
		JSStringRelease(str);
		rb_raise(cJavascriptError, buf);
		free(buf);
	}

	return output;
}


/* Code */
static VALUE
WebKit_CLASS_set_web_database_path(VALUE self, VALUE __v_path)
{
  char * path; char * __orig_path;
  __orig_path = path = ( NIL_P(__v_path) ? NULL : StringValuePtr(__v_path) );

#line 213 "/home/geoff/Projects/gtk-webkit-ruby/ext/webkit/webkit.cr"
  webkit_set_web_database_directory_path(path);
 
  return self;
}

static VALUE
WebKit_CLASS_remove_all_web_databases(VALUE self)
{

#line 216 "/home/geoff/Projects/gtk-webkit-ruby/ext/webkit/webkit.cr"
  webkit_remove_all_web_databases();
 
  return Qnil;
}

static VALUE
WebKit_CLASS_set_default_web_database_quota(VALUE self, VALUE __v_quota)
{
  guint64 quota; guint64 __orig_quota;
  __orig_quota = quota = rb_num2ull(__v_quota);

#line 219 "/home/geoff/Projects/gtk-webkit-ruby/ext/webkit/webkit.cr"
  webkit_set_default_web_database_quota(quota);
 
  return self;
}

static VALUE
WebSettings_initialize(VALUE self)
{

#line 225 "/home/geoff/Projects/gtk-webkit-ruby/ext/webkit/webkit.cr"
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

#line 231 "/home/geoff/Projects/gtk-webkit-ruby/ext/webkit/webkit.cr"
  do { __p_retval = javascript_exec(webkit_web_frame_get_global_context(_self), js); goto out; } while(0);
out:
  return __p_retval;
}

static VALUE
WebFrame_setup_ruby(VALUE self)
{
  WebKitWebFrame *_self = ((WebKitWebFrame*)RVAL2GOBJ(self));

#line 234 "/home/geoff/Projects/gtk-webkit-ruby/ext/webkit/webkit.cr"
  javascript_setup_ruby(webkit_web_frame_get_global_context(_self));
 
  return Qnil;
}

static VALUE
WebFrame_add_js_api(VALUE self, VALUE __v_name)
{
  char * name; char * __orig_name;
  WebKitWebFrame *_self = ((WebKitWebFrame*)RVAL2GOBJ(self));
  __orig_name = name = ( NIL_P(__v_name) ? NULL : StringValuePtr(__v_name) );
  VALUE block = rb_block_proc();

#line 237 "/home/geoff/Projects/gtk-webkit-ruby/ext/webkit/webkit.cr"
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

#line 240 "/home/geoff/Projects/gtk-webkit-ruby/ext/webkit/webkit.cr"
  webkit_web_frame_load_string(_self, content, mime_type, encoding, base_uri);
 
  return Qnil;
}

static VALUE
WebView_initialize(VALUE self)
{

#line 246 "/home/geoff/Projects/gtk-webkit-ruby/ext/webkit/webkit.cr"
  RBGTK_INITIALIZE(self, webkit_web_view_new());
 
  return Qnil;
}

static VALUE
WebView_open(VALUE self, VALUE __v_uri)
{
  char * uri; char * __orig_uri;
  WebKitWebView *_self = ((WebKitWebView*)RVAL2GOBJ(self));
  __orig_uri = uri = ( NIL_P(__v_uri) ? NULL : StringValuePtr(__v_uri) );

#line 249 "/home/geoff/Projects/gtk-webkit-ruby/ext/webkit/webkit.cr"
  webkit_web_view_open(_self, uri);
 
  return Qnil;
}

static VALUE
WebView_execute_script(VALUE self, VALUE __v_script)
{
  char * script; char * __orig_script;
  WebKitWebView *_self = ((WebKitWebView*)RVAL2GOBJ(self));
  __orig_script = script = ( NIL_P(__v_script) ? NULL : StringValuePtr(__v_script) );

#line 252 "/home/geoff/Projects/gtk-webkit-ruby/ext/webkit/webkit.cr"
  webkit_web_view_execute_script(_self, script);
 
  return Qnil;
}

static VALUE
WebView_set_settings(VALUE self, VALUE __v_settings)
{
  WebKitWebSettings * settings; WebKitWebSettings * __orig_settings;
  WebKitWebView *_self = ((WebKitWebView*)RVAL2GOBJ(self));
  __orig_settings = settings = RVAL2GOBJ(__v_settings);

#line 255 "/home/geoff/Projects/gtk-webkit-ruby/ext/webkit/webkit.cr"
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

#line 258 "/home/geoff/Projects/gtk-webkit-ruby/ext/webkit/webkit.cr"
  webkit_web_view_load_string(_self, content, mime_type, encoding, base_uri);
 
  return Qnil;
}

static VALUE
WebView_load_uri(VALUE self, VALUE __v_uri)
{
  char * uri; char * __orig_uri;
  WebKitWebView *_self = ((WebKitWebView*)RVAL2GOBJ(self));
  __orig_uri = uri = ( NIL_P(__v_uri) ? NULL : StringValuePtr(__v_uri) );

#line 261 "/home/geoff/Projects/gtk-webkit-ruby/ext/webkit/webkit.cr"
  webkit_web_view_load_uri(_self, uri);
 
  return Qnil;
}

static VALUE
WebView_main_frame(VALUE self)
{
  VALUE __p_retval = Qnil;
  WebKitWebView *_self = ((WebKitWebView*)RVAL2GOBJ(self));

#line 264 "/home/geoff/Projects/gtk-webkit-ruby/ext/webkit/webkit.cr"
  do { __p_retval = GOBJ2RVAL(webkit_web_view_get_main_frame(_self)); goto out; } while(0);
out:
  return __p_retval;
}

static VALUE
WebView_focused_frame(VALUE self)
{
  VALUE __p_retval = Qnil;
  WebKitWebView *_self = ((WebKitWebView*)RVAL2GOBJ(self));

#line 267 "/home/geoff/Projects/gtk-webkit-ruby/ext/webkit/webkit.cr"
  do { __p_retval = GOBJ2RVAL(webkit_web_view_get_focused_frame(_self)); goto out; } while(0);
out:
  return __p_retval;
}

static VALUE
WebView_progress(VALUE self)
{
  VALUE __p_retval = Qnil;
  WebKitWebView *_self = ((WebKitWebView*)RVAL2GOBJ(self));

#line 272 "/home/geoff/Projects/gtk-webkit-ruby/ext/webkit/webkit.cr"
  do { __p_retval =  rb_float_new(webkit_web_view_get_progress(_self)); goto out; } while(0);
out:
  return __p_retval;
}

static VALUE
WebView_title(VALUE self)
{
  VALUE __p_retval = Qnil;
  WebKitWebView *_self = ((WebKitWebView*)RVAL2GOBJ(self));

#line 275 "/home/geoff/Projects/gtk-webkit-ruby/ext/webkit/webkit.cr"
  do { __p_retval =  rb_str_new2(webkit_web_view_get_title(_self)); goto out; } while(0);
out:
  return __p_retval;
}

static VALUE
WebView_uri(VALUE self)
{
  VALUE __p_retval = Qnil;
  WebKitWebView *_self = ((WebKitWebView*)RVAL2GOBJ(self));

#line 278 "/home/geoff/Projects/gtk-webkit-ruby/ext/webkit/webkit.cr"
  do { __p_retval =  rb_str_new2(webkit_web_view_get_uri(_self)); goto out; } while(0);
out:
  return __p_retval;
}

static VALUE
WebView_reload(VALUE self)
{
  WebKitWebView *_self = ((WebKitWebView*)RVAL2GOBJ(self));

#line 281 "/home/geoff/Projects/gtk-webkit-ruby/ext/webkit/webkit.cr"
  webkit_web_view_reload(_self);
 
  return Qnil;
}

static VALUE
WebView_reload_bypass_cache(VALUE self)
{
  WebKitWebView *_self = ((WebKitWebView*)RVAL2GOBJ(self));

#line 284 "/home/geoff/Projects/gtk-webkit-ruby/ext/webkit/webkit.cr"
  webkit_web_view_reload_bypass_cache(_self);
 
  return Qnil;
}

static VALUE
WebView_set_custom_encoding(VALUE self, VALUE __v_encoding)
{
  char * encoding; char * __orig_encoding;
  WebKitWebView *_self = ((WebKitWebView*)RVAL2GOBJ(self));
  __orig_encoding = encoding = ( NIL_P(__v_encoding) ? NULL : StringValuePtr(__v_encoding) );

#line 287 "/home/geoff/Projects/gtk-webkit-ruby/ext/webkit/webkit.cr"
  webkit_web_view_set_custom_encoding(_self, encoding);
 
  return self;
}

static VALUE
WebView_stop_loading(VALUE self)
{
  WebKitWebView *_self = ((WebKitWebView*)RVAL2GOBJ(self));

#line 290 "/home/geoff/Projects/gtk-webkit-ruby/ext/webkit/webkit.cr"
  webkit_web_view_stop_loading(_self);
 
  return Qnil;
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
  rb_define_method(cWebFrame, "setup_ruby", WebFrame_setup_ruby, 0);
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
}
