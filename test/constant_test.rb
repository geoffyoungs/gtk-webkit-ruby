require 'test/unit'
require 'gtk2'
$:.unshift 'ext/webkit'
$:.unshift 'lib'
$:.unshift 'x86_64-linux'
require 'webkit'

class ConstantTest < Test::Unit::TestCase
	def setup
	end

	def test_constants
		assert_equal WebKit::DOWNLOAD_STATUS_ERROR, WebKit::DownloadStatus::ERROR
		assert_equal WebKit::WEB_VIEW_TARGET_INFO_IMAGE, WebKit::TargetInfo::IMAGE
	end
end

