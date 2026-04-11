#include "engine/level_loader.h"
#include "engine/ecs/registry.h"
#include "engine/ecs/components.h"
#include "engine/graphics/geometry_buffer.h"
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <nlohmann/json.hpp>
#include <array>
#include <fstream>
#include <sstream>
#include <iostream>
#include <vector>
#include <string>
#include <cmath>
#include <algorithm>
#include <cctype>
#include <unordered_map>

namespace engine {
namespace level {

using engine::graphics::TriangleMesh;

static TriangleMesh generate_cube_mesh() {
    TriangleMesh mesh;
    mesh.vertices.reserve(24 * 8);
    mesh.indices.reserve(36);

    // Helper to push a face (4 verts) with a normal
    auto push_face = [&](std::array<glm::vec3,4> verts, const glm::vec3& normal) {
        uint32_t base = static_cast<uint32_t>(mesh.vertices.size() / 8);
        // UVs: (0,0),(1,0),(1,1),(0,1)
        const std::array<glm::vec2,4> uvs = {{ glm::vec2(0.0f,0.0f), glm::vec2(1.0f,0.0f), glm::vec2(1.0f,1.0f), glm::vec2(0.0f,1.0f) }};
        for (int i = 0; i < 4; ++i) {
            mesh.vertices.push_back(verts[i].x);
            mesh.vertices.push_back(verts[i].y);
            mesh.vertices.push_back(verts[i].z);
            mesh.vertices.push_back(normal.x);
            mesh.vertices.push_back(normal.y);
            mesh.vertices.push_back(normal.z);
            mesh.vertices.push_back(uvs[i].x);
            mesh.vertices.push_back(uvs[i].y);
        }
        mesh.indices.push_back(base + 0);
        mesh.indices.push_back(base + 1);
        mesh.indices.push_back(base + 2);
        mesh.indices.push_back(base + 0);
        mesh.indices.push_back(base + 2);
        mesh.indices.push_back(base + 3);
    };

    // +X face
    push_face(std::array<glm::vec3,4>{ glm::vec3(0.5f, -0.5f, -0.5f), glm::vec3(0.5f, 0.5f, -0.5f), glm::vec3(0.5f, 0.5f, 0.5f), glm::vec3(0.5f, -0.5f, 0.5f) }, glm::vec3(1,0,0));
    // -X face
    push_face(std::array<glm::vec3,4>{ glm::vec3(-0.5f, -0.5f, 0.5f), glm::vec3(-0.5f, 0.5f, 0.5f), glm::vec3(-0.5f, 0.5f, -0.5f), glm::vec3(-0.5f, -0.5f, -0.5f) }, glm::vec3(-1,0,0));
    // +Y face
    push_face(std::array<glm::vec3,4>{ glm::vec3(-0.5f, 0.5f, -0.5f), glm::vec3(-0.5f, 0.5f, 0.5f), glm::vec3(0.5f, 0.5f, 0.5f), glm::vec3(0.5f, 0.5f, -0.5f) }, glm::vec3(0,1,0));
    // -Y face
    push_face(std::array<glm::vec3,4>{ glm::vec3(-0.5f, -0.5f, 0.5f), glm::vec3(-0.5f, -0.5f, -0.5f), glm::vec3(0.5f, -0.5f, -0.5f), glm::vec3(0.5f, -0.5f, 0.5f) }, glm::vec3(0,-1,0));
    // +Z face
    push_face(std::array<glm::vec3,4>{ glm::vec3(-0.5f, -0.5f, 0.5f), glm::vec3(0.5f, -0.5f, 0.5f), glm::vec3(0.5f, 0.5f, 0.5f), glm::vec3(-0.5f, 0.5f, 0.5f) }, glm::vec3(0,0,1));
    // -Z face
    push_face(std::array<glm::vec3,4>{ glm::vec3(0.5f, -0.5f, -0.5f), glm::vec3(-0.5f, -0.5f, -0.5f), glm::vec3(-0.5f, 0.5f, -0.5f), glm::vec3(0.5f, 0.5f, -0.5f) }, glm::vec3(0,0,-1));

    return mesh;
}

static TriangleMesh generate_plane_mesh() {
    TriangleMesh mesh;
    // Quad in XZ plane at y=0
    mesh.vertices = {
        -0.5f, 0.0f, -0.5f,   0.0f, 1.0f, 0.0f,   0.0f, 0.0f,
         0.5f, 0.0f, -0.5f,   0.0f, 1.0f, 0.0f,   1.0f, 0.0f,
         0.5f, 0.0f,  0.5f,   0.0f, 1.0f, 0.0f,   1.0f, 1.0f,
        -0.5f, 0.0f,  0.5f,   0.0f, 1.0f, 0.0f,   0.0f, 1.0f
    };
    mesh.indices = { 0,1,2, 0,2,3 };
    return mesh;
}

static TriangleMesh generate_sphere_mesh(int sectors = 24, int stacks = 16) {
    TriangleMesh mesh;
    const float radius = 0.5f;
    for (int i = 0; i <= stacks; ++i) {
        float stackAngle = M_PI / 2 - i * (M_PI / static_cast<float>(stacks)); // from pi/2 to -pi/2
        float xy = radius * std::cos(stackAngle);
        float z = radius * std::sin(stackAngle);
        for (int j = 0; j <= sectors; ++j) {
            float sectorAngle = j * (2 * M_PI / static_cast<float>(sectors));
            float x = xy * std::cos(sectorAngle);
            float y = xy * std::sin(sectorAngle);
            glm::vec3 pos(x, z, y);
            glm::vec3 norm = glm::normalize(pos);
            float u = j / static_cast<float>(sectors);
            float v = i / static_cast<float>(stacks);
            mesh.vertices.push_back(pos.x);
            mesh.vertices.push_back(pos.y);
            mesh.vertices.push_back(pos.z);
            mesh.vertices.push_back(norm.x);
            mesh.vertices.push_back(norm.y);
            mesh.vertices.push_back(norm.z);
            mesh.vertices.push_back(u);
            mesh.vertices.push_back(v);
        }
    }

    for (int i = 0; i < stacks; ++i) {
        int k1 = i * (sectors + 1);
        int k2 = k1 + sectors + 1;
        for (int j = 0; j < sectors; ++j) {
            if (i != 0) {
                mesh.indices.push_back(k1 + j);
                mesh.indices.push_back(k2 + j);
                mesh.indices.push_back(k1 + j + 1);
            }
            if (i != (stacks - 1)) {
                mesh.indices.push_back(k1 + j + 1);
                mesh.indices.push_back(k2 + j);
                mesh.indices.push_back(k2 + j + 1);
            }
        }
    }

    return mesh;
}

static TriangleMesh generate_cylinder_mesh(int sectors = 24) {
    TriangleMesh mesh;
    const float radius = 0.5f;
    const float halfH = 0.5f;

    // side vertices
    for (int i = 0; i <= sectors; ++i) {
        float theta = i * (2 * M_PI / static_cast<float>(sectors));
        float x = radius * std::cos(theta);
        float z = radius * std::sin(theta);
        // bottom
        mesh.vertices.push_back(x);
        mesh.vertices.push_back(-halfH);
        mesh.vertices.push_back(z);
        mesh.vertices.push_back(x);
        mesh.vertices.push_back(0.0f);
        mesh.vertices.push_back(z);
        mesh.vertices.push_back(i / static_cast<float>(sectors));
        mesh.vertices.push_back(0.0f);
        // top
        mesh.vertices.push_back(x);
        mesh.vertices.push_back(halfH);
        mesh.vertices.push_back(z);
        mesh.vertices.push_back(x);
        mesh.vertices.push_back(0.0f);
        mesh.vertices.push_back(z);
        mesh.vertices.push_back(i / static_cast<float>(sectors));
        mesh.vertices.push_back(1.0f);
    }

    // side indices
    for (int i = 0; i < sectors; ++i) {
        int k1 = i * 2;
        int k2 = k1 + 1;
        int k3 = ((i + 1) % (sectors + 1)) * 2;
        int k4 = k3 + 1;
        mesh.indices.push_back(k1);
        mesh.indices.push_back(k3);
        mesh.indices.push_back(k2);
        mesh.indices.push_back(k2);
        mesh.indices.push_back(k3);
        mesh.indices.push_back(k4);
    }

    // top and bottom caps (triangle fans)
    int baseIndex = static_cast<int>(mesh.vertices.size() / 8);
    // center top
    mesh.vertices.push_back(0.0f); mesh.vertices.push_back(halfH); mesh.vertices.push_back(0.0f);
    mesh.vertices.push_back(0.0f); mesh.vertices.push_back(1.0f); mesh.vertices.push_back(0.0f);
    mesh.vertices.push_back(0.5f); mesh.vertices.push_back(0.5f);
    int topCenter = baseIndex++;
    // center bottom
    mesh.vertices.push_back(0.0f); mesh.vertices.push_back(-halfH); mesh.vertices.push_back(0.0f);
    mesh.vertices.push_back(0.0f); mesh.vertices.push_back(-1.0f); mesh.vertices.push_back(0.0f);
    mesh.vertices.push_back(0.5f); mesh.vertices.push_back(0.5f);
    int bottomCenter = baseIndex++;

    // top cap
    for (int i = 0; i < sectors; ++i) {
        int idx = (i * 2 + 1);
        int next = ((i + 1) % (sectors + 1)) * 2 + 1;
        mesh.indices.push_back(topCenter);
        mesh.indices.push_back(next);
        mesh.indices.push_back(idx);
    }
    // bottom cap
    for (int i = 0; i < sectors; ++i) {
        int idx = (i * 2);
        int next = ((i + 1) % (sectors + 1)) * 2;
        mesh.indices.push_back(bottomCenter);
        mesh.indices.push_back(idx);
        mesh.indices.push_back(next);
    }

    return mesh;
}

static TriangleMesh generate_wedge_mesh() {
    TriangleMesh mesh;
    // define six main points (back top has two verts, front top is at bottom plane)
    glm::vec3 v0(-0.5f, -0.5f, -0.5f); // back-left-bottom
    glm::vec3 v1( 0.5f, -0.5f, -0.5f); // back-right-bottom
    glm::vec3 v2(-0.5f, -0.5f,  0.5f); // front-left-bottom
    glm::vec3 v3( 0.5f, -0.5f,  0.5f); // front-right-bottom
    glm::vec3 v4(-0.5f,  0.5f, -0.5f); // back-left-top
    glm::vec3 v5( 0.5f,  0.5f, -0.5f); // back-right-top

    auto push_quad = [&](glm::vec3 a, glm::vec3 b, glm::vec3 c, glm::vec3 d) {
        uint32_t base = static_cast<uint32_t>(mesh.vertices.size() / 8);
        glm::vec3 normal = glm::normalize(glm::cross(b - a, d - a));
        std::array<glm::vec2,4> uvs = {{ glm::vec2(0,0), glm::vec2(1,0), glm::vec2(1,1), glm::vec2(0,1) }};
        std::array<glm::vec3,4> verts = {{a,b,c,d}};
        for (int i=0;i<4;++i) {
            mesh.vertices.push_back(verts[i].x);
            mesh.vertices.push_back(verts[i].y);
            mesh.vertices.push_back(verts[i].z);
            mesh.vertices.push_back(normal.x);
            mesh.vertices.push_back(normal.y);
            mesh.vertices.push_back(normal.z);
            mesh.vertices.push_back(uvs[i].x);
            mesh.vertices.push_back(uvs[i].y);
        }
        mesh.indices.push_back(base+0);
        mesh.indices.push_back(base+1);
        mesh.indices.push_back(base+2);
        mesh.indices.push_back(base+0);
        mesh.indices.push_back(base+2);
        mesh.indices.push_back(base+3);
    };

    auto push_tri = [&](glm::vec3 a, glm::vec3 b, glm::vec3 c) {
        uint32_t base = static_cast<uint32_t>(mesh.vertices.size() / 8);
        glm::vec3 normal = glm::normalize(glm::cross(b - a, c - a));
        std::array<glm::vec2,3> uvs = {{ glm::vec2(0,0), glm::vec2(1,0), glm::vec2(0.5f,1) }};
        std::array<glm::vec3,3> verts = {{a,b,c}};
        for (int i=0;i<3;++i) {
            mesh.vertices.push_back(verts[i].x);
            mesh.vertices.push_back(verts[i].y);
            mesh.vertices.push_back(verts[i].z);
            mesh.vertices.push_back(normal.x);
            mesh.vertices.push_back(normal.y);
            mesh.vertices.push_back(normal.z);
            mesh.vertices.push_back(uvs[i].x);
            mesh.vertices.push_back(uvs[i].y);
        }
        mesh.indices.push_back(base+0);
        mesh.indices.push_back(base+1);
        mesh.indices.push_back(base+2);
    };

    // bottom
    push_quad(v0, v1, v3, v2);
    // back vertical face
    push_quad(v4, v5, v1, v0);
    // sloped top face (quad between back-top and front-bottom)
    push_quad(v4, v5, v3, v2);
    // left triangle
    push_tri(v4, v0, v2);
    // right triangle
    push_tri(v5, v3, v1);

    return mesh;
}

static TriangleMesh generate_stairs_mesh(int steps = 8) {
    TriangleMesh mesh;
    if (steps <= 0) steps = 1;
    TriangleMesh baseCube = generate_cube_mesh();
    float stepH = 1.0f / static_cast<float>(steps);
    float stepD = 1.0f / static_cast<float>(steps);
    for (int i = 0; i < steps; ++i) {
        float cx = 0.0f;
        float cy = -0.5f + (i + 0.5f) * stepH;
        float cz = -0.5f + (i + 0.5f) * stepD;
        float sx = 1.0f;
        float sy = stepH;
        float sz = stepD;

        uint32_t baseIdx = static_cast<uint32_t>(mesh.vertices.size() / 8);
        // append transformed cube vertices
        size_t vcount = baseCube.vertices.size() / 8;
        for (size_t vi = 0; vi < vcount; ++vi) {
            float vx = baseCube.vertices[vi*8 + 0] * sx + cx;
            float vy = baseCube.vertices[vi*8 + 1] * sy + cy;
            float vz = baseCube.vertices[vi*8 + 2] * sz + cz;
            float nx = baseCube.vertices[vi*8 + 3];
            float ny = baseCube.vertices[vi*8 + 4];
            float nz = baseCube.vertices[vi*8 + 5];
            float u  = baseCube.vertices[vi*8 + 6];
            float v  = baseCube.vertices[vi*8 + 7];
            mesh.vertices.push_back(vx);
            mesh.vertices.push_back(vy);
            mesh.vertices.push_back(vz);
            mesh.vertices.push_back(nx);
            mesh.vertices.push_back(ny);
            mesh.vertices.push_back(nz);
            mesh.vertices.push_back(u);
            mesh.vertices.push_back(v);
        }
        // append indices with offset
        for (auto idx : baseCube.indices) mesh.indices.push_back(baseIdx + idx);
    }
    return mesh;
}

static TriangleMesh generate_torus_mesh(int rings = 24, int sectors = 24) {
    TriangleMesh mesh;
    float R = 0.4f; // major radius
    float r = 0.15f; // minor radius
    for (int i = 0; i < rings; ++i) {
        float phi = 2.0f * M_PI * i / static_cast<float>(rings);
        float cosPhi = std::cos(phi), sinPhi = std::sin(phi);
        for (int j = 0; j < sectors; ++j) {
            float theta = 2.0f * M_PI * j / static_cast<float>(sectors);
            float cosT = std::cos(theta), sinT = std::sin(theta);
            glm::vec3 pos((R + r * cosT) * cosPhi, r * sinT, (R + r * cosT) * sinPhi);
            glm::vec3 tubeCenter(R * cosPhi, 0.0f, R * sinPhi);
            glm::vec3 norm = glm::normalize(pos - tubeCenter);
            float u = static_cast<float>(j) / static_cast<float>(sectors);
            float v = static_cast<float>(i) / static_cast<float>(rings);
            mesh.vertices.push_back(pos.x);
            mesh.vertices.push_back(pos.y);
            mesh.vertices.push_back(pos.z);
            mesh.vertices.push_back(norm.x);
            mesh.vertices.push_back(norm.y);
            mesh.vertices.push_back(norm.z);
            mesh.vertices.push_back(u);
            mesh.vertices.push_back(v);
        }
    }
    for (int i = 0; i < rings; ++i) {
        int ni = (i + 1) % rings;
        for (int j = 0; j < sectors; ++j) {
            int nj = (j + 1) % sectors;
            uint32_t a = i * sectors + j;
            uint32_t b = ni * sectors + j;
            uint32_t c = ni * sectors + nj;
            uint32_t d = i * sectors + nj;
            mesh.indices.push_back(a);
            mesh.indices.push_back(b);
            mesh.indices.push_back(d);
            mesh.indices.push_back(b);
            mesh.indices.push_back(c);
            mesh.indices.push_back(d);
        }
    }
    return mesh;
}

// Very small Wavefront OBJ loader supporting positions, normals, uvs and triangular faces.
static TriangleMesh load_obj_mesh(const std::string& filepath) {
    TriangleMesh mesh;
    std::ifstream in(filepath);
    if (!in.is_open()) return mesh;

    std::vector<glm::vec3> positions;
    std::vector<glm::vec3> normals;
    std::vector<glm::vec2> uvs;
    std::vector<uint32_t> indices;
    struct Packed { int p=-1, t=-1, n=-1; };
    std::unordered_map<std::string, uint32_t> vertMap;
    std::vector<glm::vec3> outPos;
    std::vector<glm::vec3> outNorm;
    std::vector<glm::vec2> outUV;

    std::string line;
    while (std::getline(in, line)) {
        if (line.size() < 2) continue;
        std::istringstream ls(line);
        std::string tag;
        ls >> tag;
        if (tag == "v") {
            float x,y,z; ls >> x >> y >> z; positions.emplace_back(x,y,z);
        } else if (tag == "vn") {
            float x,y,z; ls >> x >> y >> z; normals.emplace_back(x,y,z);
        } else if (tag == "vt") {
            float u,v; ls >> u >> v; uvs.emplace_back(u,v);
        } else if (tag == "f") {
            std::string a,b,c;
            ls >> a >> b >> c;
            std::vector<std::string> face{a,b,c};
            for (auto &tok : face) {
                auto it = vertMap.find(tok);
                if (it != vertMap.end()) {
                    indices.push_back(it->second);
                    continue;
                }
                int p=-1,t=-1,n=-1;
                // parse v/vt/vn  or v//vn  or v
                size_t p1 = tok.find('/');
                if (p1 == std::string::npos) {
                    p = std::stoi(tok) - 1;
                } else {
                    std::string sp = tok.substr(0,p1);
                    p = std::stoi(sp) - 1;
                    size_t p2 = tok.find('/', p1+1);
                    if (p2 == std::string::npos) {
                        std::string st = tok.substr(p1+1);
                        if (!st.empty()) t = std::stoi(st) - 1;
                    } else {
                        std::string st = tok.substr(p1+1, p2 - (p1+1));
                        if (!st.empty()) t = std::stoi(st) - 1;
                        std::string sn = tok.substr(p2+1);
                        if (!sn.empty()) n = std::stoi(sn) - 1;
                    }
                }
                uint32_t newIndex = static_cast<uint32_t>(outPos.size());
                vertMap[tok] = newIndex;
                if (p >= 0 && p < static_cast<int>(positions.size())) outPos.push_back(positions[p]); else outPos.emplace_back(0.0f);
                if (n >= 0 && n < static_cast<int>(normals.size())) outNorm.push_back(normals[n]); else outNorm.emplace_back(0.0f);
                if (t >= 0 && t < static_cast<int>(uvs.size())) outUV.push_back(uvs[t]); else outUV.emplace_back(0.0f, 0.0f);
                indices.push_back(newIndex);
            }
        }
    }
    in.close();

    // if normals missing, compute per-vertex normals
    if (outNorm.empty() || (outNorm.size() != outPos.size())) {
        outNorm.assign(outPos.size(), glm::vec3(0.0f));
        for (size_t i = 0; i + 2 < indices.size(); i += 3) {
            uint32_t ia = indices[i], ib = indices[i+1], ic = indices[i+2];
            glm::vec3 a = outPos[ia]; glm::vec3 b = outPos[ib]; glm::vec3 c = outPos[ic];
            glm::vec3 n = glm::normalize(glm::cross(b - a, c - a));
            outNorm[ia] += n; outNorm[ib] += n; outNorm[ic] += n;
        }
        for (auto &n : outNorm) n = glm::normalize(n);
    }

    // build final vertex array
    mesh.vertices.reserve(outPos.size() * 8);
    for (size_t i = 0; i < outPos.size(); ++i) {
        mesh.vertices.push_back(outPos[i].x);
        mesh.vertices.push_back(outPos[i].y);
        mesh.vertices.push_back(outPos[i].z);
        mesh.vertices.push_back(outNorm[i].x);
        mesh.vertices.push_back(outNorm[i].y);
        mesh.vertices.push_back(outNorm[i].z);
        mesh.vertices.push_back(outUV[i].x);
        mesh.vertices.push_back(outUV[i].y);
    }
    mesh.indices = std::move(indices);
    return mesh;
}

static std::string trim(const std::string& s) {
    auto l = s.find_first_not_of(" \t\r\n");
    if (l == std::string::npos) return "";
    auto r = s.find_last_not_of(" \t\r\n");
    return s.substr(l, r - l + 1);
}

bool LevelLoader::load_from_file(const std::string& path, std::shared_ptr<engine::ecs::Registry> registry) {
    std::ifstream in(path);
    if (!in.is_open()) {
        std::cerr << "LevelLoader: failed to open file: " << path << "\n";
        return false;
    }

    // If given a JSON file, try the JSON loader helper
    if (path.size() >= 5) {
        auto ext = path.substr(path.size() - 5);
        std::transform(ext.begin(), ext.end(), ext.begin(), [](unsigned char c){ return std::tolower(c); });
        if (ext == ".json") {
            in.close();
            return load_from_json(path, registry);
        }
    }

    std::string line;
    int created = 0;
    while (std::getline(in, line)) {
        line = trim(line);
        if (line.empty()) continue;
        if (line[0] == '#') continue;

        std::istringstream iss(line);
        std::string type;
        if (!(iss >> type)) continue;
        // lowercase
        std::transform(type.begin(), type.end(), type.begin(), [](unsigned char c){ return std::tolower(c); });
        float px=0, py=0, pz=0;
        float sx=1, sy=1, sz=1;
        float r=0.8f, g=0.8f, b=0.8f;
        float rx=0, ry=0, rz=0;
        bool has_rot = false;

        TriangleMesh mesh;

        if (type == "mesh") {
            std::string meshPath;
            if (!(iss >> meshPath)) {
                std::cerr << "LevelLoader: malformed mesh line (missing path): " << line << "\n";
                continue;
            }
            if (!(iss >> px >> py >> pz)) {
                std::cerr << "LevelLoader: malformed mesh line (missing pos): " << line << "\n";
                continue;
            }

            std::string next;
            if (iss >> next) {
                if (next == "rot") {
                    if (iss >> rx >> ry >> rz) has_rot = true;
                } else {
                    // treat as scale first token
                    std::stringstream ss(next);
                    if (!(ss >> sx)) { sx = sy = sz = 1.0f; }
                    else {
                        if (!(iss >> sy >> sz)) { sy = sz = sx; }
                    }
                }
            }
            // optional color
            if (!(iss >> r >> g >> b)) { r = g = b = 0.8f; }

            // resolve mesh path relative to level file
            std::string basePath;
            auto pos = path.find_last_of("/\\");
            if (pos != std::string::npos) basePath = path.substr(0, pos+1);
            std::string fullMeshPath = basePath + meshPath;
            mesh = load_obj_mesh(fullMeshPath);
            if (mesh.vertices.empty()) {
                std::cerr << "LevelLoader: failed to load mesh: " << fullMeshPath << "\n";
                continue;
            }
        } else {
            if (!(iss >> px >> py >> pz)) {
                std::cerr << "LevelLoader: malformed line (missing pos): " << line << "\n";
                continue;
            }

            // optionally accept 'rot' keyword before scale
            std::string peek;
            if (iss >> peek) {
                if (peek == "rot") {
                    if (iss >> rx >> ry >> rz) has_rot = true;
                    // after rot, try to read scale
                    if (!(iss >> sx >> sy >> sz)) {
                        sx = sy = sz = 1.0f;
                    }
                } else {
                    // peek was probably a scale value
                    std::stringstream ss(peek);
                    if ((ss >> sx)) {
                        if (!(iss >> sy >> sz)) { sy = sz = sx; }
                    } else {
                        sx = sy = sz = 1.0f;
                    }
                }
            } else {
                sx = sy = sz = 1.0f;
            }

            // optional color
            if (!(iss >> r >> g >> b)) {
                r = g = b = 0.8f;
            }

                if (type == "cube") {
                    mesh = generate_cube_mesh();
                } else if (type == "plane") {
                    mesh = generate_plane_mesh();
                } else if (type == "sphere") {
                    mesh = generate_sphere_mesh();
                } else if (type == "cylinder") {
                    mesh = generate_cylinder_mesh();
                } else if (type == "wedge") {
                    mesh = generate_wedge_mesh();
                } else if (type == "stairs" || type == "stair") {
                    // optionally allow steps count after 'stairs' token (not required)
                    int steps = 8;
                    std::string maybe;
                    std::streampos saved = iss.tellg();
                    if (iss >> maybe) {
                        try { if (maybe == "steps" && (iss >> steps)) { /* consumed */ } else { /* nothing */ } }
                        catch(...) { /* ignore */ }
                    }
                    // reset stream if we accidentally read non-step token (we won't need it)
                    mesh = generate_stairs_mesh(steps);
                } else if (type == "torus") {
                    mesh = generate_torus_mesh();
                } else if (type == "door" || type == "window" || type == "platform") {
                    mesh = generate_cube_mesh();
                } else {
                    std::cerr << "LevelLoader: unknown shape type: " << type << "\n";
                    continue;
                }
        }

        // create GPU buffers
        engine::graphics::GeometryBuffer buf;
        buf.create(mesh.vertices, mesh.indices);

        auto ent = registry->create();
        auto& t = registry->emplace<engine::ecs::TransformComponent>(ent);
        t.position = glm::vec3(px, py, pz);
        t.scale = glm::vec3(sx, sy, sz);
        if (has_rot) t.rotation = glm::vec3(glm::radians(rx), glm::radians(ry), glm::radians(rz));

        auto& m = registry->emplace<engine::ecs::MeshComponent>(ent);
        m.vao = buf.vao;
        m.vbo = buf.vbo;
        m.ebo = buf.ebo;
        m.index_count = static_cast<uint32_t>(mesh.indices.size());
        m.draw_mode = GL_TRIANGLES;
        m.color = glm::vec3(r, g, b);

        // attach logical components for special types
        if (type == "door") {
            registry->emplace<engine::ecs::DoorComponent>(ent);
        } else if (type == "window") {
            registry->emplace<engine::ecs::WindowComponent>(ent);
        } else if (type == "platform") {
            registry->emplace<engine::ecs::PlatformComponent>(ent);
        }

        ++created;
    }

    in.close();
    if (created == 0) {
        std::cerr << "LevelLoader: no entities created from file: " << path << "\n";
    }
    return created > 0;
}

bool LevelLoader::load_from_json(const std::string& path, std::shared_ptr<engine::ecs::Registry> registry) {
    using nlohmann::json;
    std::ifstream in(path);
    if (!in.is_open()) {
        std::cerr << "LevelLoader: failed to open json file: " << path << "\n";
        return false;
    }
    json root;
    try {
        in >> root;
    } catch (const std::exception &e) {
        std::cerr << "LevelLoader: json parse error: " << e.what() << "\n";
        return false;
    }

    if (!root.is_array()) {
        std::cerr << "LevelLoader: expected JSON array at top-level in: " << path << "\n";
        return false;
    }

    int created = 0;
    for (const auto &obj : root) {
        if (!obj.is_object()) continue;
        std::string type = obj.value("type", "");
        std::transform(type.begin(), type.end(), type.begin(), [](unsigned char c){ return std::tolower(c); });
        if (type.empty()) continue;

        float px = obj.value("px", 0.0f);
        float py = obj.value("py", 0.0f);
        float pz = obj.value("pz", 0.0f);
        float sx = obj.value("sx", 1.0f);
        float sy = obj.value("sy", sx);
        float sz = obj.value("sz", sx);
        float r = obj.value("r", 0.8f);
        float g = obj.value("g", 0.8f);
        float b = obj.value("b", 0.8f);
        float rx = 0.0f, ry = 0.0f, rz = 0.0f;
        bool has_rot = false;
        if (obj.contains("rot") && obj["rot"].is_array() && obj["rot"].size() >= 3) {
            rx = obj["rot"][0].get<float>();
            ry = obj["rot"][1].get<float>();
            rz = obj["rot"][2].get<float>();
            has_rot = true;
        } else {
            rx = obj.value("rx", 0.0f);
            ry = obj.value("ry", 0.0f);
            rz = obj.value("rz", 0.0f);
            has_rot = (rx != 0.0f || ry != 0.0f || rz != 0.0f);
        }

        TriangleMesh mesh;
        if (type == "mesh") {
            std::string meshPath = obj.value("path", "");
            if (meshPath.empty()) continue;
            std::string basePath;
            auto p = path.find_last_of("/\\");
            if (p != std::string::npos) basePath = path.substr(0, p+1);
            mesh = load_obj_mesh(basePath + meshPath);
            if (mesh.vertices.empty()) continue;
        } else if (type == "cube") {
            mesh = generate_cube_mesh();
        } else if (type == "plane") {
            mesh = generate_plane_mesh();
        } else if (type == "sphere") {
            mesh = generate_sphere_mesh();
        } else if (type == "cylinder") {
            mesh = generate_cylinder_mesh();
        } else if (type == "wedge") {
            mesh = generate_wedge_mesh();
        } else if (type == "stairs" || type == "stair") {
            int steps = obj.value("steps", 8);
            mesh = generate_stairs_mesh(steps);
        } else if (type == "torus") {
            mesh = generate_torus_mesh();
        } else if (type == "door" || type == "window" || type == "platform") {
            mesh = generate_cube_mesh();
        } else {
            continue;
        }

        engine::graphics::GeometryBuffer buf;
        buf.create(mesh.vertices, mesh.indices);

        auto ent = registry->create();
        auto& t = registry->emplace<engine::ecs::TransformComponent>(ent);
        t.position = glm::vec3(px, py, pz);
        t.scale = glm::vec3(sx, sy, sz);
        if (has_rot) t.rotation = glm::vec3(glm::radians(rx), glm::radians(ry), glm::radians(rz));

        auto& m = registry->emplace<engine::ecs::MeshComponent>(ent);
        m.vao = buf.vao; m.vbo = buf.vbo; m.ebo = buf.ebo; m.index_count = static_cast<uint32_t>(mesh.indices.size()); m.draw_mode = GL_TRIANGLES; m.color = glm::vec3(r,g,b);

        if (type == "door") registry->emplace<engine::ecs::DoorComponent>(ent);
        if (type == "window") registry->emplace<engine::ecs::WindowComponent>(ent);
        if (type == "platform") registry->emplace<engine::ecs::PlatformComponent>(ent);

        ++created;
    }

    if (created == 0) {
        std::cerr << "LevelLoader: no entities created from JSON file: " << path << "\n";
    }
    return created > 0;
}

} // namespace level
} // namespace engine
