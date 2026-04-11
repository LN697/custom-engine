#include "engine/input/terraria_2d_control_scheme.h"
#include "engine/input_manager.h"

namespace engine {

Terraria2DControlScheme::Terraria2DControlScheme()
    : movement_(0.0f, 0.0f), jump_pressed_(false) {}

void Terraria2DControlScheme::update(const InputManager& input, float /*delta_time*/) {
    movement_ = glm::vec2(0.0f, 0.0f);
    jump_pressed_ = false;

    // Horizontal movement: A/D keys
    if (input.move_left()) {
        movement_.x = -1.0f;
    }
    if (input.move_right()) {
        movement_.x = 1.0f;
    }

    // Jump: Space or W key
    if (input.move_up() || input.move_forward()) {
        jump_pressed_ = true;
    }
}

glm::vec3 Terraria2DControlScheme::get_movement_direction() const {
    // Convert 2D movement to 3D (for compatibility with the 3D camera system)
    return glm::vec3(movement_.x, 0.0f, movement_.y);
}

glm::vec2 Terraria2DControlScheme::get_look_input() const {
    // No look input in 2D Terraria mode
    return glm::vec2(0.0f, 0.0f);
}

bool Terraria2DControlScheme::is_action_pressed(const std::string& action) const {
    if (action == "jump") {
        return jump_pressed_;
    }
    return false;
}

} // namespace engine
