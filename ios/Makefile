export GO_EASY_ON_ME = 1
export TARGET = iphone:clang:8.1
export ARCHS = armv7 arm64

include $(THEOS)/makefiles/common.mk

OPENCV_CFLAGS = -F /Library/Frameworks/
OPENCV_LDFLAGS = -F /Library/Frameworks/ -framework opencv2 -stdlib=libc++
OPENCV_FRAMEWORKS = UIKit Accelerate AssetsLibrary AVFoundation QuartzCore CoreGraphics CoreImage CoreMedia CoreVideo

#DIP: application
APPLICATION_NAME = DIP
DIP_FILES = $(ADDITIONAL_FILES) $(call findfiles,sources) $(call findfiles,../shared)
DIP_CFLAGS = $(OPENCV_CFLAGS) -I ../shared
DIP_LDFLAGS = $(OPENCV_LDFLAGS)
DIP_FRAMEWORKS = $(OPENCV_FRAMEWORKS) CoreFoundation Foundation MobileCoreServices

include $(THEOS_MAKE_PATH)/application.mk

ipa: $(APPLICATION_NAME).ipa

%.ipa: %
	$(ECHO_NOTHING)echo "Building $@..."$(ECHO_END)
	$(ECHO_NOTHING)pushd $(THEOS_OBJ_DIR)/ &> /dev/null; mkdir -p Payload; cp -r $<.app Payload; zip -qru $@ Payload; rm -rf Payload; popd &> /dev/null$(ECHO_END)
	$(ECHO_NOTHING)mv $(THEOS_OBJ_DIR)/$@ ./$(ECHO_END)

instapp: $(APPLICATION_NAME).ipa
	$(ECHO_NOTHING)echo "Installing $<..."$(ECHO_END)
	$(ECHO_NOTHING)[[ `which appinst` != "" && `appinst $< &> /dev/null` != 0 ]]$(ECHO_END)
	$(ECHO_NOTHING)[[ `which open` != "" && -f "Resources/Info.plist" ]] && echo "Opening `plutil -key CFBundleDisplayName Resources/Info.plist`..."$(ECHO_END)
	$(ECHO_NOTHING)[[ `which open` != "" ]] && open `plutil -key CFBundleIdentifier Resources/Info.plist`$(ECHO_END)
