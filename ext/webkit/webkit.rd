= module WebKit
--- WebKit.set_web_database_path(String path)


--- WebKit.remove_all_web_databases()


--- WebKit.set_default_web_database_quota(guint64 quota)


== class WebKit::JavascriptError < StandardError
== class WebKit::WebSettings
--- WebKit::WebSettings.new

== class WebKit::WebFrame
--- WebKit::WebFrame#exec_js(String js)


--- WebKit::WebFrame#setup_ruby

--- WebKit::WebFrame#add_js_api(String name, block)


--- WebKit::WebFrame#load_string(String content, String mime_type, String encoding, String base_uri)


== class WebKit::WebView
--- WebKit::WebView.new

--- WebKit::WebView#open(String uri)


--- WebKit::WebView#execute_script(String script)


--- WebKit::WebView#set_settings(WebKitWebSettings* settings)


--- WebKit::WebView#load_string(String content, String mime_type, String encoding, String base_uri)


--- WebKit::WebView#load_uri(String uri)


--- WebKit::WebView#main_frame

--- WebKit::WebView#focused_frame

--- WebKit::WebView#progress

--- WebKit::WebView#title

--- WebKit::WebView#uri

--- WebKit::WebView#reload

--- WebKit::WebView#reload_bypass_cache

--- WebKit::WebView#set_custom_encoding(String encoding)


--- WebKit::WebView#stop_loading

