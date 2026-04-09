#pragma once

#include "engine/renderer.h"
#include "engine/camera.h"

namespace engine {
namespace scene {

class Scene {
public:
    virtual ~Scene() {}
    virtual void init() {}  // Called when scene is set/loaded
    virtual void update(float dt) = 0;
    virtual void render(OpenGLRenderer& renderer, const Camera& camera) = 0;
};

} // namespace scene
} // namespace engine
