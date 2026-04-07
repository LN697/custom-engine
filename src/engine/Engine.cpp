#include "engine/Engine.h"
#include "engine/debug/ImGuiDebugger.h"
#include "engine/renderer/PostProcessor.h"
#include "engine/scene/DefaultScene.h"
#include <backends/imgui_impl_sdl2.h>
#include <SDL2/SDL.h>
#include <GL/gl.h>
#include <cmath>
#include <iostream>
#include <sstream>
#include <cstdlib>
#include <vector>
#include <utility>

namespace engine {

static constexpr float kDebugUpdateInterval = 0.25f;

Engine::Engine()
        : running_(false),
          width_(1280),
          height_(720),
          window_(nullptr),
          glContext_(nullptr),
          renderer_(),
          postProcessor_(),
          debugger_(),
          camera_(),
          sceneManager_(),
          time_(),
          debugUpdateInterval_(0.0f) {
}

Engine::~Engine() {
    shutdown();
}

bool Engine::initialize() {
    // Do not forcibly restrict the video driver here; allow the environment
    // or SDL to choose the best available driver (helps headless runs).
    SDL_SetHint(SDL_HINT_MOUSE_RELATIVE_MODE_WARP, "0");

    if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_EVENTS | SDL_INIT_TIMER) != 0) {
        std::cerr << "SDL_Init failed: " << SDL_GetError() << "\n";
        return false;
    }

    if (!createWindow()) return false;
    if (!renderer_.initialize(width_, height_)) return false;

    // Initialize post-processor (optional)
    postProcessor_.initialize(width_, height_);

    // Initialize ImGui debugger
    if (!debugger_.initialize(window_, glContext_)) {
        std::cerr << "ImGui debugger initialization failed\n";
        return false;
    }
    debugger_.setPostProcessor(&postProcessor_);
    debugger_.attachMovementSmoothing(camera_.smoothingPtr());

    // Attach a simple command handler for the in-game console
    debugger_.setCommandCallback([this](const std::string& cmd) {
        if (cmd == "quit" || cmd == "exit") {
            debugger_.addConsoleLog("Quitting application...");
            running_ = false;
            return;
        }

        if (cmd.rfind("set_smoothing ", 0) == 0) {
            std::istringstream ss(cmd.substr(14));
            float v = 0.0f;
            if (ss >> v) {
                camera_.setSmoothing(v);
                debugger_.addConsoleLog("Movement smoothing set.");
            } else {
                debugger_.addConsoleLog("Invalid value for set_smoothing");
            }
            return;
        }

        if (cmd == "toggle_postprocess") {
            postProcessor_.setEnabled(!postProcessor_.isEnabled());
            debugger_.addConsoleLog(postProcessor_.isEnabled() ? "Postprocess enabled" : "Postprocess disabled");
            return;
        }

        if (cmd == "help") {
            debugger_.addConsoleLog("Commands: help, quit, set_smoothing <value>, toggle_postprocess");
            return;
        }

        debugger_.addConsoleLog(std::string("Unknown command: ") + cmd);
    });

    // Create and set the default scene
    sceneManager_.setScene(std::make_unique<scene::DefaultScene>());

    SDL_ShowWindow(window_);
    SDL_RaiseWindow(window_);

    const char* driver = SDL_GetCurrentVideoDriver();
    if (driver && std::string(driver) == "x11") {
        SDL_SetWindowGrab(window_, SDL_TRUE);
    }

    // FIX: Removed SDL_SetRelativeMouseMode() from here. 
    // Wayland will silently reject locks on startup without user interaction.

    running_ = true;
    return true;
}

void Engine::handleEvent(const SDL_Event& event) {
    // Pass events to ImGui for handling
    ImGui_ImplSDL2_ProcessEvent(&event);
    
    if (inputManager_.processEvent(event)) {
        running_ = false;
        return;
    }

    if (event.type == SDL_WINDOWEVENT && event.window.event == SDL_WINDOWEVENT_SIZE_CHANGED) {
        width_ = event.window.data1;
        height_ = event.window.data2;
        renderer_.resize(width_, height_);
        postProcessor_.resize(width_, height_);
    }

    // Request the pointer lock ONLY when the user clicks inside the active window.
    if (event.type == SDL_MOUSEBUTTONDOWN) {
        if (SDL_GetRelativeMouseMode() == SDL_FALSE) {
            SDL_SetRelativeMouseMode(SDL_TRUE);
            // Flush the synthetic jump that occurs the exact moment the cursor hides
            SDL_GetRelativeMouseState(nullptr, nullptr);
        }
    }
}

void Engine::run() {
    Uint64 lastTicks = SDL_GetPerformanceCounter();
    const double frequency = static_cast<double>(SDL_GetPerformanceFrequency());

    while (running_) {
        Uint64 currentTicks = SDL_GetPerformanceCounter();
        float deltaTime = static_cast<float>((currentTicks - lastTicks) / frequency);
        lastTicks = currentTicks;

        // advance time
        time_.tick(deltaTime);
        time_.incrementFrame();

        // Stage timing: measure durations of key stages to help find bottlenecks
        Uint64 s0 = SDL_GetPerformanceCounter();

        SDL_Event event;
        while (SDL_PollEvent(&event)) {
            handleEvent(event);
        }
        Uint64 s1 = SDL_GetPerformanceCounter();

        debugger_.beginFrame();
        Uint64 s2 = SDL_GetPerformanceCounter();

        // Update camera and scene
        updateCamera(deltaTime);
        Uint64 s3 = SDL_GetPerformanceCounter();
        sceneManager_.update(deltaTime);
        Uint64 s4 = SDL_GetPerformanceCounter();
        sceneManager_.render(renderer_, camera_);
        Uint64 s5 = SDL_GetPerformanceCounter();


        // Capture the rendered scene for post-processing (before any UI is drawn)
        Uint64 s6 = s5;
        if (postProcessor_.isEnabled()) {
            postProcessor_.capture();
            s6 = SDL_GetPerformanceCounter();
        }

        // Accumulate debug interval
        debugUpdateInterval_ += deltaTime;

        // Update debug statistics at a lower frequency to avoid jitter
        if (debugUpdateInterval_ >= kDebugUpdateInterval) {
            int frames = time_.getFrameCountAndReset();
            float fps = (debugUpdateInterval_ > 0.0f && frames > 0) ? static_cast<float>(frames) / debugUpdateInterval_ : 0.0f;
            updateDebugUI(fps, frames, time_.uptime());
            debugUpdateInterval_ = 0.0f;
        }

        // Draw post-processed scene (if enabled) before UI so UI overlays on top
        Uint64 s7 = s6;
        if (postProcessor_.isEnabled()) {
            postProcessor_.render();
            s7 = SDL_GetPerformanceCounter();
        }

        // Collect stage timings (in milliseconds)
        std::vector<std::pair<std::string, float>> stageTimings;
        stageTimings.emplace_back("events", static_cast<float>((s1 - s0) / frequency * 1000.0));
        stageTimings.emplace_back("imgui_begin", static_cast<float>((s2 - s1) / frequency * 1000.0));
        stageTimings.emplace_back("camera_update", static_cast<float>((s3 - s2) / frequency * 1000.0));
        stageTimings.emplace_back("scene_update", static_cast<float>((s4 - s3) / frequency * 1000.0));
        stageTimings.emplace_back("scene_render", static_cast<float>((s5 - s4) / frequency * 1000.0));
        stageTimings.emplace_back("post_capture", static_cast<float>((s6 - s5) / frequency * 1000.0));
        stageTimings.emplace_back("post_render", static_cast<float>((s7 - s6) / frequency * 1000.0));

        // Push timings to the debugger for display
        debugger_.setStageTimings(stageTimings);

        // Always draw the UI (it uses the latest cached stats)
        debugger_.drawUI();

        // End ImGui frame and render
        debugger_.endFrame();
        Uint64 s8 = SDL_GetPerformanceCounter();
        debugger_.render();
        Uint64 s9 = SDL_GetPerformanceCounter();

        SDL_GL_SwapWindow(window_);
        Uint64 s10 = SDL_GetPerformanceCounter();

        // Add swap/render timings to the debugger (append)
        std::vector<std::pair<std::string, float>> moreTimes;
        moreTimes.emplace_back("imgui_render", static_cast<float>((s9 - s8) / frequency * 1000.0));
        moreTimes.emplace_back("swap", static_cast<float>((s10 - s9) / frequency * 1000.0));
        // merge
        auto current = std::move(stageTimings);
        current.insert(current.end(), moreTimes.begin(), moreTimes.end());
        debugger_.setStageTimings(current);

        inputManager_.resetFrameState();
    }
}

void Engine::shutdown() {
    debugger_.shutdown();
    
    if (glContext_) {
        SDL_GL_DeleteContext(glContext_);
        glContext_ = nullptr;
    }

    if (window_) {
        SDL_DestroyWindow(window_);
        window_ = nullptr;
    }

    SDL_Quit();
}

bool Engine::createWindow() {
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 2);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 1);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_COMPATIBILITY);
    SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);

    window_ = SDL_CreateWindow(
        "Custom Engine",
        SDL_WINDOWPOS_CENTERED,
        SDL_WINDOWPOS_CENTERED,
        width_,
        height_,
        SDL_WINDOW_OPENGL | SDL_WINDOW_RESIZABLE | SDL_WINDOW_SHOWN
    );

    if (!window_) {
        std::cerr << "SDL_CreateWindow failed: " << SDL_GetError() << "\n";
        return false;
    }

    glContext_ = SDL_GL_CreateContext(window_);
    if (!glContext_) {
        std::cerr << "SDL_GL_CreateContext failed: " << SDL_GetError() << "\n";
        return false;
    }

    if (SDL_GL_SetSwapInterval(1) != 0) {
        std::cerr << "Warning: Unable to enable VSync: " << SDL_GetError() << "\n";
    }

    return true;
}

void Engine::updateCamera(float deltaTime) {
    camera_.update(deltaTime, inputManager_);
}

void Engine::updateDebugUI(float fps, int frameCount, float uptime) {
    const char* glVersion = reinterpret_cast<const char*>(glGetString(GL_VERSION));
    const char* glRenderer = reinterpret_cast<const char*>(glGetString(GL_RENDERER));
    const bool depthEnabled = glIsEnabled(GL_DEPTH_TEST) == GL_TRUE;

    // Update debugger with frame data (cached until next interval)
    debugger_.setCameraPosition(camera_.x(), camera_.y(), camera_.z());
    debugger_.setCameraRotation(camera_.yaw(), camera_.pitch());
    debugger_.setFrameStats(fps, frameCount, uptime);
    debugger_.setGLInfo(glVersion, glRenderer);
    debugger_.setDepthTestEnabled(depthEnabled);
}

} // namespace engine