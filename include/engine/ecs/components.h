#pragma once

#include <glm/glm.hpp>
#include <GL/gl.h>
#include <cstdint>

namespace engine::ecs {

struct TransformComponent {
    glm::vec3 position = glm::vec3(0.0f);
    glm::vec3 rotation = glm::vec3(0.0f);
    glm::vec3 scale = glm::vec3(1.0f);

    glm::mat4 get_transform_matrix() const;
};

struct MeshComponent {
    GLuint vao = 0;
    GLuint vbo = 0;
    GLuint ebo = 0;
    uint32_t index_count = 0;
    GLuint shader_program = 0;
    GLenum draw_mode = GL_TRIANGLES;
    glm::vec3 color = glm::vec3(1.0f, 1.0f, 1.0f);
};

struct AxesComponent {};
struct GridComponent {};

} // namespace engine::ecs
