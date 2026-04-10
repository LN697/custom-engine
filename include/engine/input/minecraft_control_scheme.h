#pragma once

#include "engine/input/control_scheme.h"
#include <unordered_map>
#include <string>

namespace engine {

/**
 * Minecraft-style control scheme:
 * - WASD for movement (relative to camera direction)
 * - Space for up, Ctrl for down (vertical flight controls)
 * - Mouse look for camera rotation
 * - Traditional first-person survival controls
 */
class MinecraftControlScheme : public ControlScheme {
public:
    MinecraftControlScheme();
    ~MinecraftControlScheme() override = default;

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
