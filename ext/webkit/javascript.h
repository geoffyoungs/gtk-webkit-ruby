#include <alloca.h>

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
convert_string_to_jsval(JSContextRef ctx, char *str) {
	JSStringRef js_s_ref;
	JSValueRef js_val_ref;

	js_s_ref  = JSStringCreateWithUTF8CString(str);
	js_val_ref = JSValueMakeString(ctx, js_s_ref);
	JSStringRelease(js_s_ref);

	return js_val_ref;
}

static char *
convert_javascript_to_utf8_string(JSContextRef ctx, JSValueRef val, size_t* out_len)
{
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

static inline VALUE
convert_javascript_string_to_ruby(JSContextRef ctx, JSValueRef val)
{
	VALUE       output = Qnil;
	JSStringRef str;
	size_t      len = 0, max = 0;
	char 		*buf;

	str = JSValueToStringCopy(ctx, val, NULL);
	max = JSStringGetMaximumUTF8CStringSize(str);
	buf = malloc(max);
	len = JSStringGetUTF8CString(str, buf, max);

	output = rb_str_new(buf, len-1); // Ignore terminator

	free(buf);
	JSStringRelease(str);

	return output;
}

static VALUE
convert_javascript_to_ruby(JSContextRef ctx, JSValueRef val)
{
	VALUE output = Qnil;
	JSValueRef *_exception = NULL;

	switch (JSValueGetType(ctx, val)) {
		case kJSTypeUndefined:
		case kJSTypeNull:
			output = Qnil;
			break;
		case kJSTypeBoolean:
			output = JSValueToBoolean(ctx, val) ? Qtrue : Qfalse;
			break;
		case kJSTypeNumber:
	    	output = rb_float_new(JSValueToNumber(ctx, val, _exception));
			break;
        case kJSTypeString:
			output = convert_javascript_string_to_ruby(ctx, val);
			break;
		case kJSTypeObject:
			output = convert_javascript_string_to_ruby(ctx, val);
			break;
	}

	return output;
}


static JSValueRef
convert_ruby_to_javascript(JSContextRef ctx, VALUE value)
{
	JSStringRef str;
	JSValueRef jsval;

	switch (TYPE(value)) {
		case T_FIXNUM:
			return JSValueMakeNumber(ctx, (double) FIX2LONG(value));
		case T_FLOAT:
			return JSValueMakeNumber(ctx, (double) NUM2DBL(value));
		case T_BIGNUM:
			return JSValueMakeNumber(ctx, (double) rb_big2long(value));
		case T_TRUE:
		case T_FALSE:
			return JSValueMakeBoolean(ctx, RTEST(value));
		case T_UNDEF:
			return JSValueMakeUndefined(ctx);
		case T_NIL:
			return JSValueMakeNull(ctx);
		default:
			if (TYPE(value) != T_STRING) {
				value = rb_funcall(value, rb_intern("to_s"), 0);
			}
			str   = JSStringCreateWithUTF8CString(RSTRING_PTR(value));
			jsval = JSValueMakeString(ctx, str);
			JSStringRelease(str);
			return jsval;
	}
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


