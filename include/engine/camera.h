#pragma once

#include "engine/input_manager.h"
#include <glm/glm.hpp>
#include <glm/gtc/quaternion.hpp>
#include <glm/gtc/matrix_transform.hpp>

namespace engine {

class Camera {
public:
    Camera();

    void update(float delta_time, const InputManager& input);

    float x() const { return position_.x; }
    float y() const { return position_.y; }
    float z() const { return position_.z; }
    float yaw() const { return yaw_; }
    float pitch() const { return pitch_; }

    void set_position(float x, float y, float z) { position_ = glm::vec3(x, y, z); }
    void set_rotation(float yaw, float pitch) { yaw_ = yaw; pitch_ = pitch; }

    void set_smoothing(float s) { move_smoothing_ = s; }
    float* smoothing_ptr() { return &move_smoothing_; }

    const glm::vec3& position() const { return position_; }
    const glm::quat& orientation() const { return orientation_; }
    glm::mat4 get_view_matrix() const;

private:
    glm::vec3 position_;
    glm::quat orientation_;
    glm::vec3 velocity_;
    float yaw_, pitch_;
    float move_smoothing_;

    static constexpr float k_movement_speed = 4.0f;
    static constexpr float k_mouse_sensitivity = 0.0025f;
    static constexpr float k_keyboard_look_speed = 1.5f;
};

} // namespace engine
