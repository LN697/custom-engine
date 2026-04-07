#pragma once

#include <memory>
#include "engine/scene/Scene.h"

namespace engine {
namespace scene {

class SceneManager {
public:
    SceneManager();
    void setScene(std::unique_ptr<Scene> s);
    void update(float dt);
    void render(engine::OpenGLRenderer& renderer, const engine::camera::Camera& camera);

private:
    std::unique_ptr<Scene> current_;
};

} // namespace scene
} // namespace engine
