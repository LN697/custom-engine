#pragma once

#include "engine/input/control_scheme.h"
#include <unordered_map>
#include <string>

namespace engine {

/**
 * Doom-style control scheme:
 * - WASD for movement (independent of view direction, strafe-based)
 * - Arrow keys for turning (alternative to mouse)
 * - Mouse look for vertical aim (up/down)
 * - Space for jump, Ctrl for crouch
 * - Classic 90s FPS with decoupled movement and view
 */
class DoomControlScheme : public ControlScheme {
public:
    DoomControlScheme();
    ~DoomControlScheme() override = default;

    void update(const InputManager& input_manager, float delta_time) override;
    glm::vec3 get_movement_direction() const override { return movement_direction_; }
    glm::vec2 get_look_input() const override { return look_input_; }
    bool is_action_pressed(const std::string& action) const override;

private:
    glm::vec3 movement_direction_;
    glm::vec2 look_input_;
    std::unordered_map<std::string, bool> actions_;
};

} // namespace engine
