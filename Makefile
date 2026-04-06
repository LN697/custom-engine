# Modular Makefile: auto-detect modules, include dirs and sources

PKG_CONFIG := pkg-config
CXX := g++
STD := -std=c++17
CXXFLAGS := -Wall -Wextra -O3 $(STD)
SDL_CFLAGS := $(shell $(PKG_CONFIG) --cflags sdl2 2>/dev/null)
SDL_LIBS := $(shell $(PKG_CONFIG) --libs sdl2 2>/dev/null)
GL_LIBS := -lGL
LDLIBS := $(SDL_LIBS) $(GL_LIBS)

# ImGui support - include if third_party/imgui directory exists
IMGUI_DIR := third_party/imgui
ifeq ($(shell test -d $(IMGUI_DIR) && echo 1),1)
    IMGUI_SRCS := $(IMGUI_DIR)/imgui.cpp \
                  $(IMGUI_DIR)/imgui_demo.cpp \
                  $(IMGUI_DIR)/imgui_draw.cpp \
                  $(IMGUI_DIR)/imgui_widgets.cpp \
                  $(IMGUI_DIR)/imgui_tables.cpp \
                  $(IMGUI_DIR)/backends/imgui_impl_sdl2.cpp \
                  $(IMGUI_DIR)/backends/imgui_impl_opengl2.cpp
    IMGUI_CPPFLAGS := -I$(IMGUI_DIR) -I$(IMGUI_DIR)/backends
else
    IMGUI_SRCS :=
    IMGUI_CPPFLAGS :=
endif

# find all include directories under include/
INC_DIRS := $(shell find include -type d -print)
CPPFLAGS := $(patsubst %,-I%,$(INC_DIRS)) $(SDL_CFLAGS) $(IMGUI_CPPFLAGS)

# collect all source files under src/
SRCS := $(shell find src -name '*.cpp' ! -path './build/*' -print | sed 's|^./||') $(IMGUI_SRCS)

# place objects under build/ preserving directory structure
OBJS := $(patsubst %.cpp,build/%.o,$(SRCS))

TARGET := custom-engine

.DEFAULT_GOAL := all
.PHONY: all clean show deps setup-imgui

deps:
	@echo "Install Linux dependencies for SDL2 and OpenGL:"
	@echo "  sudo apt update && sudo apt install -y libsdl2-dev libgl1-mesa-dev"
	@echo "If you use another distro, install the equivalent SDL2 and OpenGL development packages."
	@echo ""
	@echo "Then run: make setup-imgui"

setup-imgui:
	@echo "Setting up ImGui..."
	@mkdir -p third_party
	@if [ ! -d third_party/imgui ]; then \
		cd third_party && \
		git clone https://github.com/ocornut/imgui.git && \
		echo "ImGui downloaded successfully"; \
	else \
		echo "ImGui already exists at third_party/imgui"; \
	fi

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
	@echo "IMGUI support: $(IMGUI_SRCS)"
	@echo "Sources: $(SRCS)"
	@echo "Objects: $(OBJS)"

clean:
	@echo Cleaning build artifacts
	@rm -rf build/ $(TARGET)
