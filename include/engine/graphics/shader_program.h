#pragma once

#include <GL/gl.h>
#include <string>
#include <SDL2/SDL.h>

namespace engine {
namespace graphics {

class ShaderProgram {
public:
    ShaderProgram();
    ~ShaderProgram();

    bool compile(const std::string& vert_source, const std::string& frag_source, std::string& out_error);
    bool compile_from_files(const std::string& vert_path, const std::string& frag_path, std::string& out_error);

    GLuint get_program() const { return program_; }
    bool is_valid() const { return program_ != 0; }

    void use() const;
    void set_mat4(const std::string& name, const float* matrix) const;
    void set_vec3(const std::string& name, float x, float y, float z) const;
    void set_vec3(const std::string& name, const float* vec) const;
    void set_int(const std::string& name, int value) const;
    void set_float(const std::string& name, float value) const;

private:
    GLuint program_ = 0;

    GLuint compileShader(const std::string& source, GLenum type, std::string& outError);
    std::string loadFile(const std::string& path);
    static bool initGLFunctions();
};

} // namespace graphics
} // namespace engine
