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

The sample application opens an SDL2 window with a simple OpenGL triangle and responds to input events.

## Clean

To remove compiled artifacts:

```bash
make clean
```
