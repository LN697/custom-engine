#include "engine/scene/SceneManager.h"

namespace engine {
namespace scene {

SceneManager::SceneManager() : current_(nullptr) {}

void SceneManager::setScene(std::unique_ptr<Scene> s) {
    current_ = std::move(s);
}

void SceneManager::update(float dt) {
    if (current_) current_->update(dt);
}

void SceneManager::render(engine::OpenGLRenderer& renderer, const engine::camera::Camera& camera) {
    if (current_) current_->render(renderer, camera);
}

} // namespace scene
} // namespace engine
