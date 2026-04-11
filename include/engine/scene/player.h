#pragma once

#include <glm/glm.hpp>

namespace engine {
namespace scene {

class Player {
public:
    Player(float x = 100.0f, float y = 100.0f);

    void update(float dt);
    void render();

    void set_velocity(float vx, float vy) { velocity_ = glm::vec2(vx, vy); }
    void add_velocity(float vx, float vy) { velocity_ += glm::vec2(vx, vy); }

    glm::vec2 position() const { return position_; }
    void set_position(float x, float y) { position_ = glm::vec2(x, y); }

private:
    glm::vec2 position_;
    glm::vec2 velocity_;
    static constexpr float radius_ = 8.0f;
    static constexpr float gravity_ = 0.0f; // gravity disabled for now
    static constexpr float jump_power_ = 10.0f;
};

} // namespace scene
} // namespace engine
