#pragma once

#include "engine/graphics/geometry_buffer.h"
#include <functional>

namespace engine {
namespace graphics {

// Generate a merged triangle mesh for a block-based terrain.
// width/depth define the number of columns; height_fn(x,z) returns the integer column height (>=0).
TriangleMesh create_terrain_mesh(int width, int depth, const std::function<int(int,int)>& height_fn);

// Generate a wireframe mesh (GL_LINES) for the same terrain (edges of visible faces).
TriangleMesh create_terrain_wireframe(int width, int depth, const std::function<int(int,int)>& height_fn);

} // namespace graphics
} // namespace engine
