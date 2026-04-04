# Custom Engine

A modular C++ game engine foundation using SDL2 for input/window management and OpenGL for rendering.

## Project Layout

- `src/` — application source files and engine modules
- `include/` — public engine headers
- `build/` — generated object files and build artifacts
- `docs/` — design, setup, and architecture documentation

## Build

```bash
make
```

## Run

```bash
./custom-engine
```

## Controls

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

The window title bar shows runtime telemetry including FPS, camera position, orientation, depth test state, and OpenGL information.

## Documentation

Read the docs in the `docs/` folder for setup, architecture, and module details.
