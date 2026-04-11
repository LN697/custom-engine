# Custom Engine Component Notes

## main.cpp
**Purpose**: Application entry point

**Key Components**:
- `engine::Engine` - Main engine instance
- Initialization: Creates engine, calls `initialize()`, starts main loop with `run()`, cleans up with `shutdown()`
- Error handling: Returns 1 on initialization failure, 0 on success

**Execution Flow**:
```
main()
  → Engine::initialize()
  → Engine::run()
  → Engine::shutdown()
```

---

## engine.h/cpp
**Purpose**: Core engine class managing application lifecycle and main loop

**Member Variables**:
- `is_running_` - Loop control flag
- `window_width_`, `window_height_` - Window dimensions (1280x720 default)
- `window_` - SDL2 window pointer
- `gl_context_` - OpenGL context
- `input_manager_` - Handles keyboard/mouse input
- `renderer_` - OpenGL rendering subsystem
- `post_processor_` - Post-processing effects (currently disabled)
- `debugger_` - ImGui debug UI and telemetry
- `camera_` - First-person camera
- `scene_manager_` - Scene lifecycle and rendering
- `time_` - Timing and frame counting
- `debug_update_interval_` - UI update frequency (0.5s)

**Key Methods**:

### initialize()
- Initializes SDL2 (SDL_INIT_VIDEO | SDL_INIT_EVENTS | SDL_INIT_TIMER)
- Creates window via `create_window()`
- Initializes: renderer, debugger, camera, scene manager
- Sets up debugger command callbacks (quit, set_smoothing, help)
- Loads BlockWorldScene demo
- Shows window and enables mouse grab on X11
- Sets `is_running_ = true`

### run()
- Main event loop controlled by `is_running_`
- **Timing**: Uses SDL_GetPerformanceCounter for high-precision delta_time calculation
- **Stage measurements**: Multi-point timing (s0-s7) for bottleneck analysis
- **Event processing**: SDL_PollEvent up to 1000 events per frame
- **Input update**: InputManager processes keyboard state after events
- **Camera update**: Updates first-person camera with delta_time
- **Scene update**: Updates scene entities/logic
- **Rendering**: Scene renders through renderer; optional post-processing
- **Debug UI**: ImGui panels drawn each frame
- **Frame timing**: Sends FPS/uptime to debugger
- Loop breaks on quit request (Q key or SDL_QUIT event)

### handle_event()
- Records events for debugger history
- Routes ImGui events (excluding arrow keys and Escape for game controls)
- Passes input to InputManager
- Handles SDL_QUIT event
- Handles window resize (updates dimensions, resizes renderer/post-processor)
- Manages relative mouse mode (click to grab, Escape to release)

### create_window()
- Creates SDL2 window with SDL_WINDOW_OPENGL | SDL_WINDOW_HIDDEN
- Sets OpenGL attributes: 3.3 core profile
- Returns false on failure

### shutdown()
- Cleanup via destructor

---

## renderer.h/cpp
**Purpose**: OpenGL rendering system

**Member Variables**:
- `width_`, `height_` - Framebuffer dimensions
- `aspect_ratio_` - Width/Height ratio

**Key Methods**:

### initialize(int width, int height)
- Sets up OpenGL 3.3 core context
- Initializes viewport, projection matrix
- Returns false on error

### shutdown()
- Releases GL resources

### resize(int width, int height)
- Updates viewport and aspect ratio when window resizes

### render(const Camera& camera)
- Main rendering function
- Clears viewport
- Delegates to `render_scene()` which renders current scene
- Uses camera view matrix for proper perspective

---

## input_manager.h/cpp
**Purpose**: Keyboard and mouse input handling

**Member Variables**:
- `keys_` - SDL keyboard state array
- `quit_requested_` - Q key pressed flag
- `num_keys_` - Total SDL scancodes

**Key Methods**:

### update()
- Called after event processing
- Queries SDL keyboard state via SDL_GetKeyboardState()

### handle_event(const SDL_Event& event)
- Processes SDL events
- Sets quit_requested_ on Q key (SDL_SCANCODE_Q)

**Accessor Methods**:
- `move_forward()` - W key (SDL_SCANCODE_W)
- `move_backward()` - S key (SDL_SCANCODE_S)
- `move_left()` - A key (SDL_SCANCODE_A)
- `move_right()` - D key (SDL_SCANCODE_D)
- `move_up()` - Space (SDL_SCANCODE_SPACE)
- `move_down()` - Left Ctrl (SDL_SCANCODE_LCTRL)
- `look_left()`, `look_right()`, `look_up()`, `look_down()` - Arrow keys

---

## camera.h/cpp
**Purpose**: First-person camera system with smooth movement

**Member Variables**:
- `position_` - World position (glm::vec3)
- `orientation_` - Quaternion for rotations
- `velocity_` - Current movement velocity
- `yaw_`, `pitch_` - Euler angles for rotation
- `move_smoothing_` - Interpolation factor for smooth movement (default varies)
- **Constants**:
  - `k_movement_speed` = 4.0f
  - `k_mouse_sensitivity` = 0.0025f
  - `k_keyboard_look_speed` = 1.5f

**Key Methods**:

### update(float delta_time, const InputManager& input)
- Updates position based on WASD input
- Applies movement smoothing (interpolation)
- Updates rotation from mouse motion and arrow keys
- Constrains pitch to [-90, 90] degrees
- Calls `get_view_matrix()` to update orientation

### get_view_matrix()
- Returns glm::mat4 view matrix for rendering
- Used in renderer to set camera perspective

### Accessors/Mutators**:
- `position()`, `orientation()` - Current state
- `x()`, `y()`, `z()` - Position components
- `yaw()`, `pitch()` - Rotation angles
- `set_position()`, `set_rotation()` - Direct setters
- `set_smoothing()`, `smoothing_ptr()` - Movement smoothing control

---

## post_processor.h/cpp
**Purpose**: Post-processing effects pipeline (currently mostly disabled)

**Member Variables**:
- `texture_`, `fbo_`, `depth_rbo_` - Framebuffer objects
- `shader_program_` - Compiled shader program
- `width_`, `height_` - Framebuffer dimensions
- `initialized_`, `enabled_` - State flags
- `effect_` - Effect mode (0=none, 1=grayscale, 2=invert, 3=sepia)
- `last_log_` - Error/compile messages
- `watched_vert_`, `watched_frag_` - Paths for hot-reload

**Key Methods**:

### initialize(int width, int height)
- Sets up FBO, texture, RBO for off-screen rendering
- Returns false on error

### shutdown()
- Releases GL resources

### load_shader_files(const string& vert_path, const string& frag_path)
- Loads and compiles GLSL shaders from files
- Enables hot-reload of shaders
- Returns false on compile error

### begin_capture()
- Binds FBO to redirect rendering to texture

### end_capture()
- Unbinds FBO, returns to default framebuffer

### render()
- Applies post-processing shader to captured scene
- Outputs to screen

### check_for_shader_reload()
- Watches shader files for changes
- Recompiles if modified (hot-reload)

**Status**: Mostly disabled in current build; can be re-enabled via ImGui debugger

---

## debug_imgui.h/cpp (ImGuiDebugger)
**Purpose**: In-game debug UI using ImGui with OpenGL3 backend

**Member Variables**:
- `window_`, `gl_context_` - SDL and OpenGL context
- `post_processor_` - Reference to post-processor for UI control
- `movement_smoothing_ptr_` - Pointer to camera smoothing value
- `command_callback_` - Lambda for command processing
- Camera state: `camera_pos_`, `camera_yaw_`, `camera_pitch_`
- Frame stats: `fps_`, `frame_count_`, `uptime_`
- OpenGL info: `gl_version_`, `gl_renderer_`
- Console log (vector of messages)
- Event history (SDL_Event recording)
- Stage timings (bottleneck analysis)

**Key Methods**:

### initialize(SDL_Window* window, SDL_GLContext context)
- Initializes ImGui context
- Sets up OpenGL3 backend
- Returns false on failure

### shutdown()
- Cleanup ImGui resources

### begin_frame()
- Starts ImGui frame

### draw_ui()
- Renders all debug panels:
  - Performance metrics (FPS, frame time, stage timings)
  - Camera state display
  - Rendering controls (depth test, face culling)
  - Post-processor controls
  - Command console with history
  - Event log viewer

### end_frame()
- Finalizes ImGui frame

### render()
- Renders ImGui draw data to screen

### process_event(const SDL_Event& event)
- Routes events to ImGui backend

### record_event(const SDL_Event& event)
- Stores events for event history display

### set_command_callback()
- Registers callback for console commands

### Command Support**:
- `quit` / `exit` - Stop engine
- `set_smoothing <value>` - Adjust camera movement smoothing
- `help` - Show available commands

---

## scene_manager.h/cpp
**Purpose**: Scene lifecycle and rendering management

**Member Variables**:
- `current_` - Current scene (std::unique_ptr<Scene>)

**Key Methods**:

### set_scene(std::unique_ptr<Scene> s)
- Changes active scene
- Old scene destroyed automatically

### update(float dt)
- Calls current scene's update method
- Passes delta_time for frame-independent logic

### render(OpenGLRenderer& renderer, const Camera& camera)
- Calls current scene's render method
- Passes renderer and camera for rendering

**Current Scene**: BlockWorldScene (voxel/block-based terrain demo)

---

## time.h/cpp
**Purpose**: Timing system for frame counting and uptime tracking

**Member Variables**:
- `uptime_` - Elapsed seconds since start
- `frame_count_` - Frames elapsed in current measuring period

**Key Methods**:

### tick(float delta)
- Accumulates delta_time to uptime_
- Called once per frame

### increment_frame()
- Increments frame_count_
- Called once per frame

### get_frame_count_and_reset()
- Returns accumulated frame_count_ and resets to 0
- Used by debugger to calculate FPS (frames per sampling interval)

### uptime()
- Returns total elapsed time in seconds since engine start
