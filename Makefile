
SRC_DIR := ./src
INC_DIR := ./include
BUILD_DIR := ./build

SOURCES := $(wildcard $(SRC_DIR)/*.cpp)
OBJECTS := $(patsubst $(SRC_DIR)/%.cpp, $(BUILD_DIR)/%.o, $(SOURCES))

CC=clang++
CFLAGS=-Wall -std=c++17 -I$(INC_DIR) -I./third-party/metal-cpp -I./third-party/metal-cpp-extensions -fno-objc-arc $(DBG_OPT_FLAGS) $(ASAN_FLAGS)
LDFLAGS=-framework Metal -framework Foundation -framework Cocoa -framework CoreGraphics -framework MetalKit 

ifdef DEBUG
CFLAGS += -g
else
CFLAGS += -O2
endif

ifdef ASAN
CFLAGS += -fsanitize=address
endif

TARGET := $(BUILD_DIR)/renderer

all: $(TARGET)

$(TARGET): $(OBJECTS)
	$(CC) $(CFLAGS) $(LDFLAGS) $^ -o $@

$(BUILD_DIR)/%.o: $(SRC_DIR)/%.cpp
	mkdir -p $(BUILD_DIR)
	$(CC) $(CFLAGS) -c $< -o $@

.PHONY: clean

clean:
	rm -rf $(BUILD_DIR)/*.o $(TARGET)