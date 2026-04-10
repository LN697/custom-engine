#pragma once

#include <sys/types.h>
#include <time.h>
#include <memory>

namespace engine {
namespace ecs { class Registry; }
namespace scene {

class SceneBuilder {
public:
    virtual ~SceneBuilder() = default;
    virtual void build(std::shared_ptr<engine::ecs::Registry> registry) = 0;
};

} // namespace scene
} // namespace engine
