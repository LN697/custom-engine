#include "engine/input/doom_control_scheme.h"
#include "engine/input_manager.h"

namespace engine {

DoomControlScheme::DoomControlScheme()
    : movement_direction_(0.0f), look_input_(0.0f) {
}

void DoomControlScheme::update(const InputManager& input_manager, float delta_time) {
    // Doom-style: Movement is independent of view (true strafe-based)
    movement_direction_ = glm::vec3(0.0f);

    // WASD movement
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

    // Normalize movement direction
    if (glm::length(movement_direction_) > 0.0f) {
        movement_direction_ = glm::normalize(movement_direction_);
    }

    // Doom-style: View rotation from arrow keys (primary) or mouse (mouse look is vertical only)
    look_input_.x = static_cast<float>(input_manager.look_right() - input_manager.look_left());
    look_input_.y = static_cast<float>(input_manager.look_down() - input_manager.look_up());

    // Doom-style actions: Jump and Crouch
    actions_["jump"] = input_manager.move_up();
    actions_["crouch"] = input_manager.move_down();
    actions_["use"] = false;  // TODO: Wire up 'E' key for using objects
    actions_["fire"] = false;  // TODO: Wire up left-click for weapon fire
}

bool DoomControlScheme::is_action_pressed(const std::string& action) const {
    auto it = actions_.find(action);
    return it != actions_.end() ? it->second : false;
}

} // namespace engine
