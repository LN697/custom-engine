#pragma once

#include <string>
#include <memory>

namespace engine {
namespace ecs {
class Registry;
}

namespace level {

class LevelLoader {
public:
    // Load a simple ASCII level file and spawn basic shapes into the provided registry.
    // Format (whitespace-separated per line) and extensions supported:
    //  # comment
    //  cube px py pz [rot rx ry rz] [sx sy sz] [r g b]
    //  plane px py pz [rot rx ry rz] [sx sy sz] [r g b]
    //  sphere px py pz [rot rx ry rz] [sx sy sz] [r g b]
    //  cylinder px py pz [rot rx ry rz] [sx sy sz] [r g b]
    //  wedge px py pz [rot rx ry rz] [sx sy sz] [r g b]
    //  stairs px py pz [steps N] [rot rx ry rz] [sx sy sz] [r g b]
    //  torus px py pz [rings sectors] [rot rx ry rz] [sx sy sz] [r g b]
    //  mesh <path> px py pz [rot rx ry rz] [sx sy sz] [r g b]
    //  door/window/platform px py pz [rot rx ry rz] [sx sy sz] [r g b]
    // Use the optional keyword `rot` followed by three Euler angles in degrees
    // for specifying rotation. If a file with extension `.json` is provided,
    // the loader will parse it using `nlohmann::json` (array of objects
    // with the same fields as above).
    // Returns true if file parsed and at least one entity created.
    static bool load_from_file(const std::string& path, std::shared_ptr<engine::ecs::Registry> registry);

    // Optionally load from a JSON file (internal helper).
    static bool load_from_json(const std::string& path, std::shared_ptr<engine::ecs::Registry> registry);
};

} // namespace level
} // namespace engine
