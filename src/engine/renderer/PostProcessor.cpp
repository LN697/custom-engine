#include "engine/post_processor.h"
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
// FBO/renderbuffer function pointers (optional)
typedef void (APIENTRYP PFNGLGENFRAMEBUFFERSPROC)(GLsizei, GLuint*);
typedef void (APIENTRYP PFNGLBINDFRAMEBUFFERPROC)(GLenum, GLuint);
typedef void (APIENTRYP PFNGLFRAMEBUFFERTEXTURE2DPROC)(GLenum, GLenum, GLenum, GLuint, GLint);
typedef void (APIENTRYP PFNGLDELETEFRAMEBUFFERSPROC)(GLsizei, const GLuint*);
typedef void (APIENTRYP PFNGLGENRENDERBUFFERSPROC)(GLsizei, GLuint*);
typedef void (APIENTRYP PFNGLBINDRENDERBUFFERPROC)(GLenum, GLuint);
typedef void (APIENTRYP PFNGLRENDERBUFFERSTORAGEPROC)(GLenum, GLenum, GLsizei, GLsizei);
typedef void (APIENTRYP PFNGLFRAMEBUFFERRENDERBUFFERPROC)(GLenum, GLenum, GLenum, GLuint);
typedef GLenum (APIENTRYP PFNGLCHECKFRAMEBUFFERSTATUSPROC)(GLenum);
typedef void (APIENTRYP PFNGLDELETERENDERBUFFERSPROC)(GLsizei, const GLuint*);

static PFNGLGENFRAMEBUFFERSPROC glGenFramebuffers_ptr = nullptr;
static PFNGLBINDFRAMEBUFFERPROC glBindFramebuffer_ptr = nullptr;
static PFNGLFRAMEBUFFERTEXTURE2DPROC glFramebufferTexture2D_ptr = nullptr;
static PFNGLDELETEFRAMEBUFFERSPROC glDeleteFramebuffers_ptr = nullptr;
static PFNGLGENRENDERBUFFERSPROC glGenRenderbuffers_ptr = nullptr;
static PFNGLBINDRENDERBUFFERPROC glBindRenderbuffer_ptr = nullptr;
static PFNGLRENDERBUFFERSTORAGEPROC glRenderbufferStorage_ptr = nullptr;
static PFNGLFRAMEBUFFERRENDERBUFFERPROC glFramebufferRenderbuffer_ptr = nullptr;
static PFNGLCHECKFRAMEBUFFERSTATUSPROC glCheckFramebufferStatus_ptr = nullptr;
static PFNGLDELETERENDERBUFFERSPROC glDeleteRenderbuffers_ptr = nullptr;

// Initialize function pointers
static bool init_gl_functions() {
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
    // FBO functions (may be available depending on driver)
    glGenFramebuffers_ptr = (PFNGLGENFRAMEBUFFERSPROC)SDL_GL_GetProcAddress("glGenFramebuffers");
    glBindFramebuffer_ptr = (PFNGLBINDFRAMEBUFFERPROC)SDL_GL_GetProcAddress("glBindFramebuffer");
    glFramebufferTexture2D_ptr = (PFNGLFRAMEBUFFERTEXTURE2DPROC)SDL_GL_GetProcAddress("glFramebufferTexture2D");
    glDeleteFramebuffers_ptr = (PFNGLDELETEFRAMEBUFFERSPROC)SDL_GL_GetProcAddress("glDeleteFramebuffers");
    glGenRenderbuffers_ptr = (PFNGLGENRENDERBUFFERSPROC)SDL_GL_GetProcAddress("glGenRenderbuffers");
    glBindRenderbuffer_ptr = (PFNGLBINDRENDERBUFFERPROC)SDL_GL_GetProcAddress("glBindRenderbuffer");
    glRenderbufferStorage_ptr = (PFNGLRENDERBUFFERSTORAGEPROC)SDL_GL_GetProcAddress("glRenderbufferStorage");
    glFramebufferRenderbuffer_ptr = (PFNGLFRAMEBUFFERRENDERBUFFERPROC)SDL_GL_GetProcAddress("glFramebufferRenderbuffer");
    glCheckFramebufferStatus_ptr = (PFNGLCHECKFRAMEBUFFERSTATUSPROC)SDL_GL_GetProcAddress("glCheckFramebufferStatus");
    glDeleteRenderbuffers_ptr = (PFNGLDELETERENDERBUFFERSPROC)SDL_GL_GetProcAddress("glDeleteRenderbuffers");
    
    initialized = true;
    return glCreateShader_ptr && glShaderSource_ptr && glCompileShader_ptr &&
           glGetShaderiv_ptr && glGetShaderInfoLog_ptr && glDeleteShader_ptr &&
           glCreateProgram_ptr && glAttachShader_ptr && glLinkProgram_ptr &&
           glGetProgramiv_ptr && glGetProgramInfoLog_ptr && glDeleteProgram_ptr &&
           glUseProgram_ptr && glGetUniformLocation_ptr && glUniform1i_ptr;
}

static const char* k_default_vert =
"#version 120\n"
"void main() {\n"
"    gl_TexCoord[0] = gl_MultiTexCoord0;\n"
"    gl_Position = ftransform();\n"
"}\n";

static const char* k_default_frag =
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
    : texture_(0), shader_program_(0), width_(0), height_(0), initialized_(false), enabled_(false), effect_(EffectNone) {
}

PostProcessor::~PostProcessor() {
    shutdown();
}

bool PostProcessor::initialize(int width, int height) {
    if (!init_gl_functions()) {
        last_log_ = "PostProcessor: Failed to initialize GL functions";
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

        // If FBO functions are available, create an FBO and attach the texture
        if (glGenFramebuffers_ptr && glBindFramebuffer_ptr && glFramebufferTexture2D_ptr) {
            glGenFramebuffers_ptr(1, &fbo_);
            glBindFramebuffer_ptr(GL_FRAMEBUFFER, fbo_);
            glFramebufferTexture2D_ptr(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, texture_, 0);

            // Create a depth renderbuffer
            if (glGenRenderbuffers_ptr && glBindRenderbuffer_ptr && glRenderbufferStorage_ptr && glFramebufferRenderbuffer_ptr) {
                glGenRenderbuffers_ptr(1, &depth_rbo_);
                glBindRenderbuffer_ptr(GL_RENDERBUFFER, depth_rbo_);
                glRenderbufferStorage_ptr(GL_RENDERBUFFER, GL_DEPTH_COMPONENT24, width_, height_);
                glFramebufferRenderbuffer_ptr(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, depth_rbo_);
                glBindRenderbuffer_ptr(GL_RENDERBUFFER, 0);
            }

            // Check FBO completeness (best-effort)
            GLenum status = GL_FRAMEBUFFER_COMPLETE;
            if (glCheckFramebufferStatus_ptr) status = glCheckFramebufferStatus_ptr(GL_FRAMEBUFFER);
            if (status != GL_FRAMEBUFFER_COMPLETE) {
                std::cerr << "PostProcessor: FBO incomplete (status=" << status << ")\n";
                // cleanup FBO if incomplete
                if (glDeleteFramebuffers_ptr) glDeleteFramebuffers_ptr(1, &fbo_);
                fbo_ = 0;
            }
            glBindFramebuffer_ptr(GL_FRAMEBUFFER, 0);
        }

        // Compile default pass-through shader so rendering can work out of the box
        std::string log;
        if (!compile_shader(k_default_vert, k_default_frag, log)) {
            last_log_ = "Default shader compile failed:\n" + log;
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
    if (shader_program_ != 0) {
        if (glDeleteProgram_ptr) glDeleteProgram_ptr(shader_program_);
        shader_program_ = 0;
    }
    if (texture_ != 0) {
        glDeleteTextures(1, &texture_);
        texture_ = 0;
    }
    if (depth_rbo_ != 0 && glDeleteRenderbuffers_ptr) {
        GLuint tmp = depth_rbo_;
        glDeleteRenderbuffers_ptr(1, &tmp);
        depth_rbo_ = 0;
    }
    if (fbo_ != 0 && glDeleteFramebuffers_ptr) {
        GLuint tmp = fbo_;
        glDeleteFramebuffers_ptr(1, &tmp);
        fbo_ = 0;
    }
    initialized_ = false;
}

std::string PostProcessor::load_file(const std::string& path) {
    std::ifstream in(path.c_str(), std::ios::in | std::ios::binary);
    if (!in) return std::string();
    std::ostringstream contents;
    contents << in.rdbuf();
    return contents.str();
}

GLuint PostProcessor::compile_single_shader(GLenum type, const char* src, std::string& out_log) {
    if (!glCreateShader_ptr || !glShaderSource_ptr || !glCompileShader_ptr ||
        !glGetShaderiv_ptr || !glGetShaderInfoLog_ptr || !glDeleteShader_ptr) {
        out_log = "GL function pointers not initialized";
        return 0;
    }

    GLuint shader = glCreateShader_ptr(type);
    glShaderSource_ptr(shader, 1, &src, nullptr);
    glCompileShader_ptr(shader);

    GLint compiled = 0;
    glGetShaderiv_ptr(shader, GL_COMPILE_STATUS, &compiled);
    GLint info_len = 0;
    glGetShaderiv_ptr(shader, GL_INFO_LOG_LENGTH, &info_len);
    if (info_len > 1) {
        std::string info;
        info.resize(info_len);
        glGetShaderInfoLog_ptr(shader, info_len, nullptr, &info[0]);
        out_log += info;
    }

    if (!compiled) {
        glDeleteShader_ptr(shader);
        return 0;
    }
    return shader;
}

bool PostProcessor::compile_shader(const char* vert_src, const char* frag_src, std::string& out_log) {
    if (!init_gl_functions()) {
        out_log = "Failed to initialize GL functions";
        return false;
    }

    GLuint vert = compile_single_shader(GL_VERTEX_SHADER, vert_src, out_log);
    if (!vert) {
        out_log = "Vertex shader compile failed:\n" + out_log;
        return false;
    }

    GLuint frag = compile_single_shader(GL_FRAGMENT_SHADER, frag_src, out_log);
    if (!frag) {
        glDeleteShader_ptr(vert);
        out_log = "Fragment shader compile failed:\n" + out_log;
        return false;
    }

    GLuint program = glCreateProgram_ptr();
    if (!program) {
        glDeleteShader_ptr(vert);
        glDeleteShader_ptr(frag);
        out_log = "Failed to create shader program";
        return false;
    }

    glAttachShader_ptr(program, vert);
    glAttachShader_ptr(program, frag);
    glLinkProgram_ptr(program);

    GLint linked = 0;
    glGetProgramiv_ptr(program, GL_LINK_STATUS, &linked);
    GLint info_len = 0;
    glGetProgramiv_ptr(program, GL_INFO_LOG_LENGTH, &info_len);
    if (info_len > 1) {
        std::string info;
        info.resize(info_len);
        glGetProgramInfoLog_ptr(program, info_len, nullptr, &info[0]);
        out_log += info;
    }

    if (!linked) {
        glDeleteProgram_ptr(program);
        glDeleteShader_ptr(vert);
        glDeleteShader_ptr(frag);
        return false;
    }

    // Delete old program if any
    if (shader_program_ != 0) {
        glDeleteProgram_ptr(shader_program_);
    }
    shader_program_ = program;

    glDeleteShader_ptr(vert);
    glDeleteShader_ptr(frag);

    return true;
}

bool PostProcessor::load_shader_files(const std::string& vert_path, const std::string& frag_path) {
    std::string vert_src = load_file(vert_path);
    std::string frag_src = load_file(frag_path);

    if (vert_src.empty()) {
        last_log_ = "Failed to load vertex shader file: " + vert_path;
        return false;
    }
    if (frag_src.empty()) {
        last_log_ = "Failed to load fragment shader file: " + frag_path;
        return false;
    }

    std::string log;
    bool result = compile_shader(vert_src.c_str(), frag_src.c_str(), log);
    last_log_ = log;

    if (result) {
        // If compilation succeeded, remember paths for hot-reload
        set_watched_shader_files(vert_path, frag_path);
    }

    return result;
}

void PostProcessor::begin_capture() {
    if (!initialized_) return;
    if (fbo_ != 0 && glBindFramebuffer_ptr) {
        glBindFramebuffer_ptr(GL_FRAMEBUFFER, fbo_);
        // Clear the attached buffers
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    }
}

void PostProcessor::end_capture() {
    if (!initialized_) return;
    if (fbo_ != 0 && glBindFramebuffer_ptr) {
        glBindFramebuffer_ptr(GL_FRAMEBUFFER, 0);
    }
}

void PostProcessor::set_watched_shader_files(const std::string& vert_path, const std::string& frag_path) {
    watched_vert_ = vert_path;
    watched_frag_ = frag_path;
}

void PostProcessor::check_for_shader_reload() {
    if (!initialized_ || !shader_program_) return;
    
    // Simplified hot-reload check without filesystem::last_write_time
    // Users can manually reload by calling load_shader_files again
}

void PostProcessor::render() {
    if (!initialized_ || !enabled_ || !shader_program_) return;

    glUseProgram_ptr(shader_program_);

    GLint tex_loc = glGetUniformLocation_ptr(shader_program_, "uTexture");
    if (tex_loc != -1) {
        glUniform1i_ptr(tex_loc, 0);
    }

    GLint effect_loc = glGetUniformLocation_ptr(shader_program_, "uEffect");
    if (effect_loc != -1) {
        glUniform1i_ptr(effect_loc, effect_);
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

}  // namespace engine
