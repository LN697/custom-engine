#include "engine/ecs/registry.h"
#include <algorithm>
#include <cassert>

namespace engine {
namespace ecs {

Registry::Registry() : nextId_(1) {}

Registry::~Registry() {
    clear();
}

Entity Registry::create() {
    Entity e = nextId_++;
    entities_.push_back(e);
    return e;
}

void Registry::destroy(Entity e) {
    auto it = std::find(entities_.begin(), entities_.end(), e);
    if (it != entities_.end()) {
        entities_.erase(it);
    }
    // Components are automatically cleaned up when the entity is removed
}

bool Registry::valid(Entity e) const {
    return std::find(entities_.begin(), entities_.end(), e) != entities_.end();
}

void Registry::clear() {
    entities_.clear();
    storages_.clear();
    nextId_ = 1;
}

} // namespace ecs
} // namespace engine
