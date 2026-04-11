#include "engine/camera/camera2d.h"

namespace engine {

Camera2D::Camera2D() : pos_(0.0f, 0.0f), zoom_(1.0f) {}

void Camera2D::update(float /*dt*/) {
    // Placeholder for future smoothing/animation
}

} // namespace engine
