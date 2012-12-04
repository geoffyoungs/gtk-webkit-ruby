require 'gtk2'
require 'webkit'

webkit = WebKit::WebView.new
webkit.open 'http://google.com'

window = Gtk::Window.new
window.title = 'Ruby Webkit'

# Center the window and resize
window.resize 800, 600
window.window_position = Gtk::Window::POS_CENTER

# Exit when closing the window
window.signal_connect('destroy') { Gtk.main_quit }

# Add the webview to the window
window.add webkit
window.show_all

Gtk.main
