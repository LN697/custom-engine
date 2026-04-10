# Dependencies

This engine requires the following core libraries and tools:

## Core Dependencies (Required)

- `g++` / GNU Compiler Collection with C++17 support
- `pkg-config` - For discovering library compiler and linker flags
- `libsdl2-dev` - SDL2 development headers and libraries (window, input, graphics context)
- `libgl1-mesa-dev` - OpenGL 3.3 core profile development headers

## Linux Installation

For Debian/Ubuntu-based systems:

```bash
sudo apt update
sudo apt install -y build-essential pkg-config libsdl2-dev libgl1-mesa-dev
```

## Optional Dependencies

These are optional and can be enabled by modifying source code:

- `libsdl2-image-dev` - For PNG/JPG texture loading support
- `libtinyobjloader-dev` - For OBJ 3D model loading support

To enable optional features:
1. Install the libraries above
2. Uncomment the corresponding includes in `src/engine/AssetManager.cpp`
3. Rebuild with `make clean && make`

## Build System

The `Makefile` uses:
- `pkg-config` to discover SDL2 compiler flags and libraries
- Standard GNU Make for build orchestration
- No external build system (CMake) required

## Package Discovery

The build process automatically discovers SDL2 and OpenGL via:
```bash
pkg-config --cflags sdl2   # Compiler flags
pkg-config --libs sdl2     # Linker libraries
```

If SDL2 is not found, ensure `libsdl2-dev` is installed and `pkg-config` can locate it.

## Verified Configurations

- **Ubuntu 20.04+**: All core dependencies available via `apt`
- **Debian 11+**: All core dependencies available via `apt`
- **Fedora/RHEL**: Substitute `dnf` for `apt` in installation commands

For other distributions, install equivalent development packages for SDL2 and OpenGL.
