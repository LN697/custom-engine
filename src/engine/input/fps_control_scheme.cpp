#include "engine/input/fps_control_scheme.h"
#include "engine/input_manager.h"

namespace engine {

FPSControlScheme::FPSControlScheme()
    : movement_direction_(0.0f), look_input_(0.0f), is_sprinting_(false) {
}

void FPSControlScheme::update(const InputManager& input_manager, float delta_time) {
    // Reset movement direction each frame
    movement_direction_ = glm::vec3(0.0f);

    // WASD movement (relative to camera direction)
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

    // Space for jump (no vertical movement in FPS style, jump is action-based)
    bool jump_pressed = input_manager.move_up();

    // Normalize movement direction
    if (glm::length(movement_direction_) > 0.0f) {
        movement_direction_ = glm::normalize(movement_direction_);
    }

    // Look input from arrow keys
    look_input_.x = static_cast<float>(input_manager.look_right() - input_manager.look_left());
    look_input_.y = static_cast<float>(input_manager.look_down() - input_manager.look_up());

    // FPS-specific: Sprint (Shift key) - typically detected via key state
    // Note: InputManager doesn't expose Shift directly, could be extended
    is_sprinting_ = false;  // TODO: Wire up Shift key when InputManager is extended

    // Update actions
    actions_["jump"] = jump_pressed;
    actions_["interact"] = false;  // TODO: Wire up 'E' key
    actions_["primary_fire"] = false;  // TODO: Wire up left-click
    actions_["secondary_fire"] = false;  // TODO: Wire up right-click
    actions_["sprint"] = is_sprinting_;
}

bool FPSControlScheme::is_action_pressed(const std::string& action) const {
    auto it = actions_.find(action);
    return it != actions_.end() ? it->second : false;
}

} // namespace engine
