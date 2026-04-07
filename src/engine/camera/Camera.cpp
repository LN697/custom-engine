#include "engine/camera/Camera.h"
#include <SDL2/SDL.h>
#include <algorithm>
#include <cmath>

namespace engine {
namespace camera {

Camera::Camera()
    : x_(0.0f), y_(1.6f), z_(4.0f), yaw_(0.0f), pitch_(0.0f),
      velX_(0.0f), velY_(0.0f), velZ_(0.0f), movementSmoothing_(12.0f) {}

void Camera::update(float deltaTime, const engine::InputManager& input) {
    // Movement inputs
    float forward = 0.0f;
    float strafe = 0.0f;
    float rise = 0.0f;

    if (input.moveForward())  forward += 1.0f;
    if (input.moveBackward()) forward -= 1.0f;
    if (input.moveRight())    strafe += 1.0f;
    if (input.moveLeft())     strafe -= 1.0f;
    if (input.moveUp())       rise += 1.0f;
    if (input.moveDown())     rise -= 1.0f;

    float length = std::sqrt(forward * forward + strafe * strafe + rise * rise);
    if (length > 0.0f) {
        forward /= length;
        strafe /= length;
        rise /= length;
    }

    // Mouse look (use SDL relative state for robust behavior)
    int mouseDX = 0, mouseDY = 0;
    SDL_GetRelativeMouseState(&mouseDX, &mouseDY);

    yaw_ += mouseDX * kMouseSensitivity;
    pitch_ += mouseDY * kMouseSensitivity;

    float yawInput = static_cast<float>(input.lookRight() - input.lookLeft());
    float pitchInput = static_cast<float>(input.lookDown() - input.lookUp());

    yaw_ += yawInput * kKeyboardLookSpeed * deltaTime;
    pitch_ += pitchInput * kKeyboardLookSpeed * deltaTime;

    // Clamp pitch
    pitch_ = std::fmax(std::fmin(pitch_, 1.5f), -1.5f);

    // Compute direction vectors
    float cosYaw = std::cos(yaw_);
    float sinYaw = std::sin(yaw_);
    float cosPitch = std::cos(pitch_);
    float sinPitch = std::sin(pitch_);

    float forwardX = sinYaw * cosPitch;
    float forwardY = -sinPitch;
    float forwardZ = -cosYaw * cosPitch;

    float rightX = cosYaw;
    float rightY = 0.0f;
    float rightZ = sinYaw;

    float desiredVelX = (forwardX * forward + rightX * strafe) * kMovementSpeed;
    float desiredVelY = (forwardY * forward + rightY * strafe + rise) * kMovementSpeed;
    float desiredVelZ = (forwardZ * forward + rightZ * strafe) * kMovementSpeed;

    // Exponential smoothing of velocity
    float k = movementSmoothing_;
    float t = 1.0f - std::exp(-k * deltaTime);
    velX_ += (desiredVelX - velX_) * t;
    velY_ += (desiredVelY - velY_) * t;
    velZ_ += (desiredVelZ - velZ_) * t;

    // Integrate
    x_ += velX_ * deltaTime;
    y_ += velY_ * deltaTime;
    z_ += velZ_ * deltaTime;
}

} // namespace camera
} // namespace engine
