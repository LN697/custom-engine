#pragma once

#include "engine/scene/scene.h"
#include <memory>

namespace engine {
class TextRenderer;
namespace ecs { class Registry; }
namespace graphics { class ShaderProgram; }
namespace scene {

class LevelDemoScene : public Scene {
public:
    LevelDemoScene();
    ~LevelDemoScene() override = default;

    void init() override;
    void update(float dt) override;
    void render(OpenGLRenderer& renderer, const Camera& camera) override;

private:
    std::shared_ptr<engine::ecs::Registry> registry_;
    std::shared_ptr<engine::graphics::ShaderProgram> shader_;
    std::unique_ptr<engine::TextRenderer> text_renderer_;
};

} // namespace scene
} // namespace engine
