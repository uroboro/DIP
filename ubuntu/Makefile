#findfiles = $(foreach ext, c cpp m mm x xm xi xmi, $(wildcard $(1)/*.$(ext)))
findfiles = $(foreach ext, c cpp, $(wildcard $(1)/*.$(ext)))
getobjs = $(foreach ext, c cpp m mm x xm xi xmi, $(filter %.o,$(patsubst %.$(ext),%.o,$(1))))

OPENCV_CDFLAGS = `pkg-config --cflags opencv`
OPENCV_LDFLAGS = -L/usr/lib/x86_64-linux-gnu/ `pkg-config --libs opencv`
#-stdlib=libc++
# -fopenmp

DIP_FILES = $(call findfiles,sources) $(call findfiles,../shared)
DIP_CFLAGS = $(OPENCV_CFLAGS) -c -Wall -O3 -I ../shared
DIP_LDFLAGS = $(OPENCV_LDFLAGS)

CC = clang++
OBJ_DIR = .objs
OBJECTS = $(patsubst %,$(OBJ_DIR)/%,$(call getobjs, $(DIP_FILES)))
EXECUTABLE = exe

all: $(DIP_FILES) $(EXECUTABLE)

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

$(EXECUTABLE): $(OBJECTS)
	@echo -e "==> Linker"
	$(CC) $^ -o $@ $(DIP_LDFLAGS)

test: $(EXECUTABLE)
	@./$(EXECUTABLE)

clean:
	rm -rf $(OBJ_DIR) $(EXECUTABLE)
