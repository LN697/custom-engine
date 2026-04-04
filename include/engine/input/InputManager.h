#pragma once

#include <SDL2/SDL.h>
#include <deque>
#include <set>
#include <string>

namespace engine {

class InputManager {
public:
    InputManager();

    bool processEvent(const SDL_Event& event);
    void resetFrameState();

    bool quitRequested() const;
    bool moveForward() const;
    bool moveBackward() const;
    bool moveLeft() const;
    bool moveRight() const;
    bool moveUp() const;
    bool moveDown() const;

    int mouseDeltaX() const;
    int mouseDeltaY() const;
    std::string keySummary() const;
    std::string mouseSummary() const;
    std::string eventHistory() const;

private:
    void recordKey(SDL_Keycode key, bool pressed);
    void appendHistory(const std::string& entry);
    std::string mapKeyName(SDL_Keycode key) const;

    bool quitRequested_;
    bool movingForward_;
    bool movingBackward_;
    bool movingLeft_;
    bool movingRight_;
    bool movingUp_;
    bool movingDown_;

    int mouseDeltaX_;
    int mouseDeltaY_;
    int mouseX_;
    int mouseY_;

    std::set<SDL_Keycode> keysDown_;
    std::deque<std::string> eventLog_;
};

} // namespace engine
