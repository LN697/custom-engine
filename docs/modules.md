# Module Overview

This section describes the modular components of the custom engine and how they interact.

## Engine Module

- `include/engine/Engine.h`
- `src/engine/Engine.cpp`

Responsibilities:
- Initialize SDL2 and the OpenGL context.
- Manage the application lifecycle and main loop.
- Forward input events to the input subsystem.
- Forward render calls to the renderer subsystem.

## Renderer Module

- `include/engine/renderer/OpenGLRenderer.h`
- `src/engine/renderer/OpenGLRenderer.cpp`

Responsibilities:
- Initialize OpenGL rendering state.
- Handle viewport resizing.
- Render a simple triangle as a graphics test.
- Provide a clean interface for future renderer extensions.

## Input Module

- `include/engine/input/InputManager.h`
- `src/engine/input/InputManager.cpp`

Responsibilities:
- Process SDL events for input.
- Detect quit requests from window close and ESC key.
- Allow the engine loop to remain platform-agnostic.

## Platform Structure

- `src/engine/Engine.cpp` contains SDL-specific setup for window creation and context management.
- Future platform backends can be added under `src/engine/platform/` and swapped by abstracting the platform layer.
