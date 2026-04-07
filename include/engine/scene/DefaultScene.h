#pragma once

#include "engine/scene/Scene.h"

namespace engine {
namespace scene {

class DefaultScene : public Scene {
public:
    DefaultScene();
    void update(float dt) override;
    void render(engine::OpenGLRenderer& renderer, const engine::camera::Camera& camera) override;
};

} // namespace scene
} // namespace engine
