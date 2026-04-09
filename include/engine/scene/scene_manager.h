#pragma once

#include <memory>
#include "engine/scene/scene.h"

namespace engine {
namespace scene {

class SceneManager {
public:
    SceneManager();
    void set_scene(std::unique_ptr<Scene> s);
    void update(float dt);
    void render(OpenGLRenderer& renderer, const Camera& camera);

private:
    std::unique_ptr<Scene> current_;
};

} // namespace scene
} // namespace engine
