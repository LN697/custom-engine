#pragma once

#include <glm/glm.hpp>
#include <string>

namespace engine {

class InputManager;

/**
 * Abstract base class for control schemes.
 * 
 * A control scheme interprets raw input from the InputManager
 * and translates it into game-specific actions and camera movements.
 */
class ControlScheme {
public:
    virtual ~ControlScheme() = default;

    /**
     * Update the control scheme based on current input state.
     * @param input_manager The input manager with current key states
     * @param delta_time Time elapsed since last update
     */
    virtual void update(const InputManager& input_manager, float delta_time) = 0;

    // Movement queries
    virtual glm::vec3 get_movement_direction() const = 0;

    // Camera/Look queries
    virtual glm::vec2 get_look_input() const = 0;

    // Action queries
    virtual bool is_action_pressed(const std::string& action) const = 0;
};

} // namespace engine
