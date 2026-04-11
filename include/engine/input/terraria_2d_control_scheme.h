#pragma once

#include "engine/input/control_scheme.h"
#include <glm/glm.hpp>

namespace engine {

class Terraria2DControlScheme : public ControlScheme {
public:
    Terraria2DControlScheme();
    ~Terraria2DControlScheme() override = default;

    void update(const InputManager& input, float delta_time) override;

    glm::vec3 get_movement_direction() const override;
    glm::vec2 get_look_input() const override;
    bool is_action_pressed(const std::string& action) const override;

    glm::vec2 get_movement_input() const { return movement_; }
    bool is_jump_pressed() const { return jump_pressed_; }

private:
    glm::vec2 movement_;
    bool jump_pressed_;
};

} // namespace engine
