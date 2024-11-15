CC := clang
MODE := debug
LIBS := -lreadline

SOURCE_DIR := src
BUILD_DIR := bin
OBJECT_DIR := $(BUILD_DIR)/tmp

TARGET := clox
BUILD_TARGET := $(BUILD_DIR)/$(TARGET)

ifeq ($(CPP),true)
	CC := clang++
	CFLAGS := -std=c++11 -Wno-c99-designator -Wno-writable-strings
	C_LANG := -x c++
else
	CFLAGS := -std=c99
endif

CFLAGS += -Wall -Wextra -Wno-unused-parameter

ifeq ($(SNIPPET),true)
	CFLAGS += -Wno-unused-function
endif

ifeq ($(MODE),debug)
	CFLAGS += -O0 -DDEBUG -ggdb
else
	CFLAGS += -O3 -flto
endif

HEADERS := $(wildcard $(SOURCE_DIR)/include/*.h)
SOURCES := $(wildcard $(SOURCE_DIR)/*.c)
OBJECTS := $(patsubst $(SOURCE_DIR)/%.c, $(OBJECT_DIR)/%.o, $(SOURCES))

default: $(OBJECTS)
	@ echo "Building $(TARGET)..."
	@ $(CC) $(CFLAGS) $^ -o $(BUILD_TARGET) $(LIBS)
	@ rm $(OBJECT_DIR)/*.o
	@ echo "$(TARGET) built successfully!"

clean:
	@ rm $(BUILD_TARGET) $(TARGET)

$(OBJECT_DIR)/%.o: $(SOURCE_DIR)/%.c $(HEADERS)
	@ mkdir -p $(OBJECT_DIR)
	@ echo "  compiling: $< --> $@"
	@ $(CC) -c $(C_LANG) $(CFLAGS) -o $@ $<

$(OBJECT_DIR):
	@ mkdir -p $(OBJECT_DIR)

$(BUILD_DIR):
	@ mkdir -p $(BUILD_DIR)

.PHONY: default clean
.DEFAULT: default
