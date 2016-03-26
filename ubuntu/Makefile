#findfiles = $(foreach ext, c cpp m mm x xm xi xmi, $(wildcard $(1)/*.$(ext)))
findfiles = $(foreach ext, c cpp, $(wildcard $(1)/*.$(ext)))
getobjs = $(foreach ext, c cpp m mm x xm xi xmi, $(filter %.o,$(patsubst %.$(ext),%.o,$(1))))

OPENCV_CDFLAGS = `pkg-config --cflags opencv`
OPENCV_LDFLAGS = -L/usr/lib/x86_64-linux-gnu/ `pkg-config --libs opencv`
#-stdlib=libc++
# -fopenmp

DIP_FILES = $(call findfiles,sources) $(call findfiles,shared)
DIP_CFLAGS = $(OPENCV_CFLAGS) -c -Wall -O3 -I shared
DIP_LDFLAGS = $(OPENCV_LDFLAGS)

TARGET = native:clang
TARGET_CC = clang
TARGET_CXX = clang++
#ARCHS = x86_64
TOOL_NAME = DIP
#include $(THEOS)/makefiles/common.mk
#include $(THEOS_MAKE_PATH)/tool.mk

CC = clang++
OBJ_DIR = .objs
OBJECTS = $(patsubst %,$(OBJ_DIR)/%,$(call getobjs, $(DIP_FILES)))

all: $(DIP_FILES) $(TOOL_NAME)

test: $(TOOL_NAME)
	@echo "==> ./$^"
	@$(or $(THEOS_OBJ_DIR),./)/$(TOOL_NAME)

$(OBJECTS): | $(OBJ_DIR)

$(OBJ_DIR):
	@mkdir -p $@

$(OBJ_DIR)/%.o: %.c
	@mkdir -p `dirname $@`
	@echo "==> Compilando" $<
	$(CC) $(DIP_CFLAGS) -x c $< -o $@

$(OBJ_DIR)/%.o: %.cpp
	@mkdir -p `dirname $@`
	@echo "==> Compilando" $<
	$(CC) $(DIP_CFLAGS) $< -o $@

$(TOOL_NAME): $(OBJECTS)
	@echo -e "==> Linker"
	$(CC) $^ -o $@ $(DIP_LDFLAGS)

clean:
	rm -rf $(OBJ_DIR) $(TOOL_NAME)
