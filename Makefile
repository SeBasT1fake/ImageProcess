CXX = g++
CXXFLAGS = -std=c++11 -Wall -O2
LDFLAGS = -lm

# Source files
SOURCES = main.cpp buddy_allocator.cpp image_processor.cpp
OBJECTS = $(SOURCES:.cpp=.o)
EXECUTABLE = programa_imagen

# Header files with STB
INCLUDES = -I./

# Libraries
LIBS = stb_image.h stb_image_write.h

all: $(EXECUTABLE)

$(EXECUTABLE): $(OBJECTS)
	$(CXX) $(OBJECTS) -o $@ $(LDFLAGS)

%.o: %.cpp $(LIBS)
	$(CXX) $(CXXFLAGS) $(INCLUDES) -c $< -o $@

# Download STB libraries if they don't exist
stb_image.h:
	curl -o stb_image.h https://raw.githubusercontent.com/nothings/stb/master/stb_image.h

stb_image_write.h:
	curl -o stb_image_write.h https://raw.githubusercontent.com/nothings/stb/master/stb_image_write.h

clean:
	rm -f $(OBJECTS) $(EXECUTABLE)

.PHONY: all clean