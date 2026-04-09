# Engine Architecture (Post-Refactoring)

The custom engine is organized into modular subsystems with consolidated file structure and snake_case naming conventions.

## Directory Structure (Refactored)

```
include/engine/
├── engine.h              # Main engine class
├── time.h                # Timing system
├── asset_manager.h       # Asset loading
├── input_manager.h       # Input (consolidated from input/)
├── camera.h              # Camera (consolidated from camera/)
├── debug_imgui.h         # Debug UI (consolidated from debug/)
├── renderer.h            # OpenGL rendering
├── post_processor.h      # Post-processing effects
├── ecs/
│   ├── registry.h        # ECS entity registry
│   └── components.h      # Component definitions
├── scene/
│   ├── scene.h
│   ├── default_scene.h
│   └── scene_manager.h
└── graphics/
    ├── geometry_buffer.h
    └── shader_program.h
```

## Consolidation Changes

- **Removed nested directories:** `input/`, `camera/`, `debug/` merged to root `engine/` level
- **Kept modular structure:** `ecs/`, `scene/`, `graphics/` retained for organization
- **Benefit:** Flatter hierarchy for single-unit modules without sacrificing organization

## Core Subsystems

### Main Engine (`src/engine/engine.cpp`)
- Application lifecycle (initialize, run, shutdown)
- Window creation and management
- Main frame loop
- Event dispatch to subsystems

### Input Management (`src/engine/input_manager.cpp`)
- SDL event polling and processing
- Keyboard/mouse state tracking
- Quit request handling

### Camera System (`src/engine/camera.cpp`)
- First-person camera with smooth movement
- View matrix generation
- WASD/mouse look controls

### Rendering System (`src/engine/renderer.cpp`)
- OpenGL initialization and viewport management
- Frame buffer clear and setup
- Delegate rendering to scene

### Scene Management (`src/engine/scene/scene_manager.cpp`)
- Scene lifecycle management
- Entity/component update and render
- Grid and triangle demo geometry

### ECS System (`src/engine/ecs/registry.cpp`)
- Entity creation/destruction
- Component storage
- View iteration for efficient processing

### Post-Processing (`src/engine/post_processor.cpp`)
- Post-render effects pipeline
- Depth of field, bloom, etc.

## Naming Conventions (Applied Post-Refactoring)

| Item | Convention | Example |
|------|-----------|---------|
| Files | snake_case | `input_manager.h`, `debug_imgui.cpp` |
| Classes | PascalCase | `Engine`, `Camera`, `InputManager` |
| Functions | snake_case | `handle_event()`, `get_view_matrix()` |
| Variables | snake_case_suffix | `input_manager_`, `is_running_` |
| Constants | k_snake_case | `k_movement_speed`, `k_mouse_sensitivity` |
| GL Pointers | gl_name | `gl_bind_vertex_array`, `gl_draw_elements` |

## Execution Flow

```
main()
  ↓
Engine::initialize()
  ├─ create_window()
  ├─ InputManager setup
  ├─ OpenGLRenderer setup
  ├─ ImGuiDebugger setup
  └─ SceneManager setup
  ↓
Engine::run() [Main Loop]
  ├─ handle_event()
  ├─ update_camera(delta_time)
  ├─ scene_manager_->update()
  ├─ renderer_->render()
  ├─ debugger_->draw_ui()
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
