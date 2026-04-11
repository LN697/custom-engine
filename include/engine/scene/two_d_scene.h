#pragma once

#include "engine/scene/scene.h"
#include "engine/text_renderer.h"
#include "engine/camera/camera2d.h"
#include "engine/scene/terraria_world.h"
#include "engine/scene/player.h"
#include <memory>

namespace engine {
namespace ecs { class Registry; }
namespace graphics { class ShaderProgram; }
namespace scene {

class TwoDScene : public Scene {
public:
    TwoDScene();
    ~TwoDScene() override = default;

    void init() override;
    void update(float dt) override;
    void render(OpenGLRenderer& renderer, const Camera& camera) override;

private:
    std::shared_ptr<engine::ecs::Registry> registry_;
    std::shared_ptr<engine::graphics::ShaderProgram> shader_;
    std::unique_ptr<engine::TextRenderer> text_renderer_;
    std::unique_ptr<engine::Camera2D> camera2d_;
    std::unique_ptr<TerrariaWorld> world_;
    std::unique_ptr<Player> player_;
};

} // namespace scene
} // namespace engine
