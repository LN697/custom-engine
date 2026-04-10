#pragma once

#include "engine/input/control_scheme.h"
#include <unordered_map>
#include <string>

namespace engine {

/**
 * FPS-style control scheme:
 * - WASD for movement (relative to camera direction)
 * - Mouse look for camera rotation
 * - Shift for sprint/speed modifier
 * - E for interact, Left-Click for primary action, Right-Click for secondary
 * - Classic first-person shooter controls (Quake/Half-Life style)
 */
class FPSControlScheme : public ControlScheme {
public:
    FPSControlScheme();
    ~FPSControlScheme() override = default;

    void update(const InputManager& input_manager, float delta_time) override;
    glm::vec3 get_movement_direction() const override { return movement_direction_; }
    glm::vec2 get_look_input() const override { return look_input_; }
    bool is_action_pressed(const std::string& action) const override;

    float get_speed_multiplier() const { return is_sprinting_ ? 1.5f : 1.0f; }

private:
    glm::vec3 movement_direction_;
    glm::vec2 look_input_;
    bool is_sprinting_;
    std::unordered_map<std::string, bool> actions_;
};

} // namespace engine
