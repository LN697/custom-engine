#pragma once


namespace engine {
class OpenGLRenderer;
namespace camera { class Camera; }
namespace scene {

class Scene {
public:
    virtual ~Scene() {}
    virtual void update(float dt) = 0;
    virtual void render(engine::OpenGLRenderer& renderer, const engine::camera::Camera& camera) = 0;
};

} // namespace scene
} // namespace engine
