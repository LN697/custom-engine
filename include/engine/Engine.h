#pragma once

#include "engine/input/InputManager.h"
#include "engine/renderer/OpenGLRenderer.h"
#include "engine/renderer/PostProcessor.h"
#include "engine/debug/ImGuiDebugger.h"
#include "engine/camera/Camera.h"
#include "engine/scene/SceneManager.h"
#include "engine/Time.h"
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
    void updateDebugUI(float fps, int frameCount, float uptime);

    bool running_;
    int width_;
    int height_;

    SDL_Window* window_;
    SDL_GLContext glContext_;

    InputManager inputManager_;
    OpenGLRenderer renderer_;
    renderer::PostProcessor postProcessor_;
    debug::ImGuiDebugger debugger_;

    engine::camera::Camera camera_;
    scene::SceneManager sceneManager_;
    engine::Time time_;

    float debugUpdateInterval_;
};

} // namespace engine