#pragma once

#include <GL/gl.h>
#include <vector>
#include <glm/glm.hpp>

namespace engine {
namespace graphics {

// Helper to create a simple triangle mesh
struct TriangleMesh {
    std::vector<float> vertices;  // x, y, z, nx, ny, nz, u, v (8 floats per vertex)
    std::vector<uint32_t> indices;
};

// Helper to create a simple grid in the XZ plane
TriangleMesh create_grid_mesh(float size, int divisions);

// Helper to create a triangle
TriangleMesh create_triangle_mesh();
// Helper to create a quad (two triangles) for UV demo
TriangleMesh create_quad_mesh();

// VAO/VBO creation helper
struct GeometryBuffer {
    GLuint vao = 0;
    GLuint vbo = 0;
    GLuint ebo = 0;
    uint32_t index_count = 0;

    void create(const std::vector<float>& vertices, const std::vector<uint32_t>& indices);
    void destroy();
    void bind() const;
    void draw(GLuint shaderProgram) const;
};

} // namespace graphics
} // namespace engine
