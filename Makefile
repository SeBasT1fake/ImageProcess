CXX = g++
CXXFLAGS = -std=c++11 -Wall -Wextra -Wno-missing-field-initializers -O
LDFLAGS = -lm

# Directory structure
SRC_DIR = src
BUILD_DIR = build
BIN_DIR = bin
TARGET = $(BIN_DIR)/program_image

# Source files
SOURCES = $(wildcard $(SRC_DIR)/*.cpp)
OBJECTS = $(SOURCES:$(SRC_DIR)/%.cpp=$(BUILD_DIR)/%.o)

# Header files with STB
INCLUDES = -I./$(SRC_DIR)
LIBS = $(SRC_DIR)/stb_image.h $(SRC_DIR)/stb_image_write.h

# Create necessary directories
MKDIR_P = mkdir -p

all: directories $(LIBS) $(TARGET)

directories:
	$(MKDIR_P) $(BUILD_DIR)
	$(MKDIR_P) $(BIN_DIR)

$(TARGET): $(OBJECTS)
	$(CXX) $(OBJECTS) -o $@ $(LDFLAGS)

$(BUILD_DIR)/%.o: $(SRC_DIR)/%.cpp
	$(CXX) $(CXXFLAGS) $(INCLUDES) -c $< -o $@

$(SRC_DIR)/stb_image.h:
	$(MKDIR_P) $(SRC_DIR)
	curl -o $@ https://raw.githubusercontent.com/nothings/stb/master/stb_image.h

$(SRC_DIR)/stb_image_write.h:
	$(MKDIR_P) $(SRC_DIR)
	curl -o $@ https://raw.githubusercontent.com/nothings/stb/master/stb_image_write.h

clean:
	rm -rf $(BUILD_DIR) $(BIN_DIR)

clean_objs:
	rm -f $(OBJECTS)

.PHONY: all clean clean_objs directories