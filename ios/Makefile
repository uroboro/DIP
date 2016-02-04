export GO_EASY_ON_ME = 1
export TARGET = iphone:clang:8.1
export ARCHS = armv7 arm64

include $(THEOS)/makefiles/common.mk

findfiles = $(foreach ext, c cpp m mm x xm xi xmi, $(wildcard $(1)/*.$(ext)))

OPENCV_CFLAGS = -F /Library/Frameworks/
OPENCV_LDFLAGS = -F /Library/Frameworks/ -framework opencv2 -stdlib=libc++
OPENCV_FRAMEWORKS = UIKit Accelerate AssetsLibrary AVFoundation QuartzCore CoreGraphics CoreImage CoreMedia CoreVideo

#DIP: application
APPLICATION_NAME = DIP
DIP_FILES = $(call findfiles,sources) $(call findfiles,../shared)
DIP_CFLAGS = $(OPENCV_CFLAGS) -I ../shared
DIP_LDFLAGS = $(OPENCV_LDFLAGS)
DIP_FRAMEWORKS = CoreFoundation Foundation MobileCoreServices $(OPENCV_FRAMEWORKS)

#debug files
ifeq ($(debug),1)
ADDITIONAL_FILES += $(call findfiles,debug)
ADDITIONAL_CFLAGS += -include debug/signalcatcher.h
else
ADDITIONAL_CFLAGS += -DSC=""
endif

include $(THEOS_MAKE_PATH)/application.mk

ipa: $(APPLICATION_NAME).ipa

$(APPLICATION_NAME).ipa: $(APPLICATION_NAME)
	$(ECHO_NOTHING)echo "Creating $@"$(ECHO_END)
	$(ECHO_NOTHING)pushd $(THEOS_OBJ_DIR)/ &> /dev/null; mkdir -p Payload; cp -r $(APPLICATION_NAME).app Payload; zip -qru $@ Payload; rm -rf Payload; popd &> /dev/null$(ECHO_END)
	$(ECHO_NOTHING)mv $(THEOS_OBJ_DIR)/$(APPLICATION_NAME).ipa ./$(ECHO_END)

instapp: $(APPLICATION_NAME).ipa
	$(ECHO_NOTHING)echo "Installing $(APPLICATION_NAME)"$(ECHO_END)
	$(ECHO_NOTHING)[[ `which appinst` != "" && `appinst $(APPLICATION_NAME).ipa` != 0 ]]$(ECHO_END)
	$(ECHO_NOTHING)[[ `which open` != "" ]] && open `plutil -key CFBundleIdentifier Resources/Info.plist`$(ECHO_END)
