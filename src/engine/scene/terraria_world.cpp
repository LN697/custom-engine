#include "engine/scene/terraria_world.h"
#include <GL/gl.h>
#include <random>
#include <algorithm>

namespace engine {
namespace scene {

TerrariaWorld::TerrariaWorld(int width, int height)
    : width_(width), height_(height), blocks_(width * height, BlockType::AIR) {}

void TerrariaWorld::generate() {
    // Fill with dirt/stone base
    fill_dirt_and_stone();
    // Add grass on surface
    add_surface_grass();
}

void TerrariaWorld::fill_dirt_and_stone() {
    std::mt19937 rng(12345);  // Fixed seed for reproducibility
    std::uniform_real_distribution<float> noise(0.0f, 1.0f);

    for (int y = 0; y < height_; ++y) {
        // Simple height map: surface at ~60% from top
        int surface_y = static_cast<int>(height_ * 0.6f) + static_cast<int>(noise(rng) * 20.0f) - 10;
        
        for (int x = 0; x < width_; ++x) {
            if (y < surface_y) {
                blocks_[y * width_ + x] = BlockType::AIR;
            } else if (y < surface_y + 5) {
                // Dirt layer
                blocks_[y * width_ + x] = BlockType::DIRT;
            } else {
                // Stone layer
                blocks_[y * width_ + x] = BlockType::STONE;
            }
        }
    }
}

void TerrariaWorld::add_surface_grass() {
    for (int x = 0; x < width_; ++x) {
        for (int y = 0; y < height_; ++y) {
            // Find topmost dirt and add grass
            if (get_block(x, y) == BlockType::DIRT &&
                y > 0 && get_block(x, y - 1) == BlockType::AIR) {
                set_block(x, y, BlockType::GRASS);
            }
        }
    }
}

BlockType TerrariaWorld::get_block(int x, int y) const {
    if (x < 0 || x >= width_ || y < 0 || y >= height_) {
        return BlockType::AIR;
    }
    return blocks_[y * width_ + x];
}

void TerrariaWorld::set_block(int x, int y, BlockType type) {
    if (x < 0 || x >= width_ || y < 0 || y >= height_) {
        return;
    }
    blocks_[y * width_ + x] = type;
}

void TerrariaWorld::render() {
    const int block_size = 16;  // 16 pixels per block for the 200x200 = 3200x3200 pixel world
    
    glBegin(GL_QUADS);

    for (int y = 0; y < height_; ++y) {
        for (int x = 0; x < width_; ++x) {
            BlockType block = get_block(x, y);
            if (block == BlockType::AIR) continue;

            float r, g, b;
            switch (block) {
                case BlockType::DIRT:
                    r = 0.6f; g = 0.45f; b = 0.2f;
                    break;
                case BlockType::STONE:
                    r = 0.5f; g = 0.5f; b = 0.5f;
                    break;
                case BlockType::GRASS:
                    r = 0.2f; g = 0.7f; b = 0.2f;
                    break;
                default:
                    r = g = b = 0.5f;
            }

            glColor3f(r, g, b);
            float x0 = static_cast<float>(x * block_size);
            float y0 = static_cast<float>(y * block_size);
            float x1 = x0 + block_size;
            float y1 = y0 + block_size;

            glVertex2f(x0, y0);
            glVertex2f(x1, y0);
            glVertex2f(x1, y1);
            glVertex2f(x0, y1);
        }
    }

    glEnd();

    // Draw wireframe grid lines (block boundaries) to improve visibility
    glLineWidth(1.0f);
    glColor3f(0.0f, 0.0f, 0.0f);
    glBegin(GL_LINES);
    // vertical lines
    for (int x = 0; x <= width_; ++x) {
        float xpos = static_cast<float>(x * block_size);
        glVertex2f(xpos, 0.0f);
        glVertex2f(xpos, static_cast<float>(height_ * block_size));
    }
    // horizontal lines
    for (int y = 0; y <= height_; ++y) {
        float ypos = static_cast<float>(y * block_size);
        glVertex2f(0.0f, ypos);
        glVertex2f(static_cast<float>(width_ * block_size), ypos);
    }
    glEnd();
}

int TerrariaWorld::surface_y_at(int x) const {
    if (x < 0 || x >= width_) return height_;
    for (int y = 0; y < height_; ++y) {
        if (get_block(x, y) != BlockType::AIR) return y;
    }
    return height_;
}

} // namespace scene
} // namespace engine
