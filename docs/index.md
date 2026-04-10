# Custom Engine Documentation

Welcome to the custom 3D engine documentation. This repository contains a modular OpenGL-based rendering engine built with C++17, designed for extensibility and educational purposes.

## Engine Overview

The custom engine provides:
- **SDL2 integration** - Cross-platform window management and input
- **OpenGL 3.3 core rendering** - Modern graphics pipeline
- **First-person camera** - Smooth movement with mouse/keyboard controls
- **Modular architecture** - Decoupled subsystems for rendering, input, debugging, and scene management
- **ImGui integration** - In-game debug UI with real-time telemetry
- **Post-processing pipeline** - Shader effects with hot-reload support
- **Scene management** - Base classes for creating custom game worlds
- **ECS readiness** - Entity/component system placeholders for future expansion

## Quick Start

New to the project? Start here:

1. **[Getting Started](getting_started.md)** — Installation, build, and first run
   - Prerequisites and dependencies
   - Build instructions
   - Running the sample application
   - Control scheme

2. **[Dependencies](dependencies.md)** — Library requirements
   - Core vs. optional libraries
   - Linux installation instructions
   - Package discovery details

## Detailed Documentation

For deeper understanding of the engine:

3. **[Architecture](architecture.md)** — Design and execution flow
   - Directory structure
   - Naming conventions
   - Core subsystem design
   - Complete execution flow with timing analysis
   - Key design patterns

4. **[Modules](modules.md)** — Component responsibilities
   - Engine module (window, main loop)
   - Renderer (OpenGL 3.3)
   - Input manager (keyboard/mouse)
   - Camera system (first-person)
   - Post-processing (FBO, shaders)
   - Debug UI (ImGui telemetry)
   - Scene management
   - Time system
   - Asset manager (disabled)
   - ECS and event bus

## Goals & Philosophy

- **Modularity**: Keep subsystems isolated and independently testable
- **Clarity**: Well-documented code and clear architectural boundaries
- **Extensibility**: Easy to add new scenes, components, and features
- **Performance**: High-precision timing, stage-based bottleneck analysis
- **Learning**: Suitable for understanding engine design patterns

## Project Structure

- `src/` — Implementation files
- `include/engine/` — Public headers
- `shaders/` — GLSL vertex and fragment shaders
- `docs/` — Documentation (you are here)
- `Makefile` — Build automation
- `imgui.ini` — ImGui saved state

## Working with the Engine

### Building from Source
```bash
make              # Build
make clean        # Remove artifacts
make run          # Build and run
```

### Modifying the Code
- Add new subsystems under `src/engine/` (follow snake_case naming)
- Create scene types by extending `Scene` base class
- Use `ImGuiDebugger` for in-game debugging panels
- Recompile with `make` (incremental builds supported)

### Extending Functionality
- **New scenes**: Add to `src/engine/scene/`
- **New components**: Update `include/engine/ecs/components.h`
- **Custom effects**: Add post-processing shaders to `shaders/`
- **New subsystems**: Follow module pattern (header in `include/engine/`, implementation in `src/engine/`)

## Architecture Highlights

The engine follows a **stage-based execution model**:

1. **Events** — Process SDL events (input, window)
2. **Update** — Update camera and scene state
3. **Render** — Render scene through renderer
4. **Post-Process** — Apply effects (optional)
5. **Debug UI** — Display ImGui panels and telemetry

Each stage is timed for performance analysis via the debug UI.

## Next Steps

- Read **[Getting Started](getting_started.md)** to build and run the project
- Explore **[Architecture](architecture.md)** for system design details
- Check **[Modules](modules.md)** for individual component documentation
- Look at the `notes` file for detailed code-level documentation

## Support

For questions about the engine design or implementation, refer to:
- The comprehensive `notes` file for detailed component documentation
- Architecture diagrams and flow charts in `docs/architecture.md`
- Well-commented source code in `src/` and `include/`
