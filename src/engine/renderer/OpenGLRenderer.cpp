#include "engine/renderer.h"
#include "engine/camera.h"
#include <glm/gtc/type_ptr.hpp>
#include <GL/gl.h>
#include <cmath>
#include <iostream>

namespace engine {

OpenGLRenderer::OpenGLRenderer() : width_(0), height_(0) {}

OpenGLRenderer::~OpenGLRenderer() {
    shutdown();
}

bool OpenGLRenderer::initialize(int width, int height) {
    width_ = width;
    height_ = height;

    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LEQUAL);
    resize(width_, height_);
    glClearColor(0.05f, 0.1f, 0.18f, 1.0f);
    return true;
}

void OpenGLRenderer::resize(int width, int height) {
    width_ = width;
    height_ = height;

    glViewport(0, 0, width_, height_);
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();

    const float aspect = (height_ > 0) ? static_cast<float>(width_) / static_cast<float>(height_) : 1.0f;
    const float fov_radians = 80.0f * 3.14159265358979f / 180.0f;
    const float near_plane = 0.1f;
    const float top = tanf(fov_radians * 0.5f) * near_plane;
    const float right = top * aspect;
    glFrustum(-right, right, -top, top, near_plane, 100.0f);

    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
}

void OpenGLRenderer::render(const Camera& camera) {
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();

    // Use the camera's view matrix (computed with GLM) and load it into the GL matrix stack
    glm::mat4 view = camera.get_view_matrix();
    glLoadMatrixf(glm::value_ptr(view));

    render_scene();
}

void OpenGLRenderer::render_scene() {
    glBegin(GL_LINES);
        glColor3f(0.4f, 0.4f, 0.4f);
        for (int i = -10; i <= 10; ++i) {
            glVertex3f(static_cast<float>(i), 0.0f, -10.0f);
            glVertex3f(static_cast<float>(i), 0.0f, 10.0f);
            glVertex3f(-10.0f, 0.0f, static_cast<float>(i));
            glVertex3f(10.0f, 0.0f, static_cast<float>(i));
        }
    glEnd();

    glBegin(GL_TRIANGLES);
        glColor3f(0.9f, 0.2f, 0.2f);
        glVertex3f(0.0f, 1.0f, -5.0f);
        glColor3f(0.2f, 0.9f, 0.3f);
        glVertex3f(-1.0f, 0.0f, -4.0f);
        glColor3f(0.2f, 0.4f, 0.9f);
        glVertex3f(1.0f, 0.0f, -4.0f);
    glEnd();

    glBegin(GL_LINES);
        glColor3f(1.0f, 0.0f, 0.0f);
        glVertex3f(0.0f, 0.0f, 0.0f);
        glVertex3f(1.0f, 0.0f, 0.0f);
        glColor3f(0.0f, 1.0f, 0.0f);
        glVertex3f(0.0f, 0.0f, 0.0f);
        glVertex3f(0.0f, 1.0f, 0.0f);
        glColor3f(0.0f, 0.0f, 1.0f);
        glVertex3f(0.0f, 0.0f, 0.0f);
        glVertex3f(0.0f, 0.0f, 1.0f);
    glEnd();
}

void OpenGLRenderer::shutdown() {
    // No explicit cleanup required for immediate-mode rendering.
}

} // namespace engine
