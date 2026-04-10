# Engine Architecture

The custom engine follows a modular architecture organized by subsystems with snake_case file naming conventions and PascalCase class names.

## Directory Structure

```
include/engine/
├── engine.h              # Main engine class
├── time.h                # Timing system
├── input_manager.h       # Input handling (consolidated)
├── camera.h              # Camera system (consolidated)
├── renderer.h            # OpenGL rendering
├── post_processor.h      # Post-processing effects
├── debug_imgui.h         # ImGui debug UI (consolidated)
├── asset_manager.h       # Asset loading (currently disabled)
├── ecs/
│   ├── registry.h        # ECS entity registry
│   └── components.h      # Component definitions
├── scene/
│   ├── scene.h
│   ├── default_scene.h
│   ├── block_world_scene.h
│   └── scene_manager.h
├── graphics/
│   ├── shader_program.h
│   └── geometry_buffer.h
└── platform/
    └── (Future platform backends)

src/engine/
├── Engine.cpp
├── Time.cpp
├── camera/
│   └── Camera.cpp
├── debug/
│   └── ImGuiDebugger.cpp
├── input/
│   └── InputManager.cpp
├── renderer/
│   ├── OpenGLRenderer.cpp
│   └── PostProcessor.cpp
├── ecs/
│   ├── registry.cpp
│   └── components.cpp
└── scene/
    ├── SceneManager.cpp
    ├── DefaultScene.cpp
    └── BlockWorldScene.cpp
```

## Naming Conventions

| Item | Convention | Example |
|------|-----------|---------|
| Files | snake_case | `input_manager.h`, `debug_imgui.cpp` |
| Classes | PascalCase | `Engine`, `Camera`, `InputManager` |
| Functions | snake_case | `handle_event()`, `get_view_matrix()` |
| Variables | snake_case_suffix | `input_manager_`, `is_running_` |
| Constants | k_snake_case | `k_movement_speed`, `k_mouse_sensitivity` |
| GL Pointers | gl_prefix | `gl_bind_vertex_array`, `gl_draw_elements` |

## Core Subsystems

### Application Core
- **Engine**: Main orchestration, window management, main loop
- **Time**: Uptime tracking and frame counting

### Input & Camera
- **InputManager**: SDL event processing, keyboard/mouse state
- **Camera**: First-person camera with smooth movement, view matrix generation

### Rendering
- **OpenGLRenderer**: OpenGL 3.3 core context, viewport management
- **PostProcessor**: FBO-based post-processing with hot-reload shaders
- **ImGuiDebugger**: In-game debugging UI and telemetry

### Scene & World
- **SceneManager**: Scene lifecycle and update/render dispatch
- **Scene**: Base class for game world logic
- **BlockWorldScene**: Voxel terrain demo (current scene)
- **DefaultScene**: Basic geometry demo (alternative)

### ECS (Entity Component System)
- **Registry**: Entity and component storage
- **Components**: Component type definitions
- (Ready for EnTT integration or custom implementation)

## Execution Flow

```
main()
  │
  └─ Engine::initialize()
      ├─ SDL_Init(SDL_INIT_VIDEO | SDL_INIT_EVENTS | SDL_INIT_TIMER)
      ├─ create_window()
      ├─ InputManager setup
      ├─ OpenGLRenderer::initialize(1280, 720)
      ├─ ImGuiDebugger::initialize()
      ├─ Camera setup
      ├─ SceneManager setup (load BlockWorldScene)
      └─ SDL_ShowWindow()
  │
  └─ Engine::run() [Main Loop]
      │
      ├─ SDL_PollEvent loop (up to 1000 events)
      │   └─ handle_event()
      │       ├─ ImGuiDebugger::process_event()
      │       ├─ InputManager::handle_event()
      │       └─ Handle quit/resize
      │
      ├─ InputManager::update()
      │   └─ SDL_GetKeyboardState()
      │
      ├─ ImGuiDebugger::begin_frame()
      │
      ├─ Engine::update_camera(delta_time)
      │   └─ Camera::update(delta_time, input_manager)
      │
      ├─ SceneManager::update(delta_time)
      │   └─ Scene::update(delta_time)
      │
      ├─ Optional: PostProcessor::begin_capture()
      │
      ├─ SceneManager::render(renderer, camera)
      │   └─ Scene::render(renderer, camera)
      │
      ├─ Optional: PostProcessor::end_capture()
      ├─ Optional: PostProcessor::render()
      │
      ├─ ImGuiDebugger::draw_ui()
      │   └─ Show FPS, camera state, events, console
      │
      ├─ ImGuiDebugger::end_frame()
      │   └─ ImGui::Render()
      │
      └─ [Repeat while is_running_]
  │
  └─ Engine::shutdown()
      ├─ SDL_GL_DeleteContext()
      ├─ SDL_DestroyWindow()
      └─ SDL_Quit()
```

## Frame Timing & Performance Analysis

The engine uses **high-precision timing** for frame-independent logic:

```cpp
delta_time = (current_ticks - last_ticks) / SDL_GetPerformanceFrequency()
```

**Stage Measurements** (for bottleneck analysis):
- **s0-s1**: Event processing
- **s1-s2**: ImGui begin_frame
- **s2-s3**: Camera update
- **s3-s4**: Scene update
- **s4-s5**: Scene rendering
- **s5-s6**: Post-processing
- **s6-s7**: ImGui rendering

## Key Design Patterns

### Dependency Injection
Subsystems receive references to dependencies:
- Camera receives InputManager for input queries
- Renderer receives Camera for view matrix
- Scene receives Renderer for drawing

### Lazy Initialization
Subsystems mark initialized state and can be safely shutdown/reinitialized

### Modular Events
Events can be processed independently:
- ImGui gets non-game events (Q, Escape reserved for game)
- InputManager gets all events
- Engine core handles window/quit events

### Hot-Reloading
Post-processor watches shader files and recompiles on changes without engine restart

## Thread Safety

- Currently single-threaded design
- EventBus supports multi-threaded publish/subscribe (ready for worker threads)

## Memory Management

- Smart pointers for scene ownership (std::unique_ptr)
- Stack allocation for subsystems where possible
- No dynamic allocation in hot paths
- SDL2 handles window/context memory

## Platform Support

- **Current**: Linux (SDL2 + OpenGL)
- **Extensible**: Platform backends can be added under `src/engine/platform/`
- **Future**: Windows, macOS support via platform abstraction layer
  └─ swap_buffers()
  ↓
Engine::shutdown()
  ├─ ImGuiDebugger::shutdown()
  ├─ OpenGLRenderer::shutdown()
  └─ cleanup all resources
```

## Module Interplay

**Engine** → coordinates all subsystems
  ├─ **InputManager** → event handling and key state
  ├─ **Camera** → first-person controls with input
  ├─ **SceneManager** → delegates update/render to active scene
  │   └─ **Scene** → ECS update and drawing
  │       ├─ **Registry** → component storage
  │       ├─ **Renderer** → OpenGL drawing
  │       └─ **ShaderProgram** → mesh coloring and effects
  ├─ **PostProcessor** → screen effects post-render
  ├─ **ImGuiDebugger** → UI overlay and performance data
  └─ **Time** → delta time calculation

## Design Principles

- **Modularity:** Each subsystem has a single responsibility
- **Consolidation:** Reduced nesting for simpler include paths
- **Naming clarity:** snake_case conventions improve readability
- **ECS flexibility:** Component-based architecture for scene composition
- **Extensibility:** Easy to add new components and systems

---
**Updated:** Post-refactoring (snake_case + consolidation)
**Status:** Architecture finalized, implementation proceeding
