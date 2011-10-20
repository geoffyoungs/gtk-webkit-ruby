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

