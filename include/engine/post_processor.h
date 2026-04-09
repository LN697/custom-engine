#pragma once

#include <glm/glm.hpp>
#include <string>
#include <GL/gl.h>

namespace engine {

class PostProcessor {
public:
    PostProcessor();
    ~PostProcessor();

    bool initialize(int width, int height);
    void shutdown();
    void resize(int width, int height);

    bool load_shader_files(const std::string& vert_path, const std::string& frag_path);
    void begin_capture();
    void end_capture();
    void render();
    void check_for_shader_reload();

    bool is_enabled() const { return enabled_; }
    void set_enabled(bool enabled) { enabled_ = enabled; }
    std::string last_log() const { return last_log_; }
    // Effect control (0=none, 1=grayscale, 2=invert, 3=sepia)
    int effect() const { return effect_; }
    void set_effect(int e) { effect_ = e; }
    bool has_shader() const { return shader_program_ != 0; }

private:
    std::string load_file(const std::string& path);
    GLuint compile_single_shader(GLenum type, const char* src, std::string& out_log);
    bool compile_shader(const char* vert_src, const char* frag_src, std::string& out_log);
    void set_watched_shader_files(const std::string& vert_path, const std::string& frag_path);

    GLuint texture_;
    GLuint shader_program_;
    GLuint fbo_;
    GLuint depth_rbo_;
    int width_;
    int height_;
    bool initialized_;
    bool enabled_;
    int effect_;

    std::string last_log_;
    std::string watched_vert_;
    std::string watched_frag_;

    enum EffectType {
        EffectNone = 0,
        EffectGrayscale = 1,
        EffectInvert = 2,
        EffectSepia = 3
    };
};

// (no out-of-class inline needed; accessor exists inside class)

} // namespace engine
