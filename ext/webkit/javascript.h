#include <alloca.h>
#include "javascript-type-convert.h"

#define STRINGIZE(s) #s

#define JS_fn(f) _JS_fn(ctx, STRINGIZE(f), f)

JSObjectRef _JS_fn(JSContextRef ctx, const char *name, JSObjectCallAsFunctionCallback f)
{
    JSStringRef nameStr = JSStringCreateWithUTF8CString(name);
    JSObjectRef value = JSObjectMakeFunctionWithCallback(ctx, nameStr, f);
    JSStringRelease(nameStr);
    return value;
}

static inline bool
js_obj_set_value(JSContextRef ctx, JSObjectRef object, char *name, JSObjectRef value)
{
	JSValueRef *_exception = NULL;
	JSStringRef nameStr    = JSStringCreateWithUTF8CString(name);

    JSObjectSetProperty(ctx, object, nameStr, value, kJSPropertyAttributeNone, _exception);

    JSStringRelease(nameStr);

	if (*_exception) { return false; }

	return true;
}


static JSValueRef
js_call_ruby_eval (JSContextRef ctx, JSObjectRef function, JSObjectRef thisObject,
		size_t argumentCount, const JSValueRef arguments[], JSValueRef* exception)
{
	JSValueRef value = NULL;

	if (argumentCount == 1) {
		char *script;
		VALUE val;

		script = convert_javascript_to_utf8_string(ctx, arguments[0], NULL);
		val    = rb_eval_string(script);

		value  = convert_ruby_to_javascript(ctx, val);

		free(script);
	} else {
		*exception = convert_string_to_jsval(ctx, "Wrong number of arguments");
	}

	return value;
}

static GHashTable *ruby_fns = NULL;

static JSValueRef
js_ruby_fn(JSContextRef ctx, JSObjectRef function, JSObjectRef thisObject,
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
		args[i] = convert_javascript_to_ruby(ctx, arguments[i]);
	}

	retval = convert_ruby_to_javascript(ctx,
				rb_funcall2(ruby_fn, rb_intern("call"), argumentCount, args));

	xfree(args);

	return retval;
}

/**
 * Javascript functions called directly from binding
 **/
static void *
javascript_add_ruby_fn(JSGlobalContextRef ctx, char *name, VALUE ruby_fn)
{
	JSObjectRef fn = NULL;

	if (!ruby_fns) {
		ruby_fns = g_hash_table_new(NULL, NULL);
	}

	JSObjectRef global = JSContextGetGlobalObject(ctx);

	fn = JS_fn(js_ruby_fn);

	if (!JSObjectSetPrivate(fn, (void*)ruby_fn)) {
		gpointer old_value = g_hash_table_lookup(ruby_fns, (gpointer)fn);
		if (old_value != NULL)
			RUBYFUNC_DEL((VALUE)old_value);
		RUBYFUNC_ADD(ruby_fn);
		g_hash_table_insert(ruby_fns, (gpointer)fn, (gpointer)ruby_fn);
	}

	js_obj_set_value(ctx, global, name, fn); // Check failure?

	return NULL;
}

static void *
javascript_add_ruby_eval(JSGlobalContextRef ctx)
{
	JSObjectRef global = JSContextGetGlobalObject(ctx);

	js_obj_set_value(ctx, global, "ruby_eval", JS_fn(js_call_ruby_eval));

	return NULL;
}

static VALUE
javascript_exec(JSGlobalContextRef ctx, char *script)
{
	VALUE output          = Qnil;

	JSStringRef js_script = JSStringCreateWithUTF8CString(script);
	JSStringRef url       = JSStringCreateWithUTF8CString("x-ruby://string");

	JSValueRef err        = JSValueMakeUndefined(ctx);
	JSValueRef retval     = NULL;


	retval = JSEvaluateScript(ctx, js_script, NULL, url, 0, &err);

	JSStringRelease(url);
	JSStringRelease(js_script);

	/* Convert output to ruby */
	if (retval) {
		output = convert_javascript_to_ruby(ctx, retval);
	} else {
		JSStringRef str = JSValueToStringCopy(ctx, err, NULL);
		size_t len = 0, max = 0;
		char *buf;

		max = JSStringGetMaximumUTF8CStringSize(str);
		buf = alloca(max);
		len = JSStringGetUTF8CString(str, buf, max);

		JSStringRelease(str);
		rb_raise(cJavascriptError, buf);
		//free(buf); // Will this ever be called?
	}

	return output;
}


