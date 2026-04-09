#pragma once

#include "engine/scene/scene.h"
#include "engine/ecs/registry.h"
#include "engine/graphics/shader_program.h"
#include <memory>

namespace engine {
namespace scene {

class DefaultScene : public Scene {
public:
    DefaultScene();
    void update(float dt) override;
    void render(OpenGLRenderer& renderer, const Camera& camera) override;
    
    // Initialize ECS entities and shader
    void init();

private:
    std::shared_ptr<engine::ecs::Registry> registry_;
    std::shared_ptr<engine::graphics::ShaderProgram> shader_;
    std::shared_ptr<engine::graphics::ShaderProgram> uv_shader_;
};

} // namespace scene
} // namespace engine
