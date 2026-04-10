#pragma once

#include <sys/types.h>
#include <time.h>
#include <memory>
#include "engine/renderer.h"
#include "engine/camera.h"
#include "engine/ecs/registry.h"
#include "engine/graphics/shader_program.h"

namespace engine {
namespace scene {

// Render all mesh-bearing entities from a registry using the provided shader.
void render_registry(OpenGLRenderer& renderer,
                     const Camera& camera,
                     std::shared_ptr<engine::ecs::Registry> registry,
                     std::shared_ptr<engine::graphics::ShaderProgram> shader);

} // namespace scene
} // namespace engine
