#define _POSIX_C_SOURCE 200809L
#define _REENTRANT
#include <time.h>
#include "engine/scene/scene_renderer.h"
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <iostream>
#include <SDL2/SDL.h>
#include <GL/gl.h>

// GL function pointer typedefs
#ifndef PFNGLDRAWELEMENTSPROC
typedef void (APIENTRYP PFNGLDRAWELEMENTSPROC)(GLenum mode, GLsizei count, GLenum type, const void *indices);
#endif
#ifndef PFNGLBINDVERTEXARRAYPROC
typedef void (APIENTRYP PFNGLBINDVERTEXARRAYPROC)(GLuint array);
#endif

static PFNGLBINDVERTEXARRAYPROC glBindVertexArray_ptr = nullptr;
static PFNGLDRAWELEMENTSPROC glDrawElements_ptr = nullptr;

static void init_render_functions() {
    static bool initialized = false;
    if (initialized) return;
    glBindVertexArray_ptr = (PFNGLBINDVERTEXARRAYPROC)SDL_GL_GetProcAddress("glBindVertexArray");
    glDrawElements_ptr = (PFNGLDRAWELEMENTSPROC)SDL_GL_GetProcAddress("glDrawElements");
    initialized = true;
}

namespace engine {
namespace scene {

void render_registry(OpenGLRenderer& renderer,
                     const Camera& camera,
                     std::shared_ptr<engine::ecs::Registry> registry,
                     std::shared_ptr<engine::graphics::ShaderProgram> shader) {
    // Clear framebuffer
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    if (!shader || !shader->is_valid()) {
        std::cerr << "scene_renderer: shader not valid" << std::endl;
        return;
    }

    init_render_functions();
    if (!glBindVertexArray_ptr || !glDrawElements_ptr) {
        std::cerr << "scene_renderer: GL functions not available" << std::endl;
        return;
    }

    shader->use();

    glm::mat4 view = camera.get_view_matrix();
    float aspect = (renderer.height() > 0) ? static_cast<float>(renderer.width()) / static_cast<float>(renderer.height()) : 1.0f;
    glm::mat4 projection = glm::perspective(glm::radians(45.0f), aspect, 0.1f, 1000.0f);

    shader->set_mat4("uView", glm::value_ptr(view));
    shader->set_mat4("uProjection", glm::value_ptr(projection));

    registry->view<engine::ecs::TransformComponent>(
        [&](engine::ecs::Entity entity, engine::ecs::TransformComponent& transform) {
            auto* mesh = registry->try_get<engine::ecs::MeshComponent>(entity);
            if (!mesh || mesh->vao == 0 || mesh->index_count == 0) return;

            glm::mat4 model = transform.get_transform_matrix();
            shader->set_mat4("uModel", glm::value_ptr(model));
            shader->set_vec3("uColor", mesh->color.x, mesh->color.y, mesh->color.z);

            if (glBindVertexArray_ptr && glDrawElements_ptr) {
                glBindVertexArray_ptr(mesh->vao);
                if (mesh->draw_mode == GL_LINES) {
                    // Use thicker lines with antialiasing enabled
                    glLineWidth(1.5f);
                }

                // Avoid z-fighting by offsetting filled polygons slightly.
                // Enable polygon offset for triangles, draw, then disable.
                bool usedOffset = false;
                if (mesh->draw_mode == GL_TRIANGLES) {
                    usedOffset = true;
                    glEnable(GL_POLYGON_OFFSET_FILL);
                    glPolygonOffset(1.0f, 1.0f);
                }

                glDrawElements_ptr(mesh->draw_mode, mesh->index_count, GL_UNSIGNED_INT, nullptr);

                if (usedOffset) {
                    glDisable(GL_POLYGON_OFFSET_FILL);
                    glPolygonOffset(0.0f, 0.0f);
                }
            }
        }
    );
}

} // namespace scene
} // namespace engine
