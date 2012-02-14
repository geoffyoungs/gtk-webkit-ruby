#include <alloca.h>
#include "javascript-type-convert.h"

#define STRINGIZE(s) #s

#define JS_fn(f) _JS_fn(ctx, STRINGIZE(f), f)


typedef struct {
	JSClassDefinition definition;
	JSClassRef class_ref;
	VALUE rbObject;
	JSContextRef ctx;
} ruby_js_class_def;
static ruby_js_class_def *jsrb_newClass(VALUE rObject);

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

static void jsrb_class_init(JSContextRef ctx, JSObjectRef object)
{
	// Do something here?
	/*fprintf(stderr, "init: -> object is %p\n", object);
	fflush(stderr);*/
}

static void jsrb_class_final(JSObjectRef object) {
	// Clean up?
}

static bool jsrb_has_prop(JSContextRef ctx, JSObjectRef object, JSStringRef propertyName)
{
	VALUE robject = JSObjectGetPrivate(object);

	//rb_p(robject);
//	fprintf(stderr, "Has %s on %p?\n", jsstrref_to_charp(propertyName, NULL), robject);
//	fflush(stderr);

	if (RTEST(robject)) {
		ID prop = convert_javascript_to_intern(propertyName);
		if (rb_respond_to(robject, prop)) {
//			fprintf(stderr, "%p responds to %s\n", robject, jsstrref_to_charp(propertyName, NULL));
			return true;
		}
	}

	return false;
}

static JSObjectRef jsrb_get_prop(JSContextRef ctx, JSObjectRef object, JSStringRef propertyName, JSValueRef* exception)
{
	VALUE robject = JSObjectGetPrivate(object);

	//rb_p(robject);
//	fprintf(stderr, "Get %s on %p\n", jsstrref_to_charp(propertyName, NULL), robject);
//	fflush(stderr);

	if (RTEST(robject)) {
		ID prop = convert_javascript_to_intern(propertyName);
		if (rb_respond_to(robject, prop)) {
			VALUE rbo = rb_funcall(robject, rb_intern("method"), 1, conv_jsstrref_to_value(propertyName));
			ruby_js_class_def *def = jsrb_newClass(rbo);
			def->ctx = ctx;
			return JSObjectMake(def->ctx, def->class_ref, (void*)rbo);
		}
	}
	
	return false;
}

static bool jsrb_set_prop(JSContextRef ctx, JSObjectRef object, JSStringRef propertyName, JSValueRef value, JSValueRef* exception)
{
	return false;
}

static bool jsrb_del_prop(JSContextRef ctx, JSObjectRef object, JSStringRef propertyName, JSValueRef* exception)
{
	return false;
}

static void jsrb_get_prop_names(JSContextRef ctx, JSObjectRef object, JSPropertyNameAccumulatorRef propertyNames)
{
}

static JSValueRef jsrb_call_fn(JSContextRef ctx, JSObjectRef function, JSObjectRef thisObject, size_t argumentCount, const JSValueRef arguments[], JSValueRef* exception)
{
	// 
	VALUE robject = JSObjectGetPrivate(function);
	VALUE self    = JSObjectGetPrivate(thisObject);
	if (RTEST(robject)) {
		VALUE rbo = rb_funcall2(robject, rb_intern("call"), 0, NULL);
		JSValueRef retval = convert_ruby_to_javascript(ctx, rbo);
		return retval;
	}
	return JSValueMakeUndefined(ctx);
}

typedef struct {
	JSContextRef ctx;
	JSObjectRef constructor;
	size_t argumentCount;
	JSValueRef *arguments;
	JSValueRef* exception;
} construct_args;

static void jsrb_class_construct_ffi(ffi_cif* cif,
	void** resp,
	void** vargs,
	void* userdata) {

	ruby_js_class_def *def = (ruby_js_class_def*)userdata;
	construct_args *args = (construct_args*)vargs;
	
	{
		int no_args = 0;
		VALUE *arg_list = NULL;
		JSObjectRef object;
		VALUE value = Qnil;

		/*fprintf(stderr, "jsrb_class_construct_ffi -> rObject is %p, Def is %p\n", def->rbObject, def);
		fflush(stderr);*/

		if (is_ruby_native_thread()) {
			value = rb_class_new_instance(no_args, arg_list, def->rbObject);
			// Convert JS values...
			RUBYFUNC_ADD(value);
		}
		
		/*fprintf(stderr, "jsrb_class_construct_ffi -> ctx is %p\n", args->ctx);
		fflush(stderr);*/

		object = JSObjectMake(def->ctx, def->class_ref, (void*)value);

		/*fprintf(stderr, "jsrb_class_construct_ffi -> created %p\n", object);
		fflush(stderr);*/

		*resp = (void*)object;
	}
}


static JSObjectRef jsrb_call_create(JSContextRef ctx, JSObjectRef constructor, size_t argumentCount, const JSValueRef arguments[], JSValueRef* exception)
{
	return NULL;
}

static bool jsrb_has_instance(JSContextRef ctx, JSObjectRef constructor, JSValueRef possibleInstance, JSValueRef* exception)
{
	return false;
}

static JSValueRef jsrb_convert(JSContextRef ctx, JSObjectRef object, JSType type, JSValueRef* exception)
{
	return false;
}

static ruby_js_class_def *jsrb_newClass(VALUE rObject) {
	JSClassDefinition *jclass;
	ruby_js_class_def *def;

	def = ALLOC_N(ruby_js_class_def,1);
	def->definition = kJSClassDefinitionEmpty;
	jclass = &(def->definition);

	jclass->version    = 0;
	jclass->attributes = kJSClassAttributeNoAutomaticPrototype;

	// Check this?  It should probably be a valid JS name...

	// ignore
	// staticValues
	// staticFunctions

	/* Basics */
	jclass->initialize        = jsrb_class_init;

	jclass->finalize          = jsrb_class_final;

	/* Property accessors... */
	jclass->hasProperty       = jsrb_has_prop;
	jclass->getProperty       = jsrb_get_prop;
	jclass->setProperty       = jsrb_set_prop;
	jclass->deleteProperty    = jsrb_del_prop;
	jclass->getPropertyNames  = jsrb_get_prop_names;

	/* Function */
	//rb_p(rObject);
	if (rb_respond_to(rObject, rb_intern("call"))) {
//		fprintf(stderr, "On object %p -> responds to call()\n", rObject);
//		fflush(stderr);
		jclass->callAsFunction    = jsrb_call_fn;
	} else {
//		fprintf(stderr, "On object %p -> not a function :(\n", rObject);
//		fflush(stderr);

		jclass->callAsFunction    = NULL;
	}

	if (rb_obj_is_instance_of(rObject, rb_cClass)) {
		jclass->className     = rb_class2name(rObject);
		ffi_cif *cif;
		ffi_closure *closure;
		void *code;
		ffi_type **types;
		int args_len = 5;

		cif      = ALLOC_N(ffi_cif, 1);
		closure  = ffi_closure_alloc(sizeof(ffi_closure), &code);
		types	 = ALLOC_N(ffi_type*, args_len);

		types[0] = &ffi_type_pointer;
		types[1] = &ffi_type_pointer;
		types[2] = &ffi_type_uint32;
		types[3] = &ffi_type_pointer;
		types[4] = &ffi_type_pointer;

		if (ffi_prep_cif(cif, FFI_DEFAULT_ABI, args_len, &ffi_type_pointer, types) == FFI_OK) {
			if (ffi_prep_closure_loc(closure, cif, jsrb_class_construct_ffi, (void*)def, code) == FFI_OK) {
				jclass->callAsConstructor = closure;
			}
		}
	} else {
		jclass->className     = rb_class2name(rb_obj_class(rObject));
		jclass->callAsConstructor = NULL;
	}

	//jclass->callAsConstructor = jsrb_call_create;
	jclass->hasInstance       = jsrb_has_instance;
	jclass->convertToType     = jsrb_convert;

	def->rbObject   = rObject;
	def->class_ref = JSClassCreate(jclass);

	return def;
}

/*static void jsrb_create_class(VALUE rObject) {
	//JSClassDefinition *def = jsrb_newClass(rObject);
}*/


/**
 * Javascript functions called directly from binding
 **/
static void javascript_add_class(JSGlobalContextRef ctx, char *name, VALUE rObject)
{
	JSObjectRef global = JSContextGetGlobalObject(ctx), object;

	// Should we store this on the class?
	VALUE v = Qfalse;
	ruby_js_class_def *def;

	if (0 && RTEST(v = rb_iv_get(rObject, "_js_ref"))) {
		def = (ruby_js_class_def*)v;
	} else {
		def = jsrb_newClass(rObject);
		def->ctx = ctx;
		rb_iv_set(rObject, "_js_ref", (VALUE)def);
	}

	/*fprintf(stderr, "Klass is %p, Def is %p, ctx is %p\n", rObject, def, ctx);
	fflush(stderr);*/
	object = JSObjectMake(ctx, def->class_ref, NULL);

	js_obj_set_value(ctx, global, name, object); // Check failure?
}
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


