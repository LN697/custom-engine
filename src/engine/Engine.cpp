#include "engine/engine.h"
#include "engine/input/minecraft_control_scheme.h"
#include "engine/input/fps_control_scheme.h"
#include "engine/input/doom_control_scheme.h"
#include "engine/scene/block_world_scene.h"

namespace engine {

static constexpr float k_debug_update_interval = 0.5f;

Engine::Engine()
        : is_running_(false),
          window_width_(1280),
          window_height_(720),
          window_(nullptr),
          gl_context_(nullptr),
          renderer_(),
          post_processor_(),
          debugger_(),
          camera_(),
          scene_manager_(),
          time_(),
          current_control_scheme_(nullptr),
          debug_update_interval_(0.0f) {
}

Engine::~Engine() {
    shutdown();
}

bool Engine::initialize() {
    // Do not forcibly restrict the video driver here; allow the environment or SDL to choose the best available driver (helps headless runs).
    SDL_SetHint(SDL_HINT_MOUSE_RELATIVE_MODE_WARP, "0");

    if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_EVENTS | SDL_INIT_TIMER) != 0) {
        std::cerr << "[ENGINE] SDL_Init failed: " << SDL_GetError() << "\n";
        return false;
    }

    if (!create_window()) return false;
    if (!renderer_.initialize(window_width_, window_height_)) return false;

    // Optional post processor
    // post_processor_.initialize(window_width_, window_height_);
    //TODO: hot-reloading of shader files
    // post_processor_.load_shader_files("shaders/post_default.vert", "shaders/post_default.frag");

    if (!debugger_.initialize(window_, gl_context_)) {
        std::cerr << "[ENGINE] ImGui debugger initialization failed\n";
        return false;
    }
    // debugger_.set_post_processor(&post_processor_);
    debugger_.attach_movement_smoothing(camera_.smoothing_ptr());
    // Debugger CLI setup
    debugger_.set_command_callback([this](const std::string& cmd) {
        if (cmd == "quit" || cmd == "exit") {
            debugger_.add_console_log("[ENGINE] Quitting application...");
            is_running_ = false;
            return;
        }

        if (cmd.rfind("set_smoothing ", 0) == 0) {
            std::istringstream ss(cmd.substr(14));
            float v = 0.0f;
            if (ss >> v) {
                camera_.set_smoothing(v);
                debugger_.add_console_log("Movement smoothing set.");
            } else {
                debugger_.add_console_log("Invalid value for set_smoothing");
            }
            return;
        }

        // if (cmd == "toggle_postprocess") {
        //     post_processor_.set_enabled(!post_processor_.is_enabled());
        //     debugger_.add_console_log(post_processor_.is_enabled() ? "Postprocess enabled" : "Postprocess disabled");
        //     return;
        // }

        if (cmd == "help") {
            debugger_.add_console_log("Commands: help, quit, set_smoothing <value>, toggle_postprocess");
            return;
        }

        debugger_.add_console_log(std::string("Unknown command: ") + cmd);
    });

    // Initialize default control scheme (Minecraft-style)
    current_control_scheme_ = std::make_shared<MinecraftControlScheme>();

    // Load the block-world demo scene (terrain builder + wireframe)
    scene_manager_.set_scene(std::make_unique<scene::BlockWorldScene>());
    
    // Set the control scheme on the current scene
    if (auto current_scene = scene_manager_.get_current_scene()) {
        current_scene->set_control_scheme(current_control_scheme_);
    }

    SDL_ShowWindow(window_);
    SDL_RaiseWindow(window_);

    const char* driver = SDL_GetCurrentVideoDriver();
    if (driver && std::string(driver) == "x11") {
        SDL_SetWindowGrab(window_, SDL_TRUE);
    }

    is_running_ = true;
    return true;
}

void Engine::handle_event(const SDL_Event& event) {
    // For debugger event list
    debugger_.record_event(event);

    // Pass events to ImGui EXCEPT arrow keys and escape (game controls)
    if (event.type == SDL_KEYDOWN || event.type == SDL_KEYUP) {
        if (event.key.keysym.sym != SDLK_UP && event.key.keysym.sym != SDLK_DOWN &&
            event.key.keysym.sym != SDLK_LEFT && event.key.keysym.sym != SDLK_RIGHT &&
            event.key.keysym.sym != SDLK_ESCAPE) {
            debugger_.process_event(event);
        }
    } else {
        debugger_.process_event(event);
    }

    input_manager_.handle_event(event);
    
    // Check for quit event
    if (event.type == SDL_QUIT) {
        is_running_ = false;
        return;
    }

    if (event.type == SDL_WINDOWEVENT && event.window.event == SDL_WINDOWEVENT_SIZE_CHANGED) {
        window_width_ = event.window.data1;
        window_height_ = event.window.data2;
        renderer_.resize(window_width_, window_height_);
        // post_processor_.resize(window_width_, window_height_);
    }

    // Request the pointer lock ONLY when the user clicks inside the active window.
    if (event.type == SDL_MOUSEBUTTONDOWN) {
        if (SDL_GetRelativeMouseMode() == SDL_FALSE) {
            SDL_SetRelativeMouseMode(SDL_TRUE);
            // Flush the synthetic jump that occurs the exact moment the cursor hides
            SDL_GetRelativeMouseState(nullptr, nullptr);
        }
    }

    // Allow Escape to disable mouse capture
    if (event.type == SDL_KEYDOWN && event.key.keysym.sym == SDLK_ESCAPE) {
        if (SDL_GetRelativeMouseMode() == SDL_TRUE) {
            SDL_SetRelativeMouseMode(SDL_FALSE);
        }
    }
}

void Engine::run() {
    Uint64 last_ticks = SDL_GetPerformanceCounter();
    const double frequency = static_cast<double>(SDL_GetPerformanceFrequency());

    while (is_running_) {
        Uint64 current_ticks = SDL_GetPerformanceCounter();
        float delta_time = static_cast<float>((current_ticks - last_ticks) / frequency);
        last_ticks = current_ticks;

        time_.tick(delta_time);
        time_.increment_frame();

        // Stage timing: measure durations of key stages to help find bottlenecks
        Uint64 s0 = SDL_GetPerformanceCounter();

        SDL_Event event;
        int event_count = 0;
        while (SDL_PollEvent(&event)) {
            handle_event(event);
            ++event_count;
            if (event_count > 1000) {
                debugger_.add_console_log("Event queue flushed (exceeded 1000 events)");
                SDL_FlushEvents(SDL_FIRSTEVENT, SDL_LASTEVENT);
                break;
            }
        }
        Uint64 s1 = SDL_GetPerformanceCounter();

        // Update input keyboard state after processing events so camera sees current keys
        input_manager_.update();

        // If input manager requested quit (eg. Q pressed), stop running
        if (input_manager_.is_quit_requested()) {
            is_running_ = false;
            break;
        }

        debugger_.begin_frame();
        Uint64 s2 = SDL_GetPerformanceCounter();

        // Update camera and scene
        update_camera(delta_time);
        Uint64 s3 = SDL_GetPerformanceCounter();
        scene_manager_.update(delta_time);
        Uint64 s4 = SDL_GetPerformanceCounter();

        Uint64 s5 = s4;
        Uint64 s6 = s4;
        if (post_processor_.is_enabled()) {
            post_processor_.begin_capture();
            scene_manager_.render(renderer_, camera_);
            post_processor_.end_capture();
            s5 = SDL_GetPerformanceCounter();
            s6 = SDL_GetPerformanceCounter();
        } else {
            scene_manager_.render(renderer_, camera_);
            s5 = SDL_GetPerformanceCounter();
            s6 = s5;
        }

        // Accumulate debug interval
        debug_update_interval_ += delta_time;

        // Update debug statistics at a lower frequency to avoid jitter
        if (debug_update_interval_ >= k_debug_update_interval) {
            int frames = time_.get_frame_count_and_reset();
            float fps = (debug_update_interval_ > 0.0f && frames > 0) ? static_cast<float>(frames) / debug_update_interval_ : 0.0f;
            update_debug_ui(fps, frames, time_.uptime());
            debug_update_interval_ = 0.0f;
        }

        // Draw post-processed scene (if enabled) before UI so UI overlays on top
        Uint64 s7 = s6;
        if (post_processor_.is_enabled()) {
            post_processor_.check_for_shader_reload();
            post_processor_.render();
            s7 = SDL_GetPerformanceCounter();
        }

        // Collect stage timings (in milliseconds)
        std::vector<std::pair<std::string, float>> stage_timings;
        stage_timings.emplace_back("events", static_cast<float>((s1 - s0) / frequency * 1000.0));
        stage_timings.emplace_back("imgui_begin", static_cast<float>((s2 - s1) / frequency * 1000.0));
        stage_timings.emplace_back("camera_update", static_cast<float>((s3 - s2) / frequency * 1000.0));
        stage_timings.emplace_back("scene_update", static_cast<float>((s4 - s3) / frequency * 1000.0));
        stage_timings.emplace_back("scene_render", static_cast<float>((s5 - s4) / frequency * 1000.0));
        stage_timings.emplace_back("post_capture", static_cast<float>((s6 - s5) / frequency * 1000.0));
        stage_timings.emplace_back("post_render", static_cast<float>((s7 - s6) / frequency * 1000.0));

        // Push timings to the debugger for display
        debugger_.set_stage_timings(stage_timings);

        // Disable ImGui mouse capture if mouse is in game area (center region)
        ImGuiIO& io = ImGui::GetIO();
        int mouse_x = io.MousePos.x;
        int mouse_y = io.MousePos.y;
        // If mouse is in center game area (not in left/right UI panels), don't capture
        if (mouse_x > 310 && mouse_y > 10) {
            io.ConfigFlags &= ~ImGuiConfigFlags_NavEnableKeyboard;
            io.WantCaptureMouse = false;
            io.WantCaptureKeyboard = false;
        } else {
            io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
        }

        // Always draw the UI (it uses the latest cached stats)
        debugger_.draw_ui();

        // End ImGui frame and render
        debugger_.end_frame();
        Uint64 s8 = SDL_GetPerformanceCounter();
        debugger_.render();
        Uint64 s9 = SDL_GetPerformanceCounter();

        SDL_GL_SwapWindow(window_);
        Uint64 s10 = SDL_GetPerformanceCounter();

        // Add swap/render timings to the debugger (append)
        std::vector<std::pair<std::string, float>> more_times;
        more_times.emplace_back("imgui_render", static_cast<float>((s9 - s8) / frequency * 1000.0));
        more_times.emplace_back("swap", static_cast<float>((s10 - s9) / frequency * 1000.0));
        // merge
        auto current = std::move(stage_timings);
        current.insert(current.end(), more_times.begin(), more_times.end());
        debugger_.set_stage_timings(current);

        // input_manager_.update() already called earlier after event polling
    }
}

void Engine::shutdown() {
    debugger_.shutdown();
    
    if (gl_context_) {
        SDL_GL_DeleteContext(gl_context_);
        gl_context_ = nullptr;
    }

    if (window_) {
        SDL_DestroyWindow(window_);
        window_ = nullptr;
    }

    SDL_Quit();
}

bool Engine::create_window() {
    // Request OpenGL 4.3 compatibility profile to support both modern and legacy rendering
    // (Legacy immediate mode until VAO/VBO rendering is fully implemented)
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 4);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 3);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_COMPATIBILITY);
    SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);

    window_ = SDL_CreateWindow(
        "Custom Engine with OpenGL 4.3",
        SDL_WINDOWPOS_CENTERED,
        SDL_WINDOWPOS_CENTERED,
        window_width_,
        window_height_,
        SDL_WINDOW_OPENGL | SDL_WINDOW_RESIZABLE | SDL_WINDOW_SHOWN
    );

    if (!window_) {
        std::cerr << "[ENGINE] SDL_CreateWindow failed: " << SDL_GetError() << "\n";
        return false;
    }

    gl_context_ = SDL_GL_CreateContext(window_);
    if (!gl_context_) {
        std::cerr << "[ENGINE] SDL_GL_CreateContext failed: " << SDL_GetError() << "\n";
        return false;
    }

    if (SDL_GL_SetSwapInterval(1) != 0) {
        // 1  == vsync, 0  == immediate, -1 == adaptive vsync
        std::cerr << "[ENGINE] Warning: Unable to enable VSync: " << SDL_GetError() << "\n";
    }

    return true;
}

void Engine::update_camera(float delta_time) {
    if (current_control_scheme_) {
        current_control_scheme_->update(input_manager_, delta_time);
        camera_.update(delta_time, *current_control_scheme_);
    } else {
        camera_.update(delta_time, input_manager_);
    }
}

void Engine::set_scene_control_scheme(std::shared_ptr<ControlScheme> control_scheme) {
    current_control_scheme_ = control_scheme;
    
    // Also set it on the current scene
    if (auto current_scene = scene_manager_.get_current_scene()) {
        current_scene->set_control_scheme(control_scheme);
    }
}


void Engine::update_debug_ui(float fps, int frame_count, float uptime) {
    const char* gl_version = reinterpret_cast<const char*>(glGetString(GL_VERSION));
    const char* gl_renderer = reinterpret_cast<const char*>(glGetString(GL_RENDERER));
    const bool depth_enabled = glIsEnabled(GL_DEPTH_TEST) == GL_TRUE;

    // Update debugger with frame data (cached until next interval)
    debugger_.set_camera_position(camera_.x(), camera_.y(), camera_.z());
    debugger_.set_camera_rotation(camera_.yaw(), camera_.pitch());
    debugger_.set_frame_stats(fps, frame_count, uptime);
    debugger_.set_gl_info(gl_version, gl_renderer);
    debugger_.set_depth_test_enabled(depth_enabled);
}

} // namespace engine