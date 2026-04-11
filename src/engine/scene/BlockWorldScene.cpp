#include "engine/scene/block_world_scene.h"
#include "engine/scene/block_world_builder.h"
#include "engine/scene/scene_renderer.h"
#include "engine/graphics/shader_program.h"
#include "engine/ecs/registry.h"
#include "engine/text_renderer.h"
#include <iostream>

namespace engine {
namespace scene {

BlockWorldScene::BlockWorldScene() {
    registry_ = std::make_shared<engine::ecs::Registry>();
    shader_ = std::make_shared<engine::graphics::ShaderProgram>();
    text_renderer_ = std::make_unique<engine::TextRenderer>();
}

void BlockWorldScene::init() {
    std::string err;
    if (!shader_->compile_from_files("shaders/default.vert", "shaders/default.frag", err)) {
        std::cerr << "BlockWorldScene: Failed to compile shader: " << err << std::endl;
        return;
    }

    // Build terrain via builder (keeps construction separate from rendering)
    auto builder = std::make_shared<BlockWorldBuilder>(200, 200);
    builder->build(registry_);

    (void)registry_; // keep compiler happy if unused here

    // Intro text for BlockWorld (Minecraft-style)
    if (text_renderer_) {
        text_renderer_->set_text("Welcome to BlockWorld! Use WASD to move, Space to jump.");
        text_renderer_->start();
    }
}

void BlockWorldScene::update(float dt) {
    // Update any scene-local systems
    if (text_renderer_) text_renderer_->update(dt);
}

void BlockWorldScene::render(OpenGLRenderer& renderer, const Camera& camera) {
    scene::render_registry(renderer, camera, registry_, shader_);
    // Render intro text via ImGui
    if (text_renderer_) text_renderer_->render();
}

} // namespace scene
} // namespace engine
