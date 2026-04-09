#include "engine/input_manager.h"
#include <SDL2/SDL.h>

namespace engine {

InputManager::InputManager()
    : keys_(SDL_GetKeyboardState(&num_keys_)), quit_requested_(false) {
}

InputManager::~InputManager() {
}

void InputManager::update() {
    // Update keyboard state - pump events first to ensure state is current
    SDL_PumpEvents();
    keys_ = SDL_GetKeyboardState(&num_keys_);
}

void InputManager::handle_event(const SDL_Event& event) {
    switch (event.type) {
        case SDL_QUIT:
            quit_requested_ = true;
            break;
        case SDL_KEYDOWN:
            // Q quits the application
            if (event.key.keysym.sym == SDLK_q) {
                quit_requested_ = true;
            }
            break;
        case SDL_KEYUP:
            break;
        default:
            break;
    }
}

int InputManager::look_left() const {
    return (keys_[SDL_SCANCODE_LEFT] ? 1 : 0);
}

int InputManager::look_right() const {
    return (keys_[SDL_SCANCODE_RIGHT] ? 1 : 0);
}

int InputManager::look_up() const {
    return (keys_[SDL_SCANCODE_UP] ? 1 : 0);
}

int InputManager::look_down() const {
    return (keys_[SDL_SCANCODE_DOWN] ? 1 : 0);
}

} // namespace engine
