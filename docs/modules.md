# Module Overview

This section describes the modular components of the custom engine and how they interact.

## Engine Module

- `include/engine/engine.h`
- `src/engine/Engine.cpp`

**Responsibilities**:
- Initialize SDL2 and the OpenGL 3.3 core context.
- Create and manage the SDL2 window.
- Manage the application lifecycle and main loop.
- Orchestrate subsystem initialization and shutdown.
- Forward events to input, renderer, and debug subsystems.
- Coordinate camera updates, scene updates, and rendering per frame.

**Key Features**:
- High-precision delta_time using SDL_GetPerformanceCounter
- Stage-based timing for performance bottleneck analysis
- Event loop with up to 1000 events/frame safeguard
- Relative mouse mode toggle (click to grab, Escape to release)
- Window resize handling with dynamic viewport updates

## Renderer Module

- `include/engine/renderer.h` (consolidated from `renderer/`)
- `src/engine/renderer/OpenGLRenderer.cpp`

**Responsibilities**:
- Initialize OpenGL 3.3 core rendering context
- Manage viewport and projection setup
- Handle screen resizing and aspect ratio recalculation
- Clear framebuffer and prepare render state each frame
- Delegate scene rendering (scene calls renderer methods)

**Key Features**:
- Viewport resizing and aspect ratio management
- High-precision delta_time calculation
- Stage-based timing measurements
- X11 mouse grab support
- Window maximize/raise support

## Input Manager

- `include/engine/input_manager.h` (consolidated from `input/`)
- `src/engine/input/InputManager.cpp`

**Responsibilities**:
- Process SDL2 keyboard and mouse events
- Maintain keyboard state via SDL_GetKeyboardState()
- Track quit requests (Q key)
- Provide accessor methods for movement and look controls

**Supported Input**:
- **Movement**: W (forward), S (backward), A (left), D (right), Space (up), Ctrl (down)
- **Look**: Arrow keys (keyboard look), mouse relative motion
- **Quit**: Q key triggers quit request
- Events propagated to engine and debugger

## Camera System

- `include/engine/camera.h` (consolidated from `camera/`)
- `src/engine/camera/Camera.cpp`

**Responsibilities**:
- Maintain first-person camera position and orientation
- Apply smooth movement interpolation
- Update view matrix based on input and delta_time
- Constrain pitch angle to prevent gimbal lock

**Movement Model**:
- Linear movement at k_movement_speed = 4.0 units/second
- Smooth interpolation via move_smoothing_ factor
- Arrow keys for keyboard look at k_keyboard_look_speed = 1.5
- Mouse look at k_mouse_sensitivity = 0.0025 radians per pixel
- Pitch clamped to [-90°, 90°]

**Output**:
- glm::mat4 view matrix via get_view_matrix()
- Position and orientation accessible to renderer and debugger

## Post-Processing Module

- `include/engine/post_processor.h` (consolidated from `renderer/`)
- `src/engine/renderer/PostProcessor.cpp`

**Responsibilities**:
- Capture scene rendering to framebuffer object (FBO)
- Apply post-processing shader effects
- Support hot-reload of shader files
- Enable/disable effects and control effect modes

**Features**:
- Framebuffer capture with depth RBO
- GLSL shader compilation with error reporting
- Watched shader files for hot-reload detection
- Effect modes: none (0), grayscale (1), invert (2), sepia (3)
- Integrated with ImGui debugger for runtime control

**Status**: Currently disabled in main loop (toggle via debugger)

## Debug UI (ImGui Integration)

- `include/engine/debug_imgui.h` (consolidated from `debug/`)
- `src/engine/debug/ImGuiDebugger.cpp`

**Responsibilities**:
- Initialize ImGui with OpenGL3 backend
- Record and display SDL events for debugging
- Show performance metrics and camera state
- Provide in-game console for runtime commands
- Control post-processor and rendering toggles

**Features**:
- Real-time FPS and frame timing display
- Per-stage performance measurements
- Camera position/rotation display
- Depth test and face culling toggles
- Post-processor shader control and hot-reload UI
- Event history viewer (SDL events)
- Console with command execution (quit, set_smoothing, help)

## Asset Manager

- `include/engine/asset_manager.h`
- `src/engine/AssetManager.cpp` (currently disabled for debugging)

**Responsibilities**:
- Cache and manage textures and 3D models
- Load textures from disk (BMP, PNG/JPG with SDL2_image)
- Load 3D models (OBJ support with tinyobjloader)
- Prevent duplicate resource loads via weak pointer caching

**Status**: Disabled pending resource system refactoring

## Scene Management

- `include/engine/scene/scene.h`
- `include/engine/scene/scene_manager.h`
- `src/engine/scene/SceneManager.cpp`

**Responsibilities**:
- Manage active scene lifecycle
- Update scene entities and logic per frame
- Render scene through provided renderer

**Available Scenes**:
- `BlockWorldScene` - Voxel/block-based terrain demo (currently active)
- `DefaultScene` - Basic geometry demo
- Custom scenes can extend `Scene` base class

## Time System

- `include/engine/time.h`
- `src/engine/Time.cpp`

**Responsibilities**:
- Track uptime since engine start
- Count frames in current measurement interval
- Provide frame count reset for FPS calculation

**Integration**:
- Called by Engine::run() each frame
- Provides uptime for debugger display
- Frame count used for FPS calculation

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

