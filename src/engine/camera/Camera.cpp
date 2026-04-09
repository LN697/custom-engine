#include "engine/camera.h"
#include <SDL2/SDL.h>
#include <algorithm>
#include <cmath>

namespace engine {

Camera::Camera()
    : position_(0.0f, 1.6f, 4.0f), orientation_(1.0f, 0.0f, 0.0f, 0.0f), velocity_(0.0f),
      yaw_(0.0f), pitch_(0.0f), move_smoothing_(12.0f) {}

void Camera::update(float delta_time, const InputManager& input) {
    // Movement inputs
    float forward_input = 0.0f;
    float strafe_input = 0.0f;
    float rise_input = 0.0f;

    if (input.move_forward())  forward_input += 1.0f;
    if (input.move_backward()) forward_input -= 1.0f;
    if (input.move_right())    strafe_input += 1.0f;
    if (input.move_left())     strafe_input -= 1.0f;
    if (input.move_up())       rise_input += 1.0f;
    if (input.move_down())     rise_input -= 1.0f;

    float len = std::sqrt(forward_input * forward_input + strafe_input * strafe_input + rise_input * rise_input);
    if (len > 0.0f) {
        forward_input /= len;
        strafe_input /= len;
        rise_input /= len;
    }

    // Mouse look (use SDL relative state for robust behavior)
    int mouse_dx = 0, mouse_dy = 0;
    SDL_GetRelativeMouseState(&mouse_dx, &mouse_dy);

    // Mouse look (SDL yrel is positive when moving down). Use additive mapping
    // so moving the mouse up (negative yrel) reduces pitch and looks up.
    yaw_ += mouse_dx * k_mouse_sensitivity;
    pitch_ += mouse_dy * k_mouse_sensitivity;

    float yaw_input = static_cast<float>(input.look_right() - input.look_left());
    float pitch_input = static_cast<float>(input.look_down() - input.look_up());

    (void)yaw_input; (void)pitch_input;

    // Keyboard look: arrow keys should match mouse conventions (Up -> look up)
    yaw_ += yaw_input * k_keyboard_look_speed * delta_time;
    pitch_ += pitch_input * k_keyboard_look_speed * delta_time;

    // Clamp pitch
    pitch_ = std::fmax(std::fmin(pitch_, 1.5f), -1.5f);

    // Compute direction vectors directly from yaw/pitch (trigonometric form)
    float cos_yaw = std::cos(yaw_);
    float sin_yaw = std::sin(yaw_);
    float cos_pitch = std::cos(pitch_);
    float sin_pitch = std::sin(pitch_);

    glm::vec3 forward = glm::vec3(sin_yaw * cos_pitch, -sin_pitch, -cos_yaw * cos_pitch);
    glm::vec3 right = glm::vec3(cos_yaw, 0.0f, sin_yaw);
    glm::vec3 up = glm::normalize(glm::cross(right, forward));

    glm::vec3 desired_vel = (forward * forward_input + right * strafe_input + up * rise_input) * k_movement_speed;

    // Exponential smoothing of velocity
    float k = move_smoothing_;
    float t = 1.0f - std::exp(-k * delta_time);
    velocity_ += (desired_vel - velocity_) * t;

    // Integrate
    position_ += velocity_ * delta_time;
}

glm::mat4 Camera::get_view_matrix() const {
    // Compute forward/up directly from yaw/pitch (same as in update())
    float cos_yaw = std::cos(yaw_);
    float sin_yaw = std::sin(yaw_);
    float cos_pitch = std::cos(pitch_);
    float sin_pitch = std::sin(pitch_);

    glm::vec3 forward = glm::vec3(sin_yaw * cos_pitch, -sin_pitch, -cos_yaw * cos_pitch);
    glm::vec3 right = glm::vec3(cos_yaw, 0.0f, sin_yaw);
    glm::vec3 up = glm::normalize(glm::cross(right, forward));
    return glm::lookAt(position_, position_ + forward, up);
}

} // namespace engine
