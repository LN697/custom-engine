#include "engine/Engine.h"
#include <GL/gl.h>
#include <cmath>
#include <iomanip>
#include <iostream>
#include <sstream>

namespace engine {

static constexpr float kMovementSpeed = 4.0f;
static constexpr float kMouseSensitivity = 0.0025f;
static constexpr float kDebugUpdateInterval = 0.25f;
static constexpr float kDegreesPerRadian = 180.0f / 3.14159265f;

Engine::Engine()
    : running_(false),
      width_(1280),
      height_(720),
      window_(nullptr),
      debugWindow_(nullptr),
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
    if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_EVENTS | SDL_INIT_TIMER) != 0) {
        std::cerr << "SDL_Init failed: " << SDL_GetError() << "\n";
        return false;
    }

    if (!createWindow()) return false;
    if (!renderer_.initialize(width_, height_)) return false;

    int windowX = SDL_WINDOWPOS_CENTERED;
    int windowY = SDL_WINDOWPOS_CENTERED;
    SDL_GetWindowPosition(window_, &windowX, &windowY);
    int debugX = windowX + width_ + 20;
    int debugY = windowY;

    debugWindow_ = SDL_CreateWindow(
        "Debug Info",
        debugX,
        debugY,
        460,
        140,
        SDL_WINDOW_SHOWN | SDL_WINDOW_SKIP_TASKBAR | SDL_WINDOW_ALWAYS_ON_TOP
    );

    if (!debugWindow_) {
        std::cerr << "Warning: failed to create debug window: " << SDL_GetError() << "\n";
    } else {
        SDL_ShowWindow(debugWindow_);
    }

    SDL_SetRelativeMouseMode(SDL_TRUE);
    SDL_RaiseWindow(window_);

    running_ = true;
    return true;
}

void Engine::run() {
    // FIX: High-resolution hardware timing
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

    if (debugWindow_) {
        SDL_DestroyWindow(debugWindow_);
        debugWindow_ = nullptr;
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
        SDL_WINDOW_OPENGL | SDL_WINDOW_RESIZABLE
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
}

void Engine::updateCamera(float deltaTime) {
    float forward = 0.0f;
    float strafe = 0.0f;
    float rise = 0.0f;

    if (inputManager_.moveForward())  forward += 1.0f;
    if (inputManager_.moveBackward()) forward -= 1.0f;
    if (inputManager_.moveRight())    strafe += 1.0f;
    if (inputManager_.moveLeft())     strafe -= 1.0f;
    if (inputManager_.moveUp())       rise += 1.0f;
    if (inputManager_.moveDown())     rise -= 1.0f;

    cameraYaw_ += inputManager_.mouseDeltaX() * kMouseSensitivity;
    cameraPitch_ += inputManager_.mouseDeltaY() * kMouseSensitivity;
    cameraPitch_ = std::fmax(std::fmin(cameraPitch_, 1.5f), -1.5f);

    float cosPitch = std::cos(cameraPitch_);
    float cosYaw = std::cos(cameraYaw_);
    float sinYaw = std::sin(cameraYaw_);

    // FIX: Corrected right-handed coordinate OpenGL vector math
    float forwardX = cosPitch * sinYaw;
    float forwardZ = -cosPitch * cosYaw; 
    float rightX = cosYaw;
    float rightZ = sinYaw;

    float speed = kMovementSpeed * deltaTime;
    
    // FIX: Removed sign inversion on translation addition
    cameraX_ += (forwardX * forward + rightX * strafe) * speed;
    cameraY_ += rise * speed;
    cameraZ_ += (forwardZ * forward + rightZ * strafe) * speed;
}

void Engine::updateDebugWindow(float deltaTime) {
    const float fps = frameCount_ > 0 ? frameCount_ / deltaTime : 0.0f;
    const char* glVersion = reinterpret_cast<const char*>(glGetString(GL_VERSION));
    const char* glRenderer = reinterpret_cast<const char*>(glGetString(GL_RENDERER));
    const char* glVendor = reinterpret_cast<const char*>(glGetString(GL_VENDOR));
    const bool depthEnabled = glIsEnabled(GL_DEPTH_TEST) == GL_TRUE;

    std::ostringstream title;
    title << "FPS=" << std::fixed << std::setprecision(1) << fps;
    title << " | Pos=(" << std::setprecision(2) << cameraX_ << "," << cameraY_ << "," << cameraZ_ << ")";
    title << " | Yaw=" << std::setprecision(1) << cameraYaw_ * kDegreesPerRadian;
    title << " | Pitch=" << cameraPitch_ * kDegreesPerRadian;
    title << " | Keys=" << inputManager_.keySummary();
    title << " | Mouse=" << inputManager_.mouseSummary();
    title << " | GL=" << (glVersion ? glVersion : "Unknown");
    title << " | GPU=" << (glRenderer ? glRenderer : "Unknown");
    title << " | Vendor=" << (glVendor ? glVendor : "Unknown");
    title << " | Depth=" << (depthEnabled ? "On" : "Off");
    title << " | Pipeline=Immediate";
    title << " | Viewport=" << width_ << "x" << height_;

    if (debugWindow_) {
        SDL_SetWindowTitle(debugWindow_, title.str().c_str());
    }
}

} // namespace engine
