#include "engine/graphics/shader_program.h"
#include <iostream>
#include <fstream>
#include <sstream>
#include <GL/glext.h>

#define GL_COMPILE_STATUS 0x8B81
#define GL_LINK_STATUS 0x8B82
#define GL_INFO_LOG_LENGTH 0x8B84
#define GL_VERTEX_SHADER 0x8B31
#define GL_FRAGMENT_SHADER 0x8B30

// Function pointers (use system typedefs)
static PFNGLCREATESHADERPROC glCreateShader_ptr = nullptr;
static PFNGLSHADERSOURCEPROC glShaderSource_ptr = nullptr;
static PFNGLCOMPILESHADERPROC glCompileShader_ptr = nullptr;
static PFNGLGETSHADERIVPROC glGetShaderiv_ptr = nullptr;
static PFNGLGETSHADERINFOLOGPROC glGetShaderInfoLog_ptr = nullptr;
static PFNGLDELETESHADERPROC glDeleteShader_ptr = nullptr;
static PFNGLCREATEPROGRAMPROC glCreateProgram_ptr = nullptr;
static PFNGLATTACHSHADERPROC glAttachShader_ptr = nullptr;
static PFNGLLINKPROGRAMPROC glLinkProgram_ptr = nullptr;
static PFNGLGETPROGRAMIVPROC glGetProgramiv_ptr = nullptr;
static PFNGLGETPROGRAMINFOLOGPROC glGetProgramInfoLog_ptr = nullptr;
static PFNGLDELETEPROGRAMPROC glDeleteProgram_ptr = nullptr;
static PFNGLUSEPROGRAMPROC glUseProgram_ptr = nullptr;
static PFNGLGETUNIFORMLOCATIONPROC glGetUniformLocation_ptr = nullptr;
static PFNGLUNIFORMMATRIX4FVPROC glUniformMatrix4fv_ptr = nullptr;
static PFNGLUNIFORM3FPROC glUniform3f_ptr = nullptr;
static PFNGLUNIFORM3FVPROC glUniform3fv_ptr = nullptr;
static PFNGLUNIFORM1IPROC glUniform1i_ptr = nullptr;
static PFNGLUNIFORM1FPROC glUniform1f_ptr = nullptr;

namespace engine {
namespace graphics {

// Initialize function pointers
bool ShaderProgram::initGLFunctions() {
    static bool initialized = false;
    if (initialized) return true;
    
    glCreateShader_ptr = (PFNGLCREATESHADERPROC)SDL_GL_GetProcAddress("glCreateShader");
    glShaderSource_ptr = (PFNGLSHADERSOURCEPROC)SDL_GL_GetProcAddress("glShaderSource");
    glCompileShader_ptr = (PFNGLCOMPILESHADERPROC)SDL_GL_GetProcAddress("glCompileShader");
    glGetShaderiv_ptr = (PFNGLGETSHADERIVPROC)SDL_GL_GetProcAddress("glGetShaderiv");
    glGetShaderInfoLog_ptr = (PFNGLGETSHADERINFOLOGPROC)SDL_GL_GetProcAddress("glGetShaderInfoLog");
    glDeleteShader_ptr = (PFNGLDELETESHADERPROC)SDL_GL_GetProcAddress("glDeleteShader");
    glCreateProgram_ptr = (PFNGLCREATEPROGRAMPROC)SDL_GL_GetProcAddress("glCreateProgram");
    glAttachShader_ptr = (PFNGLATTACHSHADERPROC)SDL_GL_GetProcAddress("glAttachShader");
    glLinkProgram_ptr = (PFNGLLINKPROGRAMPROC)SDL_GL_GetProcAddress("glLinkProgram");
    glGetProgramiv_ptr = (PFNGLGETPROGRAMIVPROC)SDL_GL_GetProcAddress("glGetProgramiv");
    glGetProgramInfoLog_ptr = (PFNGLGETPROGRAMINFOLOGPROC)SDL_GL_GetProcAddress("glGetProgramInfoLog");
    glDeleteProgram_ptr = (PFNGLDELETEPROGRAMPROC)SDL_GL_GetProcAddress("glDeleteProgram");
    glUseProgram_ptr = (PFNGLUSEPROGRAMPROC)SDL_GL_GetProcAddress("glUseProgram");
    glGetUniformLocation_ptr = (PFNGLGETUNIFORMLOCATIONPROC)SDL_GL_GetProcAddress("glGetUniformLocation");
    glUniformMatrix4fv_ptr = (PFNGLUNIFORMMATRIX4FVPROC)SDL_GL_GetProcAddress("glUniformMatrix4fv");
    glUniform3f_ptr = (PFNGLUNIFORM3FPROC)SDL_GL_GetProcAddress("glUniform3f");
    glUniform3fv_ptr = (PFNGLUNIFORM3FVPROC)SDL_GL_GetProcAddress("glUniform3fv");
    glUniform1i_ptr = (PFNGLUNIFORM1IPROC)SDL_GL_GetProcAddress("glUniform1i");
    glUniform1f_ptr = (PFNGLUNIFORM1FPROC)SDL_GL_GetProcAddress("glUniform1f");
    
    initialized = true;
    return true;
}

ShaderProgram::ShaderProgram() {
    initGLFunctions();
}

ShaderProgram::~ShaderProgram() {
    if (program_ != 0 && glDeleteProgram_ptr) {
        glDeleteProgram_ptr(program_);
    }
}

std::string ShaderProgram::loadFile(const std::string& path) {
    std::ifstream in(path);
    if (!in) return "";
    std::ostringstream oss;
    oss << in.rdbuf();
    return oss.str();
}

GLuint ShaderProgram::compileShader(const std::string& source, GLenum type, std::string& outError) {
    if (!glCreateShader_ptr) {
        outError = "GL functions not initialized";
        return 0;
    }
    
    GLuint shader = glCreateShader_ptr(type);
    const char* src = source.c_str();
    glShaderSource_ptr(shader, 1, &src, nullptr);
    glCompileShader_ptr(shader);

    GLint compiled = 0;
    glGetShaderiv_ptr(shader, GL_COMPILE_STATUS, &compiled);
    if (!compiled) {
        GLint infoLen = 0;
        glGetShaderiv_ptr(shader, GL_INFO_LOG_LENGTH, &infoLen);
        if (infoLen > 0) {
            std::string infoLog(infoLen, '\0');
            glGetShaderInfoLog_ptr(shader, infoLen, nullptr, infoLog.data());
            outError = infoLog;
        }
        glDeleteShader_ptr(shader);
        return 0;
    }
    return shader;
}

bool ShaderProgram::compile(const std::string& vertSource, const std::string& fragSource, std::string& outError) {
    if (!glCreateProgram_ptr) {
        outError = "GL functions not initialized";
        return false;
    }
    
    std::string vertError, fragError;
    GLuint vert = compileShader(vertSource, GL_VERTEX_SHADER, vertError);
    if (!vert) {
        outError = "Vertex shader failed: " + vertError;
        return false;
    }

    GLuint frag = compileShader(fragSource, GL_FRAGMENT_SHADER, fragError);
    if (!frag) {
        glDeleteShader_ptr(vert);
        outError = "Fragment shader failed: " + fragError;
        return false;
    }

    program_ = glCreateProgram_ptr();
    glAttachShader_ptr(program_, vert);
    glAttachShader_ptr(program_, frag);
    glLinkProgram_ptr(program_);

    GLint linked = 0;
    glGetProgramiv_ptr(program_, GL_LINK_STATUS, &linked);
    if (!linked) {
        GLint infoLen = 0;
        glGetProgramiv_ptr(program_, GL_INFO_LOG_LENGTH, &infoLen);
        if (infoLen > 0) {
            std::string infoLog(infoLen, '\0');
            glGetProgramInfoLog_ptr(program_, infoLen, nullptr, infoLog.data());
            outError = infoLog;
        }
        glDeleteProgram_ptr(program_);
        program_ = 0;
        glDeleteShader_ptr(vert);
        glDeleteShader_ptr(frag);
        return false;
    }

    glDeleteShader_ptr(vert);
    glDeleteShader_ptr(frag);
    return true;
}

bool ShaderProgram::compile_from_files(const std::string& vert_path, const std::string& frag_path, std::string& outError) {
    std::string vertSource = loadFile(vert_path);
    std::string fragSource = loadFile(frag_path);

    if (vertSource.empty()) {
        outError = "Failed to load vertex shader: " + vert_path;
        return false;
    }
    if (fragSource.empty()) {
        outError = "Failed to load fragment shader: " + frag_path;
        return false;
    }

    return compile(vertSource, fragSource, outError);
}

void ShaderProgram::use() const {
    if (glUseProgram_ptr && program_ != 0) {
        glUseProgram_ptr(program_);
    }
}

void ShaderProgram::set_mat4(const std::string& name, const float* matrix) const {
    if (!glUniformMatrix4fv_ptr || program_ == 0) return;
    GLint loc = glGetUniformLocation_ptr(program_, name.c_str());
    if (loc != -1) glUniformMatrix4fv_ptr(loc, 1, GL_FALSE, matrix);
}

void ShaderProgram::set_vec3(const std::string& name, float x, float y, float z) const {
    if (!glUniform3f_ptr || program_ == 0) return;
    GLint loc = glGetUniformLocation_ptr(program_, name.c_str());
    if (loc != -1) glUniform3f_ptr(loc, x, y, z);
}

void ShaderProgram::set_vec3(const std::string& name, const float* vec) const {
    if (!glUniform3fv_ptr || program_ == 0) return;
    GLint loc = glGetUniformLocation_ptr(program_, name.c_str());
    if (loc != -1) glUniform3fv_ptr(loc, 1, vec);
}

void ShaderProgram::set_int(const std::string& name, int value) const {
    if (!glUniform1i_ptr || program_ == 0) return;
    GLint loc = glGetUniformLocation_ptr(program_, name.c_str());
    if (loc != -1) glUniform1i_ptr(loc, value);
}

void ShaderProgram::set_float(const std::string& name, float value) const {
    if (!glUniform1f_ptr || program_ == 0) return;
    GLint loc = glGetUniformLocation_ptr(program_, name.c_str());
    if (loc != -1) glUniform1f_ptr(loc, value);
}

} // namespace graphics
} // namespace engine
