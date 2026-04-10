#include "engine/input/minecraft_control_scheme.h"
#include "engine/input_manager.h"

namespace engine {

MinecraftControlScheme::MinecraftControlScheme()
    : movement_direction_(0.0f), look_input_(0.0f) {
}

void MinecraftControlScheme::update(const InputManager& input_manager, float delta_time) {
    // Reset movement direction each frame
    movement_direction_ = glm::vec3(0.0f);

    // Horizontal movement (W, A, S, D)
    if (input_manager.move_forward()) {
        movement_direction_.z += 1.0f;  // Forward
    }
    if (input_manager.move_backward()) {
        movement_direction_.z -= 1.0f;  // Backward
    }
    if (input_manager.move_left()) {
        movement_direction_.x -= 1.0f;  // Left strafe
    }
    if (input_manager.move_right()) {
        movement_direction_.x += 1.0f;  // Right strafe
    }

    // Vertical movement (Space for up, Ctrl for down)
    if (input_manager.move_up()) {
        movement_direction_.y += 1.0f;  // Up
    }
    if (input_manager.move_down()) {
        movement_direction_.y -= 1.0f;  // Down
    }

    // Normalize movement direction to prevent faster diagonal movement
    if (glm::length(movement_direction_) > 0.0f) {
        movement_direction_ = glm::normalize(movement_direction_);
    }

    // Look input from arrow keys (for alternative control)
    look_input_.x = static_cast<float>(input_manager.look_right() - input_manager.look_left());
    look_input_.y = static_cast<float>(input_manager.look_down() - input_manager.look_up());

    // Update actions
    actions_["jump"] = input_manager.move_up();
    actions_["crouch"] = input_manager.move_down();
    actions_["sprint"] = false;  // Minecraft uses double-tap, simplified here
}

bool MinecraftControlScheme::is_action_pressed(const std::string& action) const {
    auto it = actions_.find(action);
    return it != actions_.end() ? it->second : false;
}

} // namespace engine
