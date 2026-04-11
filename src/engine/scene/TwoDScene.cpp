#include "engine/scene/two_d_scene.h"
#include "engine/text_renderer.h"
#include "engine/camera/camera2d.h"
#include "engine/graphics/shader_program.h"
#include "engine/ecs/registry.h"
#include "engine/input/terraria_2d_control_scheme.h"
#include <GL/gl.h>
#include <glm/gtc/type_ptr.hpp>
#include <iostream>

namespace engine {
namespace scene {

TwoDScene::TwoDScene() {
    registry_ = std::make_shared<engine::ecs::Registry>();
    shader_ = std::make_shared<engine::graphics::ShaderProgram>();
    camera2d_ = std::make_unique<engine::Camera2D>();
    text_renderer_ = std::make_unique<engine::TextRenderer>();
    world_ = std::make_unique<TerrariaWorld>(200, 200);
    // Player will be spawned after world generation (so we can place on surface)
    player_.reset();
}

void TwoDScene::init() {
    std::string err;
    if (!shader_->compile_from_files("shaders/default.vert", "shaders/default.frag", err)) {
        std::cerr << "TwoDScene: Failed to compile shader: " << err << std::endl;
        // Not fatal; continue without shader
    }

    // Generate the Terraria-like world
    if (world_) {
        world_->generate();
    }

    // Spawn player near the surface in the middle of the world
    if (world_) {
        const int block_size = 16;
        int spawn_block_x = world_->width() / 2;
        int surface_y_block = world_->surface_y_at(spawn_block_x);
        float spawn_x = static_cast<float>(spawn_block_x * block_size + block_size/2);
        float spawn_y = static_cast<float>(surface_y_block * block_size) - 10.0f;
        if (spawn_y < 50.0f) spawn_y = static_cast<float>(world_->height() * block_size / 2);
        player_ = std::make_unique<Player>(spawn_x, spawn_y);
    }

    // Set up the 2D control scheme
    auto control_scheme = std::make_shared<Terraria2DControlScheme>();
    set_control_scheme(control_scheme);

    // Example cutscene text
    text_renderer_->set_text("Welcome to Terraria 2D! Use WASD or Arrows to move, Space to jump.");
    text_renderer_->start();
}

void TwoDScene::update(float dt) {
    if (camera2d_) camera2d_->update(dt);
    if (text_renderer_) text_renderer_->update(dt);

    // Update player with control input
    if (player_ && control_scheme_) {
        auto terraria_scheme = std::dynamic_pointer_cast<Terraria2DControlScheme>(control_scheme_);
        if (terraria_scheme) {
            auto movement = terraria_scheme->get_movement_input();
            player_->set_velocity(movement.x * 150.0f, player_->position().y);  // Preserve Y velocity
            
            // Handle jump
            if (terraria_scheme->is_jump_pressed()) {
                // Simple jump: check if on ground (y near 3000)
                if (player_->position().y > 3000.0f - 20.0f) {
                    auto pos = player_->position();
                    player_->set_velocity(movement.x * 150.0f, -350.0f);
                }
            }
        }
        player_->update(dt);
    }

    // Update camera to follow player
    if (camera2d_ && player_) {
        auto player_pos = player_->position();
        // Center camera on player
        float cam_x = player_pos.x - 640.0f;  // Half of 1280 window width
        float cam_y = player_pos.y - 360.0f;  // Half of 720 window height
        camera2d_->set_position(cam_x, cam_y);
    }
}

void TwoDScene::render(OpenGLRenderer& renderer, const Camera& /*camera*/) {
    // Disable depth testing for 2D overlay rendering so everything is visible
    GLboolean depth_was_enabled = glIsEnabled(GL_DEPTH_TEST);
    if (depth_was_enabled == GL_TRUE) glDisable(GL_DEPTH_TEST);

    // Push a simple orthographic projection for 2D rendering
    glMatrixMode(GL_PROJECTION);
    glPushMatrix();
    glLoadIdentity();
    float w = static_cast<float>(renderer.width());
    float h = static_cast<float>(renderer.height());
    glOrtho(0.0f, w, h, 0.0f, -1.0f, 1.0f);

    glMatrixMode(GL_MODELVIEW);
    glPushMatrix();
    glLoadIdentity();

    // Apply camera2d transform (simple translate + scale)
    if (camera2d_) {
        auto pos = camera2d_->position();
        float zoom = camera2d_->zoom();
        glTranslatef(-pos.x, -pos.y, 0.0f);
        glScalef(zoom, zoom, 1.0f);
    }

    // Clear background for the 2D scene
    glBegin(GL_QUADS);
        glColor3f(0.4f, 0.6f, 1.0f);  // Sky blue
        glVertex2f(0.0f, 0.0f);
        glVertex2f(3200.0f, 0.0f);
        glVertex2f(3200.0f, 3200.0f);
        glVertex2f(0.0f, 3200.0f);
    glEnd();

    // Render the world
    if (world_) {
        world_->render();
    }

    // Render the player
    if (player_) {
        player_->render();
    }

    // Restore matrices
    glPopMatrix();
    glMatrixMode(GL_PROJECTION);
    glPopMatrix();
    glMatrixMode(GL_MODELVIEW);

    // Restore depth test state
    if (depth_was_enabled == GL_TRUE) glEnable(GL_DEPTH_TEST);

    // Render cutscene text via ImGui (ImGui frame is active during scene rendering)
    if (text_renderer_) text_renderer_->render();
}

} // namespace scene
} // namespace engine
