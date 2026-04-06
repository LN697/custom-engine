#include "engine/Engine.h"
#include "engine/debug/ImGuiDebugger.h"
#include "engine/renderer/PostProcessor.h"
#include <backends/imgui_impl_sdl2.h>
#include <SDL2/SDL.h>
#include <GL/gl.h>
#include <cmath>
#include <iostream>

namespace engine {

static constexpr float kMovementSpeed = 4.0f;
static constexpr float kMouseSensitivity = 0.0025f;
static constexpr float kKeyboardLookSpeed = 1.5f; // radians per second for arrow key look input
static constexpr float kDebugUpdateInterval = 0.25f;

Engine::Engine()
        : running_(false),
            width_(1280),
            height_(720),
            window_(nullptr),
            glContext_(nullptr),
            cameraX_(0.0f),
            cameraY_(1.6f),
            cameraZ_(4.0f),
            cameraYaw_(0.0f),
            cameraPitch_(0.0f),
            cameraVelX_(0.0f),
            cameraVelY_(0.0f),
            cameraVelZ_(0.0f),
            movementSmoothing_(12.0f),
            frameTimer_(0.0f),
            frameCount_(0),
            debugUpdateInterval_(0.0f) {
}

Engine::~Engine() {
    shutdown();
}

bool Engine::initialize() {
    SDL_SetHint(SDL_HINT_VIDEODRIVER, "wayland,x11");
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
    debugger_.attachMovementSmoothing(&movementSmoothing_);

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
    }

    // FIX: Request the pointer lock ONLY when the user clicks inside the active window.
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

        SDL_Event event;
        while (SDL_PollEvent(&event)) {
            handleEvent(event);
        }

        // Begin ImGui frame
        debugger_.beginFrame();

        updateCamera(deltaTime);
        renderer_.render(cameraX_, cameraY_, cameraZ_, cameraYaw_, cameraPitch_);

        // Capture the rendered scene for post-processing (before any UI is drawn)
        if (postProcessor_.isEnabled()) postProcessor_.capture();

        // Accumulate timing and count frames for the debug interval
        frameTimer_ += deltaTime;
        frameCount_++;
        debugUpdateInterval_ += deltaTime;

        // Update debug statistics at a lower frequency to avoid jitter
        if (debugUpdateInterval_ >= kDebugUpdateInterval) {
            updateDebugUI(debugUpdateInterval_);
            debugUpdateInterval_ = 0.0f;
            frameCount_ = 0;
        }

        // Draw post-processed scene (if enabled) before UI so UI overlays on top
        if (postProcessor_.isEnabled()) postProcessor_.render();

        // Always draw the UI (it uses the latest cached stats)
        debugger_.drawUI();

        // End ImGui frame and render
        debugger_.endFrame();
        debugger_.render();

        SDL_GL_SwapWindow(window_);
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

    // Note: GLEW initialization skipped to avoid header compatibility issues.
    // Standard GL functions are available via the OpenGL system headers.

    return true;
}

void Engine::updateCamera(float deltaTime) {
    // 1. Gather digital input states
    float forward = 0.0f;
    float strafe = 0.0f;
    float rise = 0.0f;

    if (inputManager_.moveForward())  forward += 1.0f;
    if (inputManager_.moveBackward()) forward -= 1.0f;
    if (inputManager_.moveRight())    strafe += 1.0f;
    if (inputManager_.moveLeft())     strafe -= 1.0f;
    if (inputManager_.moveUp())       rise += 1.0f;
    if (inputManager_.moveDown())     rise -= 1.0f;

    float length = std::sqrt(forward * forward + strafe * strafe + rise * rise);
    if (length > 0.0f) {
        forward /= length;
        strafe /= length;
        rise /= length;
    }

    // FIX: Query SDL directly for relative mouse state to bypass event-loop jitter and synthetic OS warps
    int mouseDX, mouseDY;
    SDL_GetRelativeMouseState(&mouseDX, &mouseDY);

    cameraYaw_ += mouseDX * kMouseSensitivity;
    cameraPitch_ += mouseDY * kMouseSensitivity;

    float yawInput = static_cast<float>(inputManager_.lookRight() - inputManager_.lookLeft());
    float pitchInput = static_cast<float>(inputManager_.lookDown() - inputManager_.lookUp());

    cameraYaw_ += yawInput * kKeyboardLookSpeed * deltaTime;
    cameraPitch_ += pitchInput * kKeyboardLookSpeed * deltaTime;
    
    // Clamp pitch to prevent the camera from flipping upside down
    cameraPitch_ = std::fmax(std::fmin(cameraPitch_, 1.5f), -1.5f);

    // FIX: 3D Spectator Movement Math
    float cosYaw = std::cos(cameraYaw_);
    float sinYaw = std::sin(cameraYaw_);
    float cosPitch = std::cos(cameraPitch_);
    float sinPitch = std::sin(cameraPitch_);

    // True 3D forward vector (incorporates pitch for vertical Y-axis traversal)
    float forwardX = sinYaw * cosPitch;
    float forwardY = -sinPitch; 
    float forwardZ = -cosYaw * cosPitch;

    // Right vector remains strictly horizontal so A/D strafing doesn't change your altitude
    float rightX = cosYaw;
    float rightY = 0.0f;
    float rightZ = sinYaw;

    // Desired world-space velocity (units per second)
    float desiredVelX = (forwardX * forward + rightX * strafe) * kMovementSpeed;
    float desiredVelY = (forwardY * forward + rightY * strafe + rise) * kMovementSpeed;
    float desiredVelZ = (forwardZ * forward + rightZ * strafe) * kMovementSpeed;

    // Smooth velocity using exponential approach for consistent feel across frame rates
    float k = movementSmoothing_;
    float t = 1.0f - std::exp(-k * deltaTime);
    cameraVelX_ += (desiredVelX - cameraVelX_) * t;
    cameraVelY_ += (desiredVelY - cameraVelY_) * t;
    cameraVelZ_ += (desiredVelZ - cameraVelZ_) * t;

    // Integrate position
    cameraX_ += cameraVelX_ * deltaTime;
    cameraY_ += cameraVelY_ * deltaTime;
    cameraZ_ += cameraVelZ_ * deltaTime;
}

void Engine::updateDebugUI(float elapsedInterval) {
    const float fps = (elapsedInterval > 0.0f && frameCount_ > 0) ? frameCount_ / elapsedInterval : 0.0f;
    const char* glVersion = reinterpret_cast<const char*>(glGetString(GL_VERSION));
    const char* glRenderer = reinterpret_cast<const char*>(glGetString(GL_RENDERER));
    const bool depthEnabled = glIsEnabled(GL_DEPTH_TEST) == GL_TRUE;

    // Update debugger with frame data (cached until next interval)
    debugger_.setCameraPosition(cameraX_, cameraY_, cameraZ_);
    debugger_.setCameraRotation(cameraYaw_, cameraPitch_);
    debugger_.setFrameStats(fps, frameCount_, frameTimer_);
    debugger_.setGLInfo(glVersion, glRenderer);
    debugger_.setDepthTestEnabled(depthEnabled);
}

} // namespace engine