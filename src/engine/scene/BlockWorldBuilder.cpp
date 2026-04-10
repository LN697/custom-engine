#define _POSIX_C_SOURCE 200809L
#define _REENTRANT
#include <time.h>
#include "engine/scene/block_world_builder.h"
#include "engine/graphics/terrain_mesh.h"
#include "engine/graphics/geometry_buffer.h"
#include "engine/ecs/components.h"
#include "engine/ecs/registry.h"
#include <glm/glm.hpp>
#include <random>
#include <cmath>

namespace engine {
namespace scene {

BlockWorldBuilder::BlockWorldBuilder(int width, int depth)
    : width_(width), depth_(depth) {}

void BlockWorldBuilder::build(std::shared_ptr<engine::ecs::Registry> registry) {
    // Simple deterministic procedural height function using sines/cosines to avoid external deps.
    auto height_fn = [this](int x, int z) -> int {
        const float fx = static_cast<float>(x) * 0.08f;
        const float fz = static_cast<float>(z) * 0.08f;
        float h = (std::sin(fx * 1.0f) + std::cos(fz * 1.0f) * 0.5f)
                  + 0.5f * std::sin(fx * 0.5f + fz * 0.25f);
        h = h * 3.5f + 5.0f; // scale and bias
        int ih = static_cast<int>(std::floor(h));
        if (ih < 0) ih = 0;
        if (ih > 16) ih = 16;
        return ih;
    };

    // Create merged solid mesh
    auto solid_mesh = engine::graphics::create_terrain_mesh(width_, depth_, height_fn);
    engine::graphics::GeometryBuffer solid_buffer;
    solid_buffer.create(solid_mesh.vertices, solid_mesh.indices);

    // Create wireframe mesh
    auto wire_mesh = engine::graphics::create_terrain_wireframe(width_, depth_, height_fn);
    engine::graphics::GeometryBuffer wire_buffer;
    wire_buffer.create(wire_mesh.vertices, wire_mesh.indices);

    // Add solid entity
    engine::ecs::Entity solid_ent = registry->create();
    auto& solid_transform = registry->emplace<engine::ecs::TransformComponent>(solid_ent);
    solid_transform.position = glm::vec3(0.0f, 0.0f, 0.0f);
    auto& solid_mesh_comp = registry->emplace<engine::ecs::MeshComponent>(solid_ent);
    solid_mesh_comp.vao = solid_buffer.vao;
    solid_mesh_comp.vbo = solid_buffer.vbo;
    solid_mesh_comp.ebo = solid_buffer.ebo;
    solid_mesh_comp.index_count = solid_mesh.indices.size();
    solid_mesh_comp.draw_mode = GL_TRIANGLES;
    solid_mesh_comp.color = glm::vec3(0.6f, 0.8f, 0.3f); // grassy tone

    // Add wireframe entity
    engine::ecs::Entity wire_ent = registry->create();
    auto& wire_transform = registry->emplace<engine::ecs::TransformComponent>(wire_ent);
    wire_transform.position = glm::vec3(0.0f, 0.0f, 0.0f);
    auto& wire_mesh_comp = registry->emplace<engine::ecs::MeshComponent>(wire_ent);
    wire_mesh_comp.vao = wire_buffer.vao;
    wire_mesh_comp.vbo = wire_buffer.vbo;
    wire_mesh_comp.ebo = wire_buffer.ebo;
    wire_mesh_comp.index_count = wire_mesh.indices.size();
    wire_mesh_comp.draw_mode = GL_LINES;
    wire_mesh_comp.color = glm::vec3(0.05f, 0.05f, 0.05f); // dark outline
}

} // namespace scene
} // namespace engine
