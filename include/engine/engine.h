#pragma once

#include "engine/input_manager.h"
#include "engine/input/control_scheme.h"
#include "engine/renderer.h"
#include "engine/post_processor.h"
#include "engine/debug_imgui.h"
#include "engine/camera.h"
#include "engine/scene/scene_manager.h"
#include "engine/time.h"
#include <SDL2/SDL.h>
// #include "engine/asset_manager.h"  // disabled for debugging - needs fixes to work with new resource system
#include "engine/debug_imgui.h"
#include "engine/scene/default_scene.h"
#include <imgui.h>
#include <GL/gl.h>
#include <cmath>
#include <iostream>
#include <sstream>
#include <cstdlib>
#include <vector>
#include <utility>
#include <memory>

namespace engine {

class Engine {
public:
    Engine();
    ~Engine();

    bool initialize();
    void run();
    void shutdown();

    /**
     * Set the control scheme for the current scene.
     * Supports Minecraft, FPS, Doom-like, or custom control schemes.
     */
    void set_scene_control_scheme(std::shared_ptr<ControlScheme> control_scheme);

private:
    bool create_window();
    void handle_event(const SDL_Event& event);
    void update_camera(float delta_time);
    void update_debug_ui(float fps, int frame_count, float uptime);

    bool is_running_;
    int window_width_;
    int window_height_;

    SDL_Window* window_;
    SDL_GLContext gl_context_;

    InputManager input_manager_;
    OpenGLRenderer renderer_;
    PostProcessor post_processor_;
    ImGuiDebugger debugger_;
    Camera camera_;
    // AssetManager asset_manager_;  // disabled - need to fix
    scene::SceneManager scene_manager_;
    Time time_;

    std::shared_ptr<ControlScheme> current_control_scheme_;
    float debug_update_interval_;
};

} // namespace engine
