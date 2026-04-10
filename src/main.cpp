#include "engine/engine.h"
#include <iostream>

int main() {
    engine::Engine engine;

    if (!engine.initialize()) {
        std::cerr << "[MAIN] Failed to initialize engine.\n";
        return 1;
    }

    engine.run();

    engine.shutdown();

    return 0;
}
