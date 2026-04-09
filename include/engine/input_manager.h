#pragma once

#include <SDL2/SDL.h>

namespace engine {

class InputManager {
public:
    InputManager();
    ~InputManager();

    void update();
    void handle_event(const SDL_Event& event);

    bool move_forward() const { return keys_[SDL_SCANCODE_W]; }
    bool move_backward() const { return keys_[SDL_SCANCODE_S]; }
    bool move_left() const { return keys_[SDL_SCANCODE_A]; }
    bool move_right() const { return keys_[SDL_SCANCODE_D]; }
    bool move_up() const { return keys_[SDL_SCANCODE_SPACE]; }
    bool move_down() const { return keys_[SDL_SCANCODE_LCTRL]; }

    int look_left() const;
    int look_right() const;
    int look_up() const;
    int look_down() const;

    bool is_quit_requested() const { return quit_requested_; }

private:
    const uint8_t* keys_;
    bool quit_requested_;
    int num_keys_;
};

} // namespace engine
