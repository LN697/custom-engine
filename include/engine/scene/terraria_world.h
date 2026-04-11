#pragma once

#include <vector>
#include <glm/glm.hpp>

namespace engine {
namespace scene {

enum class BlockType {
    AIR = 0,
    DIRT = 1,
    STONE = 2,
    GRASS = 3,
};

class TerrariaWorld {
public:
    TerrariaWorld(int width = 200, int height = 200);
    ~TerrariaWorld() = default;

    void generate();
    // Return the topmost non-air block y coordinate for a given x (or height_ if none)
    int surface_y_at(int x) const;
    BlockType get_block(int x, int y) const;
    void set_block(int x, int y, BlockType type);

    int width() const { return width_; }
    int height() const { return height_; }

    void render();

private:
    int width_;
    int height_;
    std::vector<BlockType> blocks_;  // Row-major: blocks_[y * width_ + x]

    void fill_dirt_and_stone();
    void add_surface_grass();
};

} // namespace scene
} // namespace engine
