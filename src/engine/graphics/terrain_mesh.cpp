#include "engine/graphics/terrain_mesh.h"
#include <glm/glm.hpp>
#include <algorithm>

namespace engine {
namespace graphics {

static inline void push_vertex(TriangleMesh& mesh, float px, float py, float pz,
                               float nx, float ny, float nz,
                               float u, float v) {
    mesh.vertices.push_back(px);
    mesh.vertices.push_back(py);
    mesh.vertices.push_back(pz);
    mesh.vertices.push_back(nx);
    mesh.vertices.push_back(ny);
    mesh.vertices.push_back(nz);
    mesh.vertices.push_back(u);
    mesh.vertices.push_back(v);
}

TriangleMesh create_terrain_mesh(int width, int depth, const std::function<int(int,int)>& height_fn) {
    TriangleMesh mesh;
    const float half_w = static_cast<float>(width) * 0.5f;
    const float half_d = static_cast<float>(depth) * 0.5f;

    // Iterate columns
    for (int x = 0; x < width; ++x) {
        for (int z = 0; z < depth; ++z) {
            int h = std::max(0, height_fn(x, z));
            if (h <= 0) continue;

            float fx = static_cast<float>(x) - half_w;
            float fz = static_cast<float>(z) - half_d;

            // Top face at y = h
            float ytop = static_cast<float>(h);
            uint32_t base = static_cast<uint32_t>(mesh.vertices.size() / 8);
            push_vertex(mesh, fx,    ytop, fz,     0.0f, 1.0f, 0.0f,  0.0f, 0.0f);
            push_vertex(mesh, fx+1,  ytop, fz,     0.0f, 1.0f, 0.0f,  1.0f, 0.0f);
            push_vertex(mesh, fx+1,  ytop, fz+1,   0.0f, 1.0f, 0.0f,  1.0f, 1.0f);
            push_vertex(mesh, fx,    ytop, fz+1,   0.0f, 1.0f, 0.0f,  0.0f, 1.0f);
            mesh.indices.push_back(base + 0);
            mesh.indices.push_back(base + 1);
            mesh.indices.push_back(base + 2);
            mesh.indices.push_back(base + 0);
            mesh.indices.push_back(base + 2);
            mesh.indices.push_back(base + 3);

            // Side faces (compare neighbor heights and generate vertical quads for exposed spans)
            auto neighbor_h = [&](int nx, int nz) -> int {
                if (nx < 0 || nx >= width || nz < 0 || nz >= depth) return 0;
                return std::max(0, height_fn(nx, nz));
            };

            // +X (east)
            {
                int nh = neighbor_h(x+1, z);
                if (nh < h) {
                    float y0 = static_cast<float>(nh);
                    float y1 = static_cast<float>(h);
                    uint32_t b = static_cast<uint32_t>(mesh.vertices.size() / 8);
                    // quad facing +X
                    push_vertex(mesh, fx+1, y0, fz,     1.0f, 0.0f, 0.0f,  0.0f, 0.0f);
                    push_vertex(mesh, fx+1, y0, fz+1,   1.0f, 0.0f, 0.0f,  1.0f, 0.0f);
                    push_vertex(mesh, fx+1, y1, fz+1,   1.0f, 0.0f, 0.0f,  1.0f, 1.0f);
                    push_vertex(mesh, fx+1, y1, fz,     1.0f, 0.0f, 0.0f,  0.0f, 1.0f);
                    mesh.indices.insert(mesh.indices.end(), { b+0, b+1, b+2, b+0, b+2, b+3 });
                }
            }

            // -X (west)
            {
                int nh = neighbor_h(x-1, z);
                if (nh < h) {
                    float y0 = static_cast<float>(nh);
                    float y1 = static_cast<float>(h);
                    uint32_t b = static_cast<uint32_t>(mesh.vertices.size() / 8);
                    // quad facing -X
                    push_vertex(mesh, fx, y0, fz+1,    -1.0f, 0.0f, 0.0f,  0.0f, 0.0f);
                    push_vertex(mesh, fx, y0, fz,      -1.0f, 0.0f, 0.0f,  1.0f, 0.0f);
                    push_vertex(mesh, fx, y1, fz,      -1.0f, 0.0f, 0.0f,  1.0f, 1.0f);
                    push_vertex(mesh, fx, y1, fz+1,    -1.0f, 0.0f, 0.0f,  0.0f, 1.0f);
                    mesh.indices.insert(mesh.indices.end(), { b+0, b+1, b+2, b+0, b+2, b+3 });
                }
            }

            // +Z (south)
            {
                int nh = neighbor_h(x, z+1);
                if (nh < h) {
                    float y0 = static_cast<float>(nh);
                    float y1 = static_cast<float>(h);
                    uint32_t b = static_cast<uint32_t>(mesh.vertices.size() / 8);
                    // quad facing +Z
                    push_vertex(mesh, fx+1, y0, fz+1,   0.0f, 0.0f, 1.0f,  0.0f, 0.0f);
                    push_vertex(mesh, fx,    y0, fz+1,   0.0f, 0.0f, 1.0f,  1.0f, 0.0f);
                    push_vertex(mesh, fx,    y1, fz+1,   0.0f, 0.0f, 1.0f,  1.0f, 1.0f);
                    push_vertex(mesh, fx+1, y1, fz+1,   0.0f, 0.0f, 1.0f,  0.0f, 1.0f);
                    mesh.indices.insert(mesh.indices.end(), { b+0, b+1, b+2, b+0, b+2, b+3 });
                }
            }

            // -Z (north)
            {
                int nh = neighbor_h(x, z-1);
                if (nh < h) {
                    float y0 = static_cast<float>(nh);
                    float y1 = static_cast<float>(h);
                    uint32_t b = static_cast<uint32_t>(mesh.vertices.size() / 8);
                    // quad facing -Z
                    push_vertex(mesh, fx, y0, fz,     0.0f, 0.0f, -1.0f,  0.0f, 0.0f);
                    push_vertex(mesh, fx+1, y0, fz,   0.0f, 0.0f, -1.0f,  1.0f, 0.0f);
                    push_vertex(mesh, fx+1, y1, fz,   0.0f, 0.0f, -1.0f,  1.0f, 1.0f);
                    push_vertex(mesh, fx, y1, fz,     0.0f, 0.0f, -1.0f,  0.0f, 1.0f);
                    mesh.indices.insert(mesh.indices.end(), { b+0, b+1, b+2, b+0, b+2, b+3 });
                }
            }
        }
    }

    return mesh;
}

TriangleMesh create_terrain_wireframe(int width, int depth, const std::function<int(int,int)>& height_fn) {
    TriangleMesh mesh;
    const float half_w = static_cast<float>(width) * 0.5f;
    const float half_d = static_cast<float>(depth) * 0.5f;

    // For each visible face (same conditions as create_terrain_mesh) emit corner vertices and line pairs
    for (int x = 0; x < width; ++x) {
        for (int z = 0; z < depth; ++z) {
            int h = std::max(0, height_fn(x, z));
            if (h <= 0) continue;

            float fx = static_cast<float>(x) - half_w;
            float fz = static_cast<float>(z) - half_d;

            auto neighbor_h = [&](int nx, int nz) -> int {
                if (nx < 0 || nx >= width || nz < 0 || nz >= depth) return 0;
                return std::max(0, height_fn(nx, nz));
            };

            // Top face
            {
                uint32_t base = static_cast<uint32_t>(mesh.vertices.size() / 8);
                float ytop = static_cast<float>(h);
                push_vertex(mesh, fx,    ytop, fz,     0.0f,0.0f,0.0f, 0.0f,0.0f);
                push_vertex(mesh, fx+1,  ytop, fz,     0.0f,0.0f,0.0f, 1.0f,0.0f);
                push_vertex(mesh, fx+1,  ytop, fz+1,   0.0f,0.0f,0.0f, 1.0f,1.0f);
                push_vertex(mesh, fx,    ytop, fz+1,   0.0f,0.0f,0.0f, 0.0f,1.0f);
                // edges (pairs)
                mesh.indices.insert(mesh.indices.end(), { base+0, base+1, base+1, base+2, base+2, base+3, base+3, base+0 });
            }

            // +X
            {
                int nh = neighbor_h(x+1, z);
                if (nh < h) {
                    uint32_t base = static_cast<uint32_t>(mesh.vertices.size() / 8);
                    float y0 = static_cast<float>(nh);
                    float y1 = static_cast<float>(h);
                    push_vertex(mesh, fx+1, y0, fz,   0,0,0, 0,0);
                    push_vertex(mesh, fx+1, y0, fz+1, 0,0,0, 1,0);
                    push_vertex(mesh, fx+1, y1, fz+1, 0,0,0, 1,1);
                    push_vertex(mesh, fx+1, y1, fz,   0,0,0, 0,1);
                    mesh.indices.insert(mesh.indices.end(), { base+0,base+1, base+1,base+2, base+2,base+3, base+3,base+0 });
                }
            }

            // -X
            {
                int nh = neighbor_h(x-1, z);
                if (nh < h) {
                    uint32_t base = static_cast<uint32_t>(mesh.vertices.size() / 8);
                    float y0 = static_cast<float>(nh);
                    float y1 = static_cast<float>(h);
                    push_vertex(mesh, fx, y0, fz+1, 0,0,0, 0,0);
                    push_vertex(mesh, fx, y0, fz,   0,0,0, 1,0);
                    push_vertex(mesh, fx, y1, fz,   0,0,0, 1,1);
                    push_vertex(mesh, fx, y1, fz+1, 0,0,0, 0,1);
                    mesh.indices.insert(mesh.indices.end(), { base+0,base+1, base+1,base+2, base+2,base+3, base+3,base+0 });
                }
            }

            // +Z
            {
                int nh = neighbor_h(x, z+1);
                if (nh < h) {
                    uint32_t base = static_cast<uint32_t>(mesh.vertices.size() / 8);
                    float y0 = static_cast<float>(nh);
                    float y1 = static_cast<float>(h);
                    push_vertex(mesh, fx+1, y0, fz+1, 0,0,0, 0,0);
                    push_vertex(mesh, fx,    y0, fz+1, 0,0,0, 1,0);
                    push_vertex(mesh, fx,    y1, fz+1, 0,0,0, 1,1);
                    push_vertex(mesh, fx+1, y1, fz+1, 0,0,0, 0,1);
                    mesh.indices.insert(mesh.indices.end(), { base+0,base+1, base+1,base+2, base+2,base+3, base+3,base+0 });
                }
            }

            // -Z
            {
                int nh = neighbor_h(x, z-1);
                if (nh < h) {
                    uint32_t base = static_cast<uint32_t>(mesh.vertices.size() / 8);
                    float y0 = static_cast<float>(nh);
                    float y1 = static_cast<float>(h);
                    push_vertex(mesh, fx, y0, fz, 0,0,0, 0,0);
                    push_vertex(mesh, fx+1, y0, fz, 0,0,0, 1,0);
                    push_vertex(mesh, fx+1, y1, fz, 0,0,0, 1,1);
                    push_vertex(mesh, fx, y1, fz, 0,0,0, 0,1);
                    mesh.indices.insert(mesh.indices.end(), { base+0,base+1, base+1,base+2, base+2,base+3, base+3,base+0 });
                }
            }
        }
    }

    return mesh;
}

} // namespace graphics
} // namespace engine
