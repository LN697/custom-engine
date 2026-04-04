#pragma once

#include "engine/input/InputManager.h"
#include "engine/renderer/OpenGLRenderer.h"
#include <SDL2/SDL.h>

namespace engine {

class Engine {
public:
    Engine();
    ~Engine();

    bool initialize();
    void run();
    void shutdown();

private:
    bool createWindow();
    void handleEvent(const SDL_Event& event);
    void updateCamera(float deltaTime);
    void updateDebugWindow(float deltaTime);

    bool running_;
    int width_;
    int height_;

    SDL_Window* window_;
    SDL_GLContext glContext_;

    InputManager inputManager_;
    OpenGLRenderer renderer_;

    float cameraX_;
    float cameraY_;
    float cameraZ_;
    float cameraYaw_;
    float cameraPitch_;

    float frameTimer_;
    int frameCount_;
    float debugUpdateInterval_;
};

} // namespace engine