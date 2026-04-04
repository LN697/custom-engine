# Dependencies

This engine depends on the following libraries and tools:

- `g++` / GNU Compiler Collection with C++17 support
- `pkg-config` for discovering SDL2 compiler and linker flags
- `libsdl2-dev` for SDL2 development headers and libraries
- `libgl1-mesa-dev` or equivalent for OpenGL development

## Linux Installation

For Debian/Ubuntu-based systems:

```bash
sudo apt update
sudo apt install -y build-essential pkg-config libsdl2-dev libgl1-mesa-dev
```

## Package Discovery

The `Makefile` uses `pkg-config --cflags sdl2` and `pkg-config --libs sdl2` to find the correct include paths and libraries.

If SDL2 is not installed or `pkg-config` cannot locate it, install the distribution package for SDL2 development or adjust your environment paths.
