#include "engine/graphics/geometry_buffer.h"
#include <iostream>
#include <cmath>
#include <SDL2/SDL.h>
#include <GL/glext.h>

// Define missing function pointer types if not available
#ifndef PFNGLDRAWELEMENTSPROC
typedef void (APIENTRYP PFNGLDRAWELEMENTSPROC)(GLenum mode, GLsizei count, GLenum type, const void *indices);
#endif
#ifndef PFNGLUSEPROGRAMPROC
typedef void (APIENTRYP PFNGLUSEPROGRAMPROC)(GLuint program);
#endif

// Function pointers (use system typedefs)
static PFNGLGENVERTEXARRAYSPROC glGenVertexArrays_ptr = nullptr;
static PFNGLBINDVERTEXARRAYPROC glBindVertexArray_ptr = nullptr;
static PFNGLDELETEVERTEXARRAYSPROC glDeleteVertexArrays_ptr = nullptr;
static PFNGLGENBUFFERSPROC glGenBuffers_ptr = nullptr;
static PFNGLBINDBUFFERPROC glBindBuffer_ptr = nullptr;
static PFNGLBUFFERDATAPROC glBufferData_ptr = nullptr;
static PFNGLDELETEBUFFERSPROC glDeleteBuffers_ptr = nullptr;
static PFNGLVERTEXATTRIBPOINTERPROC glVertexAttribPointer_ptr = nullptr;
static PFNGLENABLEVERTEXATTRIBARRAYPROC glEnableVertexAttribArray_ptr = nullptr;
static PFNGLUSEPROGRAMPROC glUseProgram_ptr = nullptr;
static PFNGLDRAWELEMENTSPROC glDrawElements_ptr = nullptr;

namespace engine {
namespace graphics {

// Initialize VAO/VBO function pointers
static bool initVAOFunctions() {
    static bool initialized = false;
    if (initialized) return true;
    
    glGenVertexArrays_ptr = (PFNGLGENVERTEXARRAYSPROC)SDL_GL_GetProcAddress("glGenVertexArrays");
    glBindVertexArray_ptr = (PFNGLBINDVERTEXARRAYPROC)SDL_GL_GetProcAddress("glBindVertexArray");
    glDeleteVertexArrays_ptr = (PFNGLDELETEVERTEXARRAYSPROC)SDL_GL_GetProcAddress("glDeleteVertexArrays");
    glGenBuffers_ptr = (PFNGLGENBUFFERSPROC)SDL_GL_GetProcAddress("glGenBuffers");
    glBindBuffer_ptr = (PFNGLBINDBUFFERPROC)SDL_GL_GetProcAddress("glBindBuffer");
    glBufferData_ptr = (PFNGLBUFFERDATAPROC)SDL_GL_GetProcAddress("glBufferData");
    glDeleteBuffers_ptr = (PFNGLDELETEBUFFERSPROC)SDL_GL_GetProcAddress("glDeleteBuffers");
    glVertexAttribPointer_ptr = (PFNGLVERTEXATTRIBPOINTERPROC)SDL_GL_GetProcAddress("glVertexAttribPointer");
    glEnableVertexAttribArray_ptr = (PFNGLENABLEVERTEXATTRIBARRAYPROC)SDL_GL_GetProcAddress("glEnableVertexAttribArray");
    glUseProgram_ptr = (PFNGLUSEPROGRAMPROC)SDL_GL_GetProcAddress("glUseProgram");
    glDrawElements_ptr = (PFNGLDRAWELEMENTSPROC)SDL_GL_GetProcAddress("glDrawElements");
    
    initialized = true;
    return true;
}

TriangleMesh create_grid_mesh(float size, int divisions) {
    TriangleMesh mesh;
    float step = size / divisions;
    float half = size * 0.5f;

    // Create grid as line segments in the XZ plane
    uint32_t index = 0;
    for (int i = 0; i <= divisions; ++i) {
        float t = -half + i * step;

        // X axis line
        mesh.vertices.insert(mesh.vertices.end(), { -half, 0.0f, t,  0.0f, 1.0f, 0.0f, 0.0f, 0.0f });
        mesh.vertices.insert(mesh.vertices.end(), {  half, 0.0f, t,  0.0f, 1.0f, 0.0f, 1.0f, 0.0f });
        mesh.indices.push_back(index);
        mesh.indices.push_back(index + 1);
        index += 2;

        // Z axis line
        mesh.vertices.insert(mesh.vertices.end(), { t, 0.0f, -half,  0.0f, 1.0f, 0.0f, 0.0f, 0.0f });
        mesh.vertices.insert(mesh.vertices.end(), { t, 0.0f,  half,  0.0f, 1.0f, 0.0f, 1.0f, 0.0f });
        mesh.indices.push_back(index);
        mesh.indices.push_back(index + 1);
        index += 2;
    }
    return mesh;
}

TriangleMesh create_triangle_mesh() {
    TriangleMesh mesh;
    mesh.vertices = {
        0.0f, 1.0f, -5.0f,   0.0f, 0.0f, 1.0f,   0.5f, 1.0f,  // top vertex
        -1.0f, 0.0f, -4.0f,  0.0f, 0.0f, 1.0f,   0.0f, 0.0f,  // bottom-left
        1.0f, 0.0f, -4.0f,   0.0f, 0.0f, 1.0f,   1.0f, 0.0f   // bottom-right
    };
    mesh.indices = { 0, 1, 2 };
    return mesh;
}

TriangleMesh create_quad_mesh() {
    TriangleMesh mesh;
    // Quad made of two triangles, UVs mapped [0,1]
    mesh.vertices = {
        -1.0f, 1.0f, -5.0f,   0.0f, 0.0f, 1.0f,   0.0f, 1.0f,  // top-left
         1.0f, 1.0f, -5.0f,   0.0f, 0.0f, 1.0f,   1.0f, 1.0f,  // top-right
         1.0f, 0.0f, -5.0f,   0.0f, 0.0f, 1.0f,   1.0f, 0.0f,  // bottom-right
        -1.0f, 0.0f, -5.0f,   0.0f, 0.0f, 1.0f,   0.0f, 0.0f   // bottom-left
    };
    mesh.indices = { 0, 1, 2, 0, 2, 3 };
    return mesh;
}

void GeometryBuffer::create(const std::vector<float>& vertices, const std::vector<uint32_t>& indices) {
    initVAOFunctions();
    if (!glGenVertexArrays_ptr) return;  // Functions not available
    
    index_count = indices.size();

    // Create VAO
    glGenVertexArrays_ptr(1, &vao);
    glBindVertexArray_ptr(vao);

    // Create VBO
    glGenBuffers_ptr(1, &vbo);
    glBindBuffer_ptr(GL_ARRAY_BUFFER, vbo);
    glBufferData_ptr(GL_ARRAY_BUFFER, vertices.size() * sizeof(float), vertices.data(), GL_STATIC_DRAW);

    // Create EBO
    glGenBuffers_ptr(1, &ebo);
    glBindBuffer_ptr(GL_ELEMENT_ARRAY_BUFFER, ebo);
    glBufferData_ptr(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(uint32_t), indices.data(), GL_STATIC_DRAW);

    // Vertex attribute layout: position (3), normal (3), uv (2)
    glVertexAttribPointer_ptr(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)0);
    glEnableVertexAttribArray_ptr(0);

    glVertexAttribPointer_ptr(1, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(3 * sizeof(float)));
    glEnableVertexAttribArray_ptr(1);

    glVertexAttribPointer_ptr(2, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(6 * sizeof(float)));
    glEnableVertexAttribArray_ptr(2);

    glBindVertexArray_ptr(0);
}

void GeometryBuffer::destroy() {
    if (!glDeleteVertexArrays_ptr || !glDeleteBuffers_ptr) return;
    if (vao) glDeleteVertexArrays_ptr(1, &vao);
    if (vbo) glDeleteBuffers_ptr(1, &vbo);
    if (ebo) glDeleteBuffers_ptr(1, &ebo);
    vao = vbo = ebo = 0;
    index_count = 0;
}

void GeometryBuffer::bind() const {
    if (glBindVertexArray_ptr) glBindVertexArray_ptr(vao);
}

void GeometryBuffer::draw(GLuint shaderProgram) const {
    if (!vao || index_count == 0) return;
    if (!glUseProgram_ptr || !glDrawElements_ptr) return;
    glUseProgram_ptr(shaderProgram);
    bind();
    glDrawElements_ptr(GL_TRIANGLES, index_count, GL_UNSIGNED_INT, nullptr);
}

} // namespace graphics
} // namespace engine
