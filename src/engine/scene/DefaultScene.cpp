#include "engine/scene/DefaultScene.h"
#include "engine/renderer/OpenGLRenderer.h"
#include "engine/camera/Camera.h"

namespace engine {
namespace scene {

DefaultScene::DefaultScene() {}

void DefaultScene::update(float /*dt*/) {
    // Default scene is static for now
}

void DefaultScene::render(engine::OpenGLRenderer& renderer, const engine::camera::Camera& camera) {
    renderer.render(camera.x(), camera.y(), camera.z(), camera.yaw(), camera.pitch());
}

} // namespace scene
} // namespace engine
