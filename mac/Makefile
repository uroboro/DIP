export ARCHS = i386
export ARCHS = x86_64
export TARGET = native::latest

include $(THEOS)/makefiles/common.mk

OPENCV_CFLAGS = $(shell PKG_CONFIG_PATH="/usr/local/opt/opencv@2/lib/pkgconfig" pkg-config --cflags opencv)
OPENCV_LDFLAGS = $(shell PKG_CONFIG_PATH="/usr/local/opt/opencv@2/lib/pkgconfig" pkg-config --libs opencv)
#-L/usr/lib/x86_64-linux-gnu/

TOOL_NAME = mac select_hands validate_image
mac_FILES = main.cpp $(call findfiles,shared) $(call findfiles,shared++)
mac_CFLAGS = $(OPENCV_CFLAGS) -I shared -I shared++
mac_LDFLAGS = $(OPENCV_LDFLAGS)

select_hands_FILES = select_hands.cpp
select_hands_CFLAGS = $(OPENCV_CFLAGS) -I shared -I shared++
select_hands_LDFLAGS = $(OPENCV_LDFLAGS)

validate_image_FILES = validate.cpp
validate_image_CFLAGS = $(OPENCV_CFLAGS) -I shared -I shared++
validate_image_LDFLAGS = $(OPENCV_LDFLAGS)

include $(THEOS_MAKE_PATH)/tool.mk

test: all
	@$(THEOS_OBJ_DIR)/mac $(ARGS)

select: all
	@$(THEOS_OBJ_DIR)/select_hands

validate: all
	# @$(THEOS_OBJ_DIR)/validate_image ../Hands/Hand_0000002.jpg
	@$(THEOS_OBJ_DIR)/validate_image $(ARGS)
