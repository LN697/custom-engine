# Getting Started

This section explains how to prepare the development environment, build the project, and run the custom engine sample.

## Prerequisites

- Linux development environment
- `g++` with C++17 support
- `pkg-config`
- SDL2 development libraries
- OpenGL development libraries

## Install Dependencies

For Debian/Ubuntu-based distributions:

```bash
sudo apt update
sudo apt install -y build-essential pkg-config libsdl2-dev libgl1-mesa-dev
```

### Optional Dependencies

To enable PNG/JPG texture loading via SDL2_image:
```bash
sudo apt install -y libsdl2-image-dev
```

To enable OBJ model loading:
```bash
sudo apt install -y libtinyobjloader-dev
```

If your distribution uses another package manager, install the equivalent SDL2, OpenGL, SDL2_image, and tinyobjloader development packages.

After installing optional libraries, uncomment the corresponding includes in `src/engine/AssetManager.cpp` to enable those features.

## Build

From the project root:

```bash
make
```

The build output is placed in `build/`, and the final executable is named `custom-engine`.

## Run

From the project root:

```bash
./custom-engine
```

The sample application opens an SDL2 window with a simple OpenGL scene.

### Controls

- `W`: move forward
- `S`: move backward
- `A`: move left
- `D`: move right
- `Space`: move up
- `Left Ctrl` / `Right Ctrl`: move down
- `Left Arrow` / `Right Arrow`: look left/right (keyboard)
- `Up Arrow` / `Down Arrow`: look up/down (keyboard)
- `Mouse Movement`: look around (when mouse is captured)
- `Mouse Click`: capture/lock mouse for look control
- `Escape`: release mouse capture
- `Q`: quit application

### Debug Telemetry

The ImGui debug panel (always visible) displays:
- FPS (frames per second)
- Frame time (milliseconds)
- Per-stage timing breakdown (event, camera, scene, render, post-process)
- Camera position (X, Y, Z)
- Camera rotation (yaw, pitch in degrees)
- OpenGL version and renderer information
- Depth test state
- Runtime uptime (seconds since start)

The window title bar updates approximately every quarter second with summary statistics.

## Clean

To remove compiled artifacts:

```bash
make clean
```
