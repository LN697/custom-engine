#pragma once

#include <string>

namespace engine {

class TextRenderer {
public:
    TextRenderer();
    void set_text(const std::string& text);
    void start();
    void update(float dt);
    void render();
    void set_chars_per_second(float cps) { chars_per_second_ = cps; }

private:
    std::string full_text_;
    size_t visible_chars_;
    float elapsed_;
    float chars_per_second_;
    bool playing_;
};

} // namespace engine
