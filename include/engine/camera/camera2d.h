#pragma once

#include <glm/glm.hpp>

namespace engine {

class Camera2D {
public:
    Camera2D();
    void set_position(float x, float y) { pos_ = glm::vec2(x, y); }
    void move(float dx, float dy) { pos_ += glm::vec2(dx, dy); }
    void set_zoom(float z) { zoom_ = z; }
    float zoom() const { return zoom_; }
    glm::vec2 position() const { return pos_; }
    void update(float dt);

private:
    glm::vec2 pos_;
    float zoom_;
};

} // namespace engine
