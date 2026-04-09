#define _GNU_SOURCE
#define __USE_POSIX
#include <sys/types.h>
#include <time.h>
#include "engine/scene/scene_manager.h"

namespace engine {
namespace scene {

SceneManager::SceneManager() : current_(nullptr) {}

void SceneManager::set_scene(std::unique_ptr<Scene> s) {
    current_ = std::move(s);
    if (current_) {
        current_->init();
    }
}

void SceneManager::update(float dt) {
    if (current_) current_->update(dt);
}

void SceneManager::render(OpenGLRenderer& renderer, const Camera& camera) {
    if (current_) current_->render(renderer, camera);
}

} // namespace scene
} // namespace engine
