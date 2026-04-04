#include "engine/Engine.h"
#include <iostream>

int main() {
    engine::Engine engine;

    if (!engine.initialize()) {
        std::cerr << "Failed to initialize engine.\n";
        return 1;
    }

    engine.run();
    engine.shutdown();

    return 0;
}
