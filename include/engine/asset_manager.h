#pragma once

#include <glm/glm.hpp>

namespace engine {

class AssetManager {
public:
    AssetManager();
    ~AssetManager();

    bool load_texture(const char* path);
    bool load_shader(const char* path);
    bool load_model(const char* path);

private:
    // Asset storage would go here
};

} // namespace engine
