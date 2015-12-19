findfiles = $(foreach ext, c cpp m mm x xm xi xmi, $(wildcard $(1)/*.$(ext)))
getobjs = $(filter %.o,$(foreach ext, c cpp m mm x xm xi xmi, $(patsubst %.$(ext),%.o,$(1))))

OPENCV_CDFLAGS = `pkg-config --cflags opencv`
OPENCV_LDFLAGS = -L/usr/lib/x86_64-linux-gnu/ `pkg-config --libs opencv`
#-stdlib=libc++

SOURCE_DIR = sources
OBJ_DIR = .objs

CC = g++
CFLAGS = $(OPENCV_CFLAGS) -c -Wall -O3
LDFLAGS = $(OPENCV_LDFLAGS)
# -fopenmp
SOURCES = $(call findfiles,$(SOURCE_DIR))
OBJECTS = $(patsubst $(SOURCE_DIR)/%,$(OBJ_DIR)/%,$(call getobjs, $(SOURCES)))
EXECUTABLE = Prueba

all: $(SOURCES) $(EXECUTABLE)

$(OBJ_DIR)/%.o: $(SOURCE_DIR)/%.c
	@mkdir -p .objs
	@echo "==> Compilando" $<
	$(CC) $(CFLAGS) -x c $< -o $@

$(OBJ_DIR)/%.o: $(SOURCE_DIR)/%.cpp
	@mkdir -p .objs
	@echo "==> Compilando" $<
	$(CC) $(CFLAGS) $< -o $@

$(EXECUTABLE): $(OBJECTS)
	@echo -e "==> Linker"
	$(CC) $^ -o $@ $(LDFLAGS)

test: $(EXECUTABLE)
	@./$(EXECUTABLE)

clean:
	rm -rf $(OBJECTS) $(EXECUTABLE)
