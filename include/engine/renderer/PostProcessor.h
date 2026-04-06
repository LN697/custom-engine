#pragma once

#include <string>
#include <GL/gl.h>

namespace engine {
namespace renderer {

class PostProcessor {
public:
    enum Effect { Effect_None = 0, Effect_Grayscale = 1, Effect_Invert = 2, Effect_Sepia = 3 };

    PostProcessor();
    ~PostProcessor();

    bool initialize(int width, int height);
    void resize(int width, int height);
    void shutdown();

    // Load/compile shader program from files. Returns true on success.
    bool loadShaderFiles(const std::string& vertPath, const std::string& fragPath);
    const std::string& getLastLog() const { return lastLog_; }

    void capture(); // copy current framebuffer into texture
    void render();  // draw full-screen quad with post-process shader

    void setEnabled(bool enabled) { enabled_ = enabled; }
    bool isEnabled() const { return enabled_; }

    void setEffect(Effect e) { effect_ = e; }
    Effect effect() const { return effect_; }

    bool hasProgram() const { return shaderProgram_ != 0; }

private:
    GLuint texture_;
    GLuint shaderProgram_;
    int width_;
    int height_;
    bool initialized_;
    bool enabled_;
    std::string lastLog_;
    Effect effect_;

    bool compileShader(const char* vertSrc, const char* fragSrc, std::string& outLog);
    std::string loadFile(const std::string& path);
    GLuint compileSingleShader(GLenum type, const char* src, std::string& outLog);
};

} // namespace renderer
} // namespace engine
