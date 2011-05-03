%pkg-config gtk+-2.0
%pkg-config webkit-1.0
%name webkit

%{
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

	output = rb_str_new(buf, len-1); // Ignore terminator
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

static JSValueRef evaluate_ruby (JSContextRef ctx, JSObjectRef function, JSObjectRef thisObject, 
		size_t argumentCount, const JSValueRef arguments[], JSValueRef* exception)
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

%}
 
%include webkit/webkit.h
%include JavaScriptCore/JavaScript.h

module WebKit
	class JavascriptError < StandardError
	end

	def self.set_web_database_path(char *path)
		webkit_set_web_database_directory_path(path);
	end
	def self.remove_all_web_databases()
		webkit_remove_all_web_databases();
	end
	def self.set_default_web_database_quota(guint64 quota)
		webkit_set_default_web_database_quota(quota);
	end

	gobject WebSettings < WEBKIT_TYPE_WEB_SETTINGS
		@type WebKitWebSettings
		def initialize()
			G_INITIALIZE(self, webkit_web_settings_new());
		end
	end
	gobject WebFrame < WEBKIT_TYPE_WEB_FRAME
		@type WebKitWebFrame
		def exec_js(char *js)
			return javascript_exec(webkit_web_frame_get_global_context(_self), js);
		end
		def setup_ruby()
			javascript_setup_ruby(webkit_web_frame_get_global_context(_self));
		end
		def add_js_api(char *name, &block)
			javascript_add_ruby_fn(webkit_web_frame_get_global_context(_self), name, block);
		end
		def load_string(char *content, char *mime_type, char *encoding, char *base_uri)
			 webkit_web_frame_load_string(_self, content, mime_type, encoding, base_uri);
		end
	end
	gobject WebView < WEBKIT_TYPE_WEB_VIEW
		@type WebKitWebView
		def initialize()
			RBGTK_INITIALIZE(self, webkit_web_view_new());
		end
		def open(char *uri)
			webkit_web_view_open(_self, uri);
		end
		def execute_script(char *script)
			webkit_web_view_execute_script(_self, script);
		end
		def set_settings(WebKitWebSettings *settings)
			webkit_web_view_set_settings(_self, settings);
		end
		def load_string(char *content, char *mime_type, char *encoding, char *base_uri)
			 webkit_web_view_load_string(_self, content, mime_type, encoding, base_uri);
		end
		def load_uri(char *uri)
			webkit_web_view_load_uri(_self, uri);
		end
		def WebKitWebFrame*:main_frame
			return webkit_web_view_get_main_frame(_self);
		end
		def WebKitWebFrame*:focused_frame
			return webkit_web_view_get_focused_frame(_self);
		end


		def double:progress
			return webkit_web_view_get_progress(_self);
		end
		def char*:title
			return webkit_web_view_get_title(_self);
		end
		def char*:uri
			return webkit_web_view_get_uri(_self);
		end
		def reload
			webkit_web_view_reload(_self);
		end
		def reload_bypass_cache
			webkit_web_view_reload_bypass_cache(_self);
		end
		def set_custom_encoding(char *encoding)
			webkit_web_view_set_custom_encoding(_self, encoding);
		end
		def stop_loading
			webkit_web_view_stop_loading(_self);
		end
	end
end

