# Engine Architecture

The custom engine is organized into modular subsystems that separate core responsibilities.

## Core Layers

- `src/main.cpp`
  - Application entry point. Constructs the engine and runs the main loop.

- `src/engine/Engine.cpp`
  - Engine lifecycle management.
  - Window creation, event polling, and frame loop.
  - Delegates rendering and input handling.

- `src/engine/renderer/OpenGLRenderer.cpp`
  - Handles OpenGL initialization, viewport management, and basic rendering.

- `src/engine/input/InputManager.cpp`
  - Handles SDL event processing, keyboard/mouse state, and quit logic.
  - Clears input state when the window loses focus to avoid sticky key behavior.

- `src/engine/Engine.cpp`
  - Manages engine lifecycle and window interactions.
  - Updates debug telemetry in the window title bar every 0.25 seconds.

## Design Principles

- **Modularity:** Each subsystem has a clear purpose and can be extended independently.
- **Separation of concerns:** Platform-specific setup lives in the engine core, while rendering and input logic are isolated.
- **Incremental expansion:** The current foundation can be expanded with scene graphs, resource loading, physics, and audio.

## Execution Flow

1. `main()` creates an `engine::Engine` instance.
2. `Engine::initialize()` starts SDL, creates a window, and initializes the renderer.
3. `Engine::run()` enters the main loop.
4. Each frame polls events, updates input state, renders geometry, and swaps buffers.
5. `Engine::shutdown()` cleans up SDL and OpenGL resources.
