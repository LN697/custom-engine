#pragma once

namespace engine {

class OpenGLRenderer {
public:
    OpenGLRenderer();
    ~OpenGLRenderer();

    bool initialize(int width, int height);
    void resize(int width, int height);
    void render(float cameraX, float cameraY, float cameraZ, float yaw, float pitch);
    void shutdown();

private:
    void renderScene();

    int width_;
    int height_;
};

} // namespace engine
