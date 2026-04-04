#include "engine/Engine.h"
#include <GL/gl.h>
#include <cmath>
#include <iomanip>
#include <iostream>
#include <sstream>

namespace engine {

static constexpr float kMovementSpeed = 4.0f;
static constexpr float kMouseSensitivity = 0.0025f;
static constexpr float kKeyboardLookSpeed = 1.5f; // radians per second for arrow key look input
static constexpr float kDebugUpdateInterval = 0.25f;
static constexpr float kDegreesPerRadian = 180.0f / 3.14159265358979f;

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

        updateCamera(deltaTime);
        renderer_.render(cameraX_, cameraY_, cameraZ_, cameraYaw_, cameraPitch_);

        SDL_GL_SwapWindow(window_);
        inputManager_.resetFrameState();

        frameTimer_ += deltaTime;
        frameCount_++;
        debugUpdateInterval_ += deltaTime;
        
        if (debugUpdateInterval_ >= kDebugUpdateInterval) {
            updateDebugWindow(debugUpdateInterval_);
            debugUpdateInterval_ = 0.0f;
            frameCount_ = 0;
        }
    }
}

void Engine::shutdown() {
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

    float speed = kMovementSpeed * deltaTime;
    
    // 5. Apply world-space translations
    cameraX_ += (forwardX * forward + rightX * strafe) * speed;
    cameraY_ += (forwardY * forward + rightY * strafe + rise) * speed;
    cameraZ_ += (forwardZ * forward + rightZ * strafe) * speed;
}

void Engine::updateDebugWindow(float deltaTime) {
    const float fps = frameCount_ > 0 ? frameCount_ / deltaTime : 0.0f;
    const char* glVersion = reinterpret_cast<const char*>(glGetString(GL_VERSION));
    const char* glRenderer = reinterpret_cast<const char*>(glGetString(GL_RENDERER));
    const bool depthEnabled = glIsEnabled(GL_DEPTH_TEST) == GL_TRUE;

    std::ostringstream title;
    title << "Engine | FPS=" << std::fixed << std::setprecision(1) << fps;
    title << " | Pos=(" << std::setprecision(2) << cameraX_ << "," << cameraY_ << "," << cameraZ_ << ")";
    title << " | Yaw=" << std::setprecision(1) << cameraYaw_ * kDegreesPerRadian;
    title << " | Pitch=" << cameraPitch_ * kDegreesPerRadian;
    title << " | Depth=" << (depthEnabled ? "On" : "Off");
    title << " | Uptime=" << std::fixed << std::setprecision(1) << frameTimer_ << "s";
    title << " | GL=" << (glVersion ? glVersion : "Unknown");
    title << " | GPU=" << (glRenderer ? glRenderer : "Unknown");
    
    // Push the telemetry dynamically to the main window's title bar
    if (window_) {
        SDL_SetWindowTitle(window_, title.str().c_str());
    }
}

} // namespace engine