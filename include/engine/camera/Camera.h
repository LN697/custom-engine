#pragma once

#include "engine/input/InputManager.h"
#include <cmath>

namespace engine {
namespace camera {

class Camera {
public:
    Camera();

    void update(float deltaTime, const engine::InputManager& input);

    float x() const { return x_; }
    float y() const { return y_; }
    float z() const { return z_; }
    float yaw() const { return yaw_; }
    float pitch() const { return pitch_; }

    void setPosition(float x, float y, float z) { x_ = x; y_ = y; z_ = z; }
    void setRotation(float yaw, float pitch) { yaw_ = yaw; pitch_ = pitch; }

    void setSmoothing(float s) { movementSmoothing_ = s; }
    float* smoothingPtr() { return &movementSmoothing_; }

private:
    float x_, y_, z_;
    float yaw_, pitch_;
    float velX_, velY_, velZ_;
    float movementSmoothing_;

    static constexpr float kMovementSpeed = 4.0f;
    static constexpr float kMouseSensitivity = 0.0025f;
    static constexpr float kKeyboardLookSpeed = 1.5f; // radians per second for keyboard look
};

} // namespace camera
} // namespace engine
