#include "engine/scene/default_scene.h"
#include "engine/renderer.h"
#include "engine/camera.h"
#include "engine/ecs/components.h"
#include "engine/graphics/geometry_buffer.h"
#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <iostream>
#include <GL/gl.h>
#include <GL/glext.h>
#include <SDL2/SDL.h>

namespace engine {
namespace scene {

// GL function pointers for rendering
static PFNGLBINDVERTEXARRAYPROC glBindVertexArray_ptr = nullptr;

// Define missing function pointer types if not available
#ifndef PFNGLDRAWELEMENTSPROC
typedef void (APIENTRYP PFNGLDRAWELEMENTSPROC)(GLenum mode, GLsizei count, GLenum type, const void *indices);
#endif
static PFNGLDRAWELEMENTSPROC glDrawElements_ptr = nullptr;

static void init_render_functions() {
    static bool initialized = false;
    if (initialized) return;
    glBindVertexArray_ptr = (PFNGLBINDVERTEXARRAYPROC)SDL_GL_GetProcAddress("glBindVertexArray");
    glDrawElements_ptr = (PFNGLDRAWELEMENTSPROC)SDL_GL_GetProcAddress("glDrawElements");
    initialized = true;
}

DefaultScene::DefaultScene() {
    init_render_functions();
    registry_ = std::make_shared<engine::ecs::Registry>();
    shader_ = std::make_shared<engine::graphics::ShaderProgram>();
    uv_shader_ = std::make_shared<engine::graphics::ShaderProgram>();
}

void DefaultScene::init() {
    // Compile shader
    std::string shader_error;
    if (!shader_->compile_from_files("shaders/default.vert", "shaders/default.frag", shader_error)) {
        std::cerr << "DefaultScene: Failed to compile shader: " << shader_error << std::endl;
        return;
    }

    // Create grid entity
    engine::ecs::Entity grid_entity = registry_->create();
    auto& grid_transform = registry_->emplace<engine::ecs::TransformComponent>(grid_entity);
    grid_transform.position = glm::vec3(0.0f, -2.0f, 0.0f);
    grid_transform.scale = glm::vec3(1.0f, 1.0f, 1.0f);
    
    // Create grid mesh
    auto grid_mesh = engine::graphics::create_grid_mesh(20.0f, 10);
    auto& grid_geom = registry_->emplace<engine::ecs::MeshComponent>(grid_entity);
    grid_geom.shader_program = shader_->get_program();
    grid_geom.color = glm::vec3(0.15f, 0.35f, 0.6f); // Dark blue
    grid_geom.draw_mode = GL_LINES;
    engine::graphics::GeometryBuffer grid_buffer;
    grid_buffer.create(grid_mesh.vertices, grid_mesh.indices);
    grid_geom.vao = grid_buffer.vao;
    grid_geom.vbo = grid_buffer.vbo;
    grid_geom.ebo = grid_buffer.ebo;
    grid_geom.index_count = grid_mesh.indices.size();
    
    // Create a quad entity (two triangles) for UV demo
    // Compile a dedicated UV shader to visualize UV coordinates directly
    std::string uv_err;
    if (!uv_shader_->compile_from_files("shaders/default.vert", "shaders/uv_debug.frag", uv_err)) {
        std::cerr << "DefaultScene: Failed to compile UV shader: " << uv_err << std::endl;
    }

    engine::ecs::Entity quad_entity = registry_->create();
    auto& quad_transform = registry_->emplace<engine::ecs::TransformComponent>(quad_entity);
    quad_transform.position = glm::vec3(0.0f, 2.0f, -5.0f);
    quad_transform.scale = glm::vec3(2.0f, 2.0f, 2.0f);

    auto quad_mesh = engine::graphics::create_quad_mesh();
    auto& quad_geom = registry_->emplace<engine::ecs::MeshComponent>(quad_entity);
    quad_geom.shader_program = uv_shader_->get_program();
    quad_geom.color = glm::vec3(1.0f, 1.0f, 1.0f);
    engine::graphics::GeometryBuffer quad_buffer;
    quad_buffer.create(quad_mesh.vertices, quad_mesh.indices);
    quad_geom.vao = quad_buffer.vao;
    quad_geom.vbo = quad_buffer.vbo;
    quad_geom.ebo = quad_buffer.ebo;
    quad_geom.index_count = quad_mesh.indices.size();
    quad_geom.draw_mode = GL_TRIANGLES;
}

void DefaultScene::update(float /*dt*/) {
    // Default scene is static for now
}

void DefaultScene::render(OpenGLRenderer& renderer, const Camera& camera) {
    // Clear the screen
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    
    if (!shader_->is_valid()) {
        std::cerr << "DefaultScene: Shader not valid!" << std::endl;
        return;
    }
    if (!glBindVertexArray_ptr || !glDrawElements_ptr) {
        std::cerr << "DefaultScene: GL function pointers not initialized!" << std::endl;
        return;
    }
    
    shader_->use();
    
    // Set camera matrices
    glm::mat4 view = camera.get_view_matrix();
    glm::mat4 projection = glm::perspective(
        glm::radians(45.0f),
        1280.0f / 720.0f,  // aspect ratio
        0.1f,
        1000.0f
    );
    
    shader_->set_mat4("uView", glm::value_ptr(view));
    shader_->set_mat4("uProjection", glm::value_ptr(projection));
    
    // Iterate through all entities with TransformComponent and render if they also have MeshComponent
    registry_->view<engine::ecs::TransformComponent>(
        [this](engine::ecs::Entity entity, engine::ecs::TransformComponent& transform) {
            auto* mesh = registry_->try_get<engine::ecs::MeshComponent>(entity);
            if (!mesh || mesh->vao == 0 || mesh->index_count == 0) {
                return;
            }
            
            // Set model matrix
            glm::mat4 model = transform.get_transform_matrix();
            shader_->set_mat4("uModel", glm::value_ptr(model));
            
            // Set per-entity color
            shader_->set_vec3("uColor", mesh->color.x, mesh->color.y, mesh->color.z);
            
            // Render the mesh
            if (glBindVertexArray_ptr && glDrawElements_ptr) {
                glBindVertexArray_ptr(mesh->vao);
                if (mesh->draw_mode == GL_LINES) {
                    glLineWidth(1.0f);
                }
                glDrawElements_ptr(mesh->draw_mode, mesh->index_count, GL_UNSIGNED_INT, nullptr);
            }
        }
    );
}

} // namespace scene
} // namespace engine
