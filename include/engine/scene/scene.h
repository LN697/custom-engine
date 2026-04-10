#pragma once

#include "engine/renderer.h"
#include "engine/camera.h"
#include <memory>

namespace engine {
class ControlScheme;

namespace scene {

class Scene {
public:
    virtual ~Scene() {}
    virtual void init() {}  // Called when scene is set/loaded
    virtual void update(float dt) = 0;
    virtual void render(OpenGLRenderer& renderer, const Camera& camera) = 0;

    /**
     * Set the control scheme for this scene.
     * Allows dynamic switching between Minecraft, FPS, Doom-like controls, etc.
     */
    virtual void set_control_scheme(std::shared_ptr<ControlScheme> control_scheme) {
        control_scheme_ = control_scheme;
    }

    /**
     * Get the control scheme for this scene.
     */
    virtual std::shared_ptr<ControlScheme> get_control_scheme() const {
        return control_scheme_;
    }

protected:
    std::shared_ptr<ControlScheme> control_scheme_;
};

} // namespace scene
} // namespace engine
