# Modular Makefile: auto-detect modules, include dirs and sources

PKG_CONFIG := pkg-config
CXX := g++
STD := -std=c++17
CXXFLAGS := -Wall -Wextra -O3 $(STD)
SDL_CFLAGS := $(shell $(PKG_CONFIG) --cflags sdl2 2>/dev/null)
SDL_LIBS := $(shell $(PKG_CONFIG) --libs sdl2 2>/dev/null)
GL_LIBS := -lGL
LDLIBS := $(SDL_LIBS) $(GL_LIBS)

# find all include directories under include/
INC_DIRS := $(shell find include -type d -print)
CPPFLAGS := $(patsubst %,-I%,$(INC_DIRS)) $(SDL_CFLAGS)

# collect all source files under src/
SRCS := $(shell find src -name '*.cpp' ! -path './build/*' -print | sed 's|^./||')

# place objects under build/ preserving directory structure
OBJS := $(patsubst %.cpp,build/%.o,$(SRCS))

TARGET := custom-engine

.DEFAULT_GOAL := all
.PHONY: all clean show deps

deps:
	@echo "Install Linux dependencies for SDL2/OpenGL:"
	@echo "  sudo apt update && sudo apt install -y libsdl2-dev libgl1-mesa-dev"
	@echo "If you use another distro, install the equivalent SDL2 and OpenGL development packages."

all: $(TARGET)

run: $(TARGET)
	./$(TARGET)

$(TARGET): $(OBJS)
	@echo Linking $@
	$(CXX) $(CXXFLAGS) -o $@ $(OBJS) $(LDLIBS)

# generic rule: compile source -> object under build/
# ensures directory exists before compiling
build/%.o: %.cpp
	@mkdir -p $(dir $@)
	@echo Compiling $<
	$(CXX) $(CXXFLAGS) $(CPPFLAGS) -c $< -o $@

show:
	@echo "Include dirs: $(INC_DIRS)"
	@echo "Sources: $(SRCS)"
	@echo "Objects: $(OBJS)"

clean:
	@echo Cleaning build artifacts
	@rm -rf build/ $(TARGET)
