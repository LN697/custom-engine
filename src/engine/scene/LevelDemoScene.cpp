#include "engine/scene/level_demo_scene.h"
#include "engine/graphics/shader_program.h"
#include "engine/ecs/registry.h"
#include "engine/text_renderer.h"
#include "engine/level_loader.h"
#include "engine/scene/scene_renderer.h"
#include <iostream>

namespace engine {
namespace scene {

LevelDemoScene::LevelDemoScene() {
    registry_ = std::make_shared<engine::ecs::Registry>();
    shader_ = std::make_shared<engine::graphics::ShaderProgram>();
    text_renderer_ = std::make_unique<engine::TextRenderer>();
}

void LevelDemoScene::init() {
    std::string err;
    if (!shader_->compile_from_files("shaders/default.vert", "shaders/default.frag", err)) {
        std::cerr << "LevelDemoScene: Failed to compile shader: " << err << std::endl;
        return;
    }

    if (!engine::level::LevelLoader::load_from_file("levels/example.lvl", registry_)) {
        std::cerr << "LevelDemoScene: Failed to load levels/example.lvl" << std::endl;
    } else if (text_renderer_) {
        text_renderer_->set_text("Demo: loaded levels/example.lvl");
        text_renderer_->start();
    }
}

void LevelDemoScene::update(float dt) {
    if (text_renderer_) text_renderer_->update(dt);
}

void LevelDemoScene::render(OpenGLRenderer& renderer, const Camera& camera) {
    scene::render_registry(renderer, camera, registry_, shader_);
    if (text_renderer_) text_renderer_->render();
}

} // namespace scene
} // namespace engine
