#pragma once

#include "engine/scene/scene.h"
#include <time.h>
#include <memory>

namespace engine {
namespace ecs { class Registry; }
namespace graphics { class ShaderProgram; }
namespace scene {

class BlockWorldScene : public Scene {
public:
    BlockWorldScene();
    ~BlockWorldScene() override = default;

    void init() override;
    void update(float dt) override;
    void render(OpenGLRenderer& renderer, const Camera& camera) override;

private:
    std::shared_ptr<engine::ecs::Registry> registry_;
    std::shared_ptr<engine::graphics::ShaderProgram> shader_;
};

} // namespace scene
} // namespace engine
