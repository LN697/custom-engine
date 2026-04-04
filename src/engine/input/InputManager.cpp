#include "engine/input/InputManager.h"

namespace engine {

InputManager::InputManager()
    : quitRequested_(false),
      movingForward_(false),
      movingBackward_(false),
      movingLeft_(false),
      movingRight_(false),
      movingUp_(false),
      movingDown_(false),
      mouseDeltaX_(0),
      mouseDeltaY_(0),
      mouseX_(0),
      mouseY_(0) {
}

bool InputManager::processEvent(const SDL_Event& event) {
    switch (event.type) {
        case SDL_QUIT:
            quitRequested_ = true;
            appendHistory("Quit requested");
            break;

        // FIX: Prevent "Sticky Keys" on focus loss
        case SDL_WINDOWEVENT:
            if (event.window.event == SDL_WINDOWEVENT_FOCUS_LOST) {
                keysDown_.clear();
                movingForward_ = movingBackward_ = movingLeft_ = movingRight_ = movingUp_ = movingDown_ = false;
                appendHistory("Focus lost: Keys cleared");
            }
            break;

        case SDL_KEYDOWN:
            if (!event.key.repeat) {
                recordKey(event.key.keysym.sym, true);
                appendHistory("Key down: " + mapKeyName(event.key.keysym.sym));
            }
            if (event.key.keysym.sym == SDLK_ESCAPE) {
                quitRequested_ = true;
            }
            break;

        case SDL_KEYUP:
            recordKey(event.key.keysym.sym, false);
            appendHistory("Key up: " + mapKeyName(event.key.keysym.sym));
            break;

        case SDL_MOUSEMOTION:
            mouseDeltaX_ += event.motion.xrel;
            mouseDeltaY_ += event.motion.yrel;
            mouseX_ = event.motion.x;
            mouseY_ = event.motion.y;
            appendHistory("Mouse motion: dx=" + std::to_string(event.motion.xrel) + " dy=" + std::to_string(event.motion.yrel));
            break;

        case SDL_MOUSEBUTTONDOWN:
            appendHistory("Mouse button down: " + std::to_string(event.button.button));
            break;

        case SDL_MOUSEBUTTONUP:
            appendHistory("Mouse button up: " + std::to_string(event.button.button));
            break;

        default:
            break;
    }

    return quitRequested_;
}

void InputManager::resetFrameState() {
    mouseDeltaX_ = 0;
    mouseDeltaY_ = 0;
}

bool InputManager::quitRequested() const {
    return quitRequested_;
}

bool InputManager::moveForward() const {
    return movingForward_;
}

bool InputManager::moveBackward() const {
    return movingBackward_;
}

bool InputManager::moveLeft() const {
    return movingLeft_;
}

bool InputManager::moveRight() const {
    return movingRight_;
}

bool InputManager::moveUp() const {
    return movingUp_;
}

bool InputManager::moveDown() const {
    return movingDown_;
}

int InputManager::mouseDeltaX() const {
    return mouseDeltaX_;
}

int InputManager::mouseDeltaY() const {
    return mouseDeltaY_;
}

std::string InputManager::keySummary() const {
    if (keysDown_.empty()) {
        return "None";
    }

    std::string result;
    for (SDL_Keycode key : keysDown_) {
        if (!result.empty()) {
            result += ", ";
        }
        result += mapKeyName(key);
    }
    return result;
}

std::string InputManager::mouseSummary() const {
    return "dx=" + std::to_string(mouseDeltaX_) + " dy=" + std::to_string(mouseDeltaY_);
}

std::string InputManager::eventHistory() const {
    std::string result;
    for (const auto& entry : eventLog_) {
        if (!result.empty()) {
            result += " | ";
        }
        result += entry;
    }
    return result;
}

void InputManager::recordKey(SDL_Keycode key, bool pressed) {
    if (pressed) {
        keysDown_.insert(key);
    } else {
        keysDown_.erase(key);
    }

    switch (key) {
        case SDLK_w:
        case SDLK_UP:
            movingForward_ = pressed;
            break;
        case SDLK_s:
        case SDLK_DOWN:
            movingBackward_ = pressed;
            break;
        case SDLK_a:
        case SDLK_LEFT:
            movingLeft_ = pressed;
            break;
        case SDLK_d:
        case SDLK_RIGHT:
            movingRight_ = pressed;
            break;
        case SDLK_SPACE:
        case SDLK_e:
            movingUp_ = pressed;
            break;
        case SDLK_LCTRL:
        case SDLK_RCTRL:
        case SDLK_q:
            movingDown_ = pressed;
            break;
        default:
            break;
    }
}

void InputManager::appendHistory(const std::string& entry) {
    eventLog_.push_back(entry);
    if (eventLog_.size() > 12) {
        eventLog_.pop_front();
    }
}

std::string InputManager::mapKeyName(SDL_Keycode key) const {
    const char* name = SDL_GetKeyName(key);
    return name ? std::string(name) : std::string("Unknown");
}

} // namespace engine
