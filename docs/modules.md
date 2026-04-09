# Module Overview

This section describes the modular components of the custom engine and how they interact.

## Engine Module

- `include/engine/Engine.h`
- `src/engine/Engine.cpp`

Responsibilities:
- Initialize SDL2 and the OpenGL 3.3 core context (upgraded from 2.1 compatibility).
- Manage the application lifecycle and main loop.
- Forward input events to the input subsystem.
- Orchestrate camera updates, scene rendering, and debug UI.

## Renderer Module

- `include/engine/renderer/OpenGLRenderer.h`
- `src/engine/renderer/OpenGLRenderer.cpp`

Responsibilities:
- Initialize modern OpenGL 3.3 core rendering state.
- Handle viewport resizing and view matrix transformation.
- Render scene geometry (currently using immediate mode; VAO/VBO refactoring in progress).
- Integration with post-processing pipeline.

### Modern Graphics Features (Implemented/Planned)
- **Camera System**: Uses GLM (`glm::vec3`, `glm::quat`) for transformation math; provides `getViewMatrix()` for clean view setup.
- **Post-Processing**: Supports FBO-backed rendering for faster effects; includes shader hot-reload to modify effects at runtime without restarting.
- **Asset Management**: Caching system for textures and models; supports BMP (SDL), ready for PNG/JPG (SDL2_image) and OBJ (tinyobjloader).
- **Modern Context**: Switched to OpenGL 3.3 core profile; ImGui now uses OpenGL3 backend.
- **Future**: VAO/VBO-based geometry rendering will replace immediate mode and enable efficient batch rendering.

## Asset Manager

- `include/engine/AssetManager.h`
- `src/engine/AssetManager.cpp`

Responsibilities:
- Cache and manage textures and 3D models.
- Load textures from disk (BMP, and PNG/JPG if SDL2_image is installed).
- Load 3D models (OBJ support ready if tinyobjloader is installed).
- Prevent duplicate resource loads by caching via weak pointers.

To enable optional loaders:
- **PNG/JPG support**: Install `libsdl2-image-dev` and uncomment SDL2_image includes in AssetManager.cpp.
- **OBJ model loading**: Install `libtinyobjloader-dev` or vendor `tinyobjloader.h`, then uncomment the tinyobjloader includes.

## Camera & Input

- **Camera**: `include/engine/camera/Camera.h`, `src/engine/camera/Camera.cpp`
  - Smooth first-person camera using GLM for math.
  - Mouse and keyboard look controls (arrow keys + mouse).
  - Position and rotation stored as `glm::vec3` and Euler angles.

- **InputManager**: `include/engine/input/InputManager.h`, `src/engine/input/InputManager.cpp`
  - Processes SDL events for keyboard and mouse input.
  - Detects quit requests and relative mouse motion.

## Debug UI (ImGui Integration)

- `include/engine/debug/ImGuiDebugger.h`
- `src/engine/debug/ImGuiDebugger.cpp`

Uses ImGui with the OpenGL3 backend for in-editor panels showing:
- Performance metrics (FPS, frame time, per-stage timings).
- Camera position and rotation.
- Rendering toggles (depth test, face culling, wireframe).
- Post-processing shader control and hot-reload UI.

## Event Bus & Messaging

- `include/engine/EventBus.h`

A thread-safe, header-only event dispatcher for decoupled system communication.
Systems can publish and subscribe to typed events without tight coupling.

## ECS (Entity Component System)

- `include/engine/ecs/Entity.h`

Placeholder for ECS integration. Ready to add EnTT or a custom ECS for data-driven entity management.

## Platform Structure

- SDL2 window and context creation is in `src/engine/Engine.cpp`.
- Future platform backends can be added under `src/engine/platform/`.

