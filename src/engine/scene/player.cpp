#include "engine/scene/player.h"
#include <GL/gl.h>
#include <glm/glm.hpp>
#include <cmath>

namespace engine {
namespace scene {

Player::Player(float x, float y)
    : position_(x, y), velocity_(0.0f, 0.0f) {}

void Player::update(float dt) {
    // Apply gravity
    velocity_.y += gravity_ * dt;

    // Update position
    position_ += velocity_ * dt;

    // Simple ground collision (y = 3000 = terrain height * block size)
    const float ground_level = 3000.0f;
    if (position_.y + radius_ > ground_level) {
        position_.y = ground_level - radius_;
        velocity_.y = 0.0f;
    }

    // Clamp horizontal movement to world bounds
    const float world_width = 3200.0f;
    if (position_.x - radius_ < 0.0f) {
        position_.x = radius_;
        velocity_.x = 0.0f;
    }
    if (position_.x + radius_ > world_width) {
        position_.x = world_width - radius_;
        velocity_.x = 0.0f;
    }
}

void Player::render() {
    glColor3f(1.0f, 0.8f, 0.2f);  // Yellow circle
    
    const int segments = 16;
    glBegin(GL_TRIANGLE_FAN);
    glVertex2f(position_.x, position_.y);
    for (int i = 0; i <= segments; ++i) {
        float angle = 2.0f * 3.14159265359f * static_cast<float>(i) / static_cast<float>(segments);
        float dx = radius_ * std::cos(angle);
        float dy = radius_ * std::sin(angle);
        glVertex2f(position_.x + dx, position_.y + dy);
    }
    glEnd();

    // Draw outline for visibility
    glLineWidth(2.0f);
    glColor3f(0.0f, 0.0f, 0.0f);
    glBegin(GL_LINE_LOOP);
    for (int i = 0; i < segments; ++i) {
        float angle = 2.0f * 3.14159265359f * static_cast<float>(i) / static_cast<float>(segments);
        float dx = radius_ * std::cos(angle);
        float dy = radius_ * std::sin(angle);
        glVertex2f(position_.x + dx, position_.y + dy);
    }
    glEnd();
}

} // namespace scene
} // namespace engine
