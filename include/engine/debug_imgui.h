#pragma once

#include <SDL2/SDL.h>
#include <glm/glm.hpp>
#include <string>
#include <vector>
#include <functional>
#include <utility>

namespace engine {

class PostProcessor;

class ImGuiDebugger {
public:
    ImGuiDebugger();
    ~ImGuiDebugger();

    bool initialize(SDL_Window* window, SDL_GLContext context);
    void shutdown();

    void begin_frame();
    void draw_ui();
    void end_frame();
    void render();

    void process_event(const SDL_Event& event);
    // Record an event for the Events UI (keeps a short history)
    void record_event(const SDL_Event& event);
    void clear_event_log();

    void set_post_processor(PostProcessor* post_processor) { post_processor_ = post_processor; }
    void attach_movement_smoothing(float* smoothing_ptr) { movement_smoothing_ptr_ = smoothing_ptr; }
    void set_command_callback(std::function<void(const std::string&)> callback) { command_callback_ = callback; }

    void add_console_log(const std::string& message);
    void set_stage_timings(const std::vector<std::pair<std::string, float>>& timings);

    void set_camera_position(float x, float y, float z) { camera_pos_ = glm::vec3(x, y, z); }
    void set_camera_rotation(float yaw, float pitch) { camera_yaw_ = yaw; camera_pitch_ = pitch; }
    void set_frame_stats(float fps, int frame_count, float uptime);
    void set_gl_info(const char* version, const char* renderer);
    void set_depth_test_enabled(bool enabled) { depth_test_enabled_ = enabled; }

private:
    bool is_initialized_;
    SDL_Window* window_;
    SDL_GLContext gl_context_;
    
    PostProcessor* post_processor_;
    float* movement_smoothing_ptr_;
    std::function<void(const std::string&)> command_callback_;
    
    glm::vec3 camera_pos_;
    float camera_yaw_;
    float camera_pitch_;
    float fps_;
    int frame_count_;
    float uptime_;
    std::string gl_version_;
    std::string gl_renderer_;
    bool depth_test_enabled_;
    
    std::vector<std::pair<std::string, float>> stage_timings_;
    std::vector<std::string> console_log_;
    // Console UI enabled flag (can disable in-game command tab)
    bool console_enabled_;

    // Recent event history (for the Events window)
    std::vector<std::string> event_log_;
    size_t event_log_capacity_;

    // FPS history for graphing
    std::vector<float> fps_history_;
    size_t fps_index_;
    size_t fps_capacity_;
};

} // namespace engine
