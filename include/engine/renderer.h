#pragma once

#include <glm/glm.hpp>

namespace engine {

class OpenGLRenderer {
public:
    OpenGLRenderer();
    ~OpenGLRenderer();

    bool initialize(int width, int height);
    void shutdown();
    void resize(int width, int height);
    void render(const class Camera& camera);

    int width() const { return width_; }
    int height() const { return height_; }

private:
    void render_scene();

    int width_;
    int height_;
    float aspect_ratio_;
};

} // namespace engine
