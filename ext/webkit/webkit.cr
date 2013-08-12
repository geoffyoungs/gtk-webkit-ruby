%pkg-config gtk+-2.0
%pkg-config webkit-1.0
%name webkit

%lib dl
%lib ffi
%include dlfcn.h
%include errno.h
%include ffi.h
%include rbgobject.h
%include sys/mman.h
%include intern.h

%{
#include <intern.h>
#include <ruby.h>
#include "javascript.h"

static inline VALUE strOrNil(const char *str) {
	if (str) {
		return rb_str_new2(str);
	} else {
		return Qnil;
	}
}

%}

%include webkit/webkit.h
%include webkit/webkitenumtypes.h
%include JavaScriptCore/JavaScript.h

%map strOrNil > VALUE : strOrNil(%%)

%map SoupMessage* > VALUE : GOBJ2RVAL(SOUP_MESSAGE(%%))

module WebKit
	gcpool RubyFunc

  class JsPtr
  end

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

	gobject WebPolicyDecision < WEBKIT_TYPE_WEB_POLICY_DECISION
		@type WebKitWebPolicyDecision
		def download
			webkit_web_policy_decision_download(_self);
		end
		def use
			webkit_web_policy_decision_use(_self);
		end
		def ignore
			webkit_web_policy_decision_ignore(_self);
		end
	end

	gobject WebFrame < WEBKIT_TYPE_WEB_FRAME
		@type WebKitWebFrame
		def exec_js(char *js)
			return javascript_exec(webkit_web_frame_get_global_context(_self), js);
		end
		def add_ruby_class(char *name, T_CLASS|T_MODULE klass)
			javascript_add_class(webkit_web_frame_get_global_context(_self), name, klass);
		end
		def add_ruby_eval()
			javascript_add_ruby_eval(webkit_web_frame_get_global_context(_self));
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
		def strOrNil:title
			return webkit_web_view_get_title(_self);
		end
		def strOrNil:uri
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
		def bool:has_selection?
			return webkit_web_view_has_selection(_self);
		end
		def WebKitWebInspector*:inspector
			return webkit_web_view_get_inspector(_self);
		end
	end

	gobject WebInspector < WEBKIT_TYPE_WEB_INSPECTOR
		@type WebKitWebInspector
		def inspect_coordinates(double x, double y)
			webkit_web_inspector_inspect_coordinates(_self, x, y);
		end
		def strOrNil:uri
			return webkit_web_inspector_get_inspected_uri(_self);
		end
		def show
			webkit_web_inspector_show(_self);
		end
		def close
			webkit_web_inspector_close(_self);
		end
		def WebKitWebView*:view
			return webkit_web_inspector_get_web_view(_self);
		end
	end

	gobject WebResource < WEBKIT_TYPE_WEB_RESOURCE
		@type WebKitWebResource
		def strOrNil:encoding
			return webkit_web_resource_get_encoding(_self);
		end
		def strOrNil:frame_name
			return webkit_web_resource_get_frame_name(_self);
		end
		def strOrNil:mime_type
			return webkit_web_resource_get_mime_type(_self);
		end
		def strOrNil:uri
			return webkit_web_resource_get_uri(_self);
		end
		def data
			GString *data = webkit_web_resource_get_data(_self);
			return rb_str_new(data->str, data->len);
		end
		def data=(T_STRING data)
			GString *odata = webkit_web_resource_get_data(_self);
			g_free(odata->str);
			odata->str = g_malloc(RSTRING_LEN(data)+1);
			odata->len = odata->allocated_len = RSTRING_LEN(data);
			memcpy(RSTRING_PTR(data), odata->str, RSTRING_LEN(data));
			return data;
		end
	end

	gobject WebNetworkRequest < WEBKIT_TYPE_NETWORK_REQUEST
		@type WebKitNetworkRequest
		def strOrNil:uri
			return webkit_network_request_get_uri(_self);
		end
		def uri=(char *uri)
			webkit_network_request_set_uri(_self, uri);
		end
		def SoupMessage*:message
			return webkit_network_request_get_message(_self);
		end
	end

	gobject WebNetworkResponse < WEBKIT_TYPE_NETWORK_RESPONSE
		@type WebKitNetworkResponse
		def strOrNil:uri
			return webkit_network_response_get_uri(_self);
		end
		def uri=(char *uri)
			webkit_network_response_set_uri(_self, uri);
		end
		def SoupMessage*:message
			return webkit_network_response_get_message(_self);
		end
 	end
  
	gobject Download < WEBKIT_TYPE_DOWNLOAD
		@type WebKitDownload
		def initialize(WebKitNetworkRequest *request)
			RBGTK_INITIALIZE(self, webkit_download_new(request));
		end
		def start
			webkit_download_start(_self);
		end
		def cancel
			webkit_download_cancel(_self);
		end
		def double:progress
			return webkit_download_get_progress(_self);
		end
		def guint64:current_size
			return webkit_download_get_current_size(_self);
		end
		def guint64:total_size
			return webkit_download_get_total_size(_self);
		end
		def char*:uri
			return webkit_download_get_uri(_self);
		end
		def char*:suggested_filename
			return webkit_download_get_suggested_filename(_self);
		end
	end

  genum TargetInfo           WEBKIT_TYPE_WEB_VIEW_TARGET_INFO    WEBKIT_;

  genum DownloadStatus       WEBKIT_TYPE_DOWNLOAD_STATUS         WEBKIT_;
  genum DownloadError        WEBKIT_TYPE_DOWNLOAD_ERROR          WEBKIT_;
  genum NetworkError         WEBKIT_TYPE_NETWORK_ERROR           WEBKIT_;
  genum PolicyError          WEBKIT_TYPE_POLICY_ERROR            WEBKIT_;
  genum PluginError          WEBKIT_TYPE_PLUGIN_ERROR            WEBKIT_;

  genum CacheModel           WEBKIT_TYPE_CACHE_MODEL             WEBKIT_;
  genum LoadStatus           WEBKIT_TYPE_LOAD_STATUS             WEBKIT_;

  genum NavigationReason     WEBKIT_TYPE_WEB_NAVIGATION_REASON   WEBKIT_;
  genum HitTestResultContext WEBKIT_TYPE_HIT_TEST_RESULT_CONTEXT WEBKIT_;
  genum EditingBehavior      WEBKIT_TYPE_EDITING_BEHAVIOR        WEBKIT_;
  genum NavigationResponse   WEBKIT_TYPE_NAVIGATION_RESPONSE     WEBKIT_;

  # genum ViewMode             WEBKIT_TYPE_WEB_VIEW_VIEW_MODE  WEBKIT_;
  # genum SelectionAffinity    WEBKIT_TYPE_SELECTION_AFFINITY  WEBKIT_;
  # genum InsertAction         WEBKIT_TYPE_INSERT_ACTION       WEBKIT_;
end

