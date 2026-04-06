#include "engine/renderer/PostProcessor.h"
#include <GL/gl.h>
#include <SDL2/SDL.h>
#include <fstream>
#include <sstream>
#include <iostream>

#define GL_COMPILE_STATUS 0x8B81
#define GL_LINK_STATUS 0x8B82
#define GL_INFO_LOG_LENGTH 0x8B84
#define GL_VERTEX_SHADER 0x8B31
#define GL_FRAGMENT_SHADER 0x8B30

namespace engine {
namespace renderer {

// GL function pointers loaded at runtime
typedef GLuint (APIENTRYP PFNGLCREATESHADERPROC)(GLenum);
typedef void (APIENTRYP PFNGLSHADERSOURCEPROC)(GLuint, GLsizei, const GLchar**, const GLint*);
typedef void (APIENTRYP PFNGLCOMPILESHADERPROC)(GLuint);
typedef void (APIENTRYP PFNGLGETSHADERIVPROC)(GLuint, GLenum, GLint*);
typedef void (APIENTRYP PFNGLGETSHADERINFOLOGPROC)(GLuint, GLsizei, GLsizei*, GLchar*);
typedef void (APIENTRYP PFNGLDELETESHADERPROC)(GLuint);
typedef GLuint (APIENTRYP PFNGLCREATEPROGRAMPROC)(void);
typedef void (APIENTRYP PFNGLATTACHSHADERPROC)(GLuint, GLuint);
typedef void (APIENTRYP PFNGLLINKPROGRAMPROC)(GLuint);
typedef void (APIENTRYP PFNGLGETPROGRAMIVPROC)(GLuint, GLenum, GLint*);
typedef void (APIENTRYP PFNGLGETPROGRAMINFOLOGPROC)(GLuint, GLsizei, GLsizei*, GLchar*);
typedef void (APIENTRYP PFNGLDELETEPROGRAMPROC)(GLuint);
typedef void (APIENTRYP PFNGLUSEPROGRAMPROC)(GLuint);
typedef GLint (APIENTRYP PFNGLGETUNIFORMLOCATIONPROC)(GLuint, const GLchar*);
typedef void (APIENTRYP PFNGLUNIFORM1IPROC)(GLint, GLint);

// Function pointers
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
static PFNGLUNIFORM1IPROC glUniform1i_ptr = nullptr;

// Initialize function pointers
static bool initGLFunctions() {
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
    glUniform1i_ptr = (PFNGLUNIFORM1IPROC)SDL_GL_GetProcAddress("glUniform1i");
    
    initialized = true;
    return glCreateShader_ptr && glShaderSource_ptr && glCompileShader_ptr &&
           glGetShaderiv_ptr && glGetShaderInfoLog_ptr && glDeleteShader_ptr &&
           glCreateProgram_ptr && glAttachShader_ptr && glLinkProgram_ptr &&
           glGetProgramiv_ptr && glGetProgramInfoLog_ptr && glDeleteProgram_ptr &&
           glUseProgram_ptr && glGetUniformLocation_ptr && glUniform1i_ptr;
}

static const char* kDefaultVert =
"#version 120\n"
"void main() {\n"
"    gl_TexCoord[0] = gl_MultiTexCoord0;\n"
"    gl_Position = ftransform();\n"
"}\n";

static const char* kDefaultFrag =
"#version 120\n"
"uniform sampler2D uTexture;\n"
"uniform int uEffect;\n"
"void main() {\n"
"    vec2 uv = gl_TexCoord[0].st;\n"
"    vec4 c = texture2D(uTexture, uv);\n"
"    if (uEffect == 1) {\n"
"        float gray = dot(c.rgb, vec3(0.299,0.587,0.114));\n"
"        c = vec4(vec3(gray), c.a);\n"
"    } else if (uEffect == 2) {\n"
"        c = vec4(vec3(1.0) - c.rgb, c.a);\n"
"    } else if (uEffect == 3) {\n"
"        vec3 s;\n"
"        s.r = dot(c.rgb, vec3(0.393,0.769,0.189));\n"
"        s.g = dot(c.rgb, vec3(0.349,0.686,0.168));\n"
"        s.b = dot(c.rgb, vec3(0.272,0.534,0.131));\n"
"        c = vec4(s, c.a);\n"
"    }\n"
"    gl_FragColor = c;\n"
"}\n";

PostProcessor::PostProcessor()
    : texture_(0), shaderProgram_(0), width_(0), height_(0), initialized_(false), enabled_(false), effect_(Effect_None) {
}

PostProcessor::~PostProcessor() {
    shutdown();
}

bool PostProcessor::initialize(int width, int height) {
    if (!initGLFunctions()) {
        lastLog_ = "PostProcessor: Failed to initialize GL functions";
        return false;
    }

    width_ = width;
    height_ = height;

    if (!initialized_) {
        glGenTextures(1, &texture_);
        glBindTexture(GL_TEXTURE_2D, texture_);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, width_, height_, 0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr);
        glBindTexture(GL_TEXTURE_2D, 0);

        // Compile default pass-through shader so rendering can work out of the box
        std::string log;
        if (!compileShader(kDefaultVert, kDefaultFrag, log)) {
            lastLog_ = "Default shader compile failed:\n" + log;
            // Not fatal — we still allow the engine to run without shader
        }

        initialized_ = true;
    }

    return true;
}

void PostProcessor::resize(int width, int height) {
    width_ = width;
    height_ = height;
    if (texture_ != 0) {
        glBindTexture(GL_TEXTURE_2D, texture_);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, width_, height_, 0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr);
        glBindTexture(GL_TEXTURE_2D, 0);
    }
}

void PostProcessor::shutdown() {
    if (shaderProgram_ != 0) {
        if (glDeleteProgram_ptr) glDeleteProgram_ptr(shaderProgram_);
        shaderProgram_ = 0;
    }
    if (texture_ != 0) {
        glDeleteTextures(1, &texture_);
        texture_ = 0;
    }
    initialized_ = false;
}

std::string PostProcessor::loadFile(const std::string& path) {
    std::ifstream in(path.c_str(), std::ios::in | std::ios::binary);
    if (!in) return std::string();
    std::ostringstream contents;
    contents << in.rdbuf();
    return contents.str();
}

GLuint PostProcessor::compileSingleShader(GLenum type, const char* src, std::string& outLog) {
    if (!glCreateShader_ptr || !glShaderSource_ptr || !glCompileShader_ptr ||
        !glGetShaderiv_ptr || !glGetShaderInfoLog_ptr || !glDeleteShader_ptr) {
        outLog = "GL function pointers not initialized";
        return 0;
    }

    GLuint shader = glCreateShader_ptr(type);
    glShaderSource_ptr(shader, 1, &src, nullptr);
    glCompileShader_ptr(shader);

    GLint compiled = 0;
    glGetShaderiv_ptr(shader, GL_COMPILE_STATUS, &compiled);
    GLint infoLen = 0;
    glGetShaderiv_ptr(shader, GL_INFO_LOG_LENGTH, &infoLen);
    if (infoLen > 1) {
        std::string info;
        info.resize(infoLen);
        glGetShaderInfoLog_ptr(shader, infoLen, nullptr, &info[0]);
        outLog += info;
    }

    if (!compiled) {
        glDeleteShader_ptr(shader);
        return 0;
    }
    return shader;
}

bool PostProcessor::compileShader(const char* vertSrc, const char* fragSrc, std::string& outLog) {
    if (!initGLFunctions()) {
        outLog = "Failed to initialize GL functions";
        return false;
    }

    GLuint vert = compileSingleShader(GL_VERTEX_SHADER, vertSrc, outLog);
    if (!vert) {
        outLog = "Vertex shader compile failed:\n" + outLog;
        return false;
    }

    GLuint frag = compileSingleShader(GL_FRAGMENT_SHADER, fragSrc, outLog);
    if (!frag) {
        glDeleteShader_ptr(vert);
        outLog = "Fragment shader compile failed:\n" + outLog;
        return false;
    }

    GLuint program = glCreateProgram_ptr();
    if (!program) {
        glDeleteShader_ptr(vert);
        glDeleteShader_ptr(frag);
        outLog = "Failed to create shader program";
        return false;
    }

    glAttachShader_ptr(program, vert);
    glAttachShader_ptr(program, frag);
    glLinkProgram_ptr(program);

    GLint linked = 0;
    glGetProgramiv_ptr(program, GL_LINK_STATUS, &linked);
    GLint infoLen = 0;
    glGetProgramiv_ptr(program, GL_INFO_LOG_LENGTH, &infoLen);
    if (infoLen > 1) {
        std::string info;
        info.resize(infoLen);
        glGetProgramInfoLog_ptr(program, infoLen, nullptr, &info[0]);
        outLog += info;
    }

    if (!linked) {
        glDeleteProgram_ptr(program);
        glDeleteShader_ptr(vert);
        glDeleteShader_ptr(frag);
        return false;
    }

    // Delete old program if any
    if (shaderProgram_ != 0) {
        glDeleteProgram_ptr(shaderProgram_);
    }
    shaderProgram_ = program;

    glDeleteShader_ptr(vert);
    glDeleteShader_ptr(frag);

    return true;
}

bool PostProcessor::loadShaderFiles(const std::string& vertPath, const std::string& fragPath) {
    std::string vertSrc = loadFile(vertPath);
    std::string fragSrc = loadFile(fragPath);

    if (vertSrc.empty()) {
        lastLog_ = "Failed to load vertex shader file: " + vertPath;
        return false;
    }
    if (fragSrc.empty()) {
        lastLog_ = "Failed to load fragment shader file: " + fragPath;
        return false;
    }

    std::string log;
    bool result = compileShader(vertSrc.c_str(), fragSrc.c_str(), log);
    lastLog_ = log;
    return result;
}

void PostProcessor::capture() {
    if (!initialized_ || !enabled_) return;

    glBindTexture(GL_TEXTURE_2D, texture_);
    glCopyTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, 0, 0, width_, height_);
    glBindTexture(GL_TEXTURE_2D, 0);
}

void PostProcessor::render() {
    if (!initialized_ || !enabled_ || !shaderProgram_) return;

    glUseProgram_ptr(shaderProgram_);

    GLint texLoc = glGetUniformLocation_ptr(shaderProgram_, "uTexture");
    if (texLoc != -1) {
        glUniform1i_ptr(texLoc, 0);
    }

    GLint effectLoc = glGetUniformLocation_ptr(shaderProgram_, "uEffect");
    if (effectLoc != -1) {
        glUniform1i_ptr(effectLoc, effect_);
    }

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, texture_);

    // Draw full-screen quad
    glMatrixMode(GL_PROJECTION);
    glPushMatrix();
    glLoadIdentity();
    glOrtho(0.0f, 1.0f, 0.0f, 1.0f, -1.0f, 1.0f);

    glMatrixMode(GL_MODELVIEW);
    glPushMatrix();
    glLoadIdentity();

    glBegin(GL_QUADS);
    glTexCoord2f(0.0f, 0.0f); glVertex2f(0.0f, 0.0f);
    glTexCoord2f(1.0f, 0.0f); glVertex2f(1.0f, 0.0f);
    glTexCoord2f(1.0f, 1.0f); glVertex2f(1.0f, 1.0f);
    glTexCoord2f(0.0f, 1.0f); glVertex2f(0.0f, 1.0f);
    glEnd();

    glPopMatrix();
    glMatrixMode(GL_PROJECTION);
    glPopMatrix();

    glMatrixMode(GL_MODELVIEW);
    glUseProgram_ptr(0);
}

}  // namespace renderer
}  // namespace engine
