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

If your distribution uses another package manager, install the equivalent SDL2 and OpenGL development packages.

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
- `Space` / `E`: move up
- `Left Ctrl` / `Right Ctrl` / `Q`: move down
- `Left Arrow` / `Right Arrow`: rotate yaw left/right
- `Up Arrow` / `Down Arrow`: rotate pitch up/down
- Mouse movement: look around (arrow keys override mouse while pressed)
- `Escape`: quit

### Debug telemetry

The application updates the SDL window title every quarter second with:

- FPS
- Camera position
- Camera yaw/pitch
- OpenGL version and renderer
- Depth test state
- Runtime uptime

## Clean

To remove compiled artifacts:

```bash
make clean
```
