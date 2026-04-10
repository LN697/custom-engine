#pragma once

#include <sys/types.h>
#include <time.h>
#include "engine/scene/scene_builder.h"
#include <memory>

namespace engine {
namespace ecs { class Registry; }
namespace scene {

class BlockWorldBuilder : public SceneBuilder {
public:
    BlockWorldBuilder(int width = 200, int depth = 200);
    void build(std::shared_ptr<engine::ecs::Registry> registry) override;

private:
    int width_;
    int depth_;
};

} // namespace scene
} // namespace engine
