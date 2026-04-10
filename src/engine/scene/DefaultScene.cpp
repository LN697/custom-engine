#include "engine/scene/default_scene.h"
#include "engine/renderer.h"
#include "engine/camera.h"
#include "engine/ecs/components.h"
#include "engine/graphics/geometry_buffer.h"
#include "engine/scene/scene_renderer.h"
#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <iostream>
#include <GL/gl.h>
#include <SDL2/SDL.h>

namespace engine {
namespace scene {


DefaultScene::DefaultScene() {
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
    scene::render_registry(renderer, camera, registry_, shader_);
}

} // namespace scene
} // namespace engine
