# Control Schemes System - Usage Guide

## Overview

The custom engine now has a **decoupled control schemes system** that allows you to dynamically switch between different input control styles:

- **Minecraft-style**: Free movement (WASD horizontal + Space/Ctrl vertical), classic survival mode controls
- **FPS-style**: Traditional first-person shooter controls (WASD movement, sprint modifier, action buttons)
- **Doom-style**: Decoupled movement and view (strafe-based movement, arrow keys for turning)

## Architecture

### Core Components

1. **ControlScheme** (`include/engine/input/control_scheme.h`)
   - Abstract base class defining the interface for all control schemes
   - Methods: `update()`, `get_movement_direction()`, `get_look_input()`, `is_action_pressed()`

2. **Concrete Implementations**
   - `MinecraftControlScheme` - Vertical flying + horizontal movement
   - `FPSControlScheme` - Classic FPS with sprint modifier
   - `DoomControlScheme` - Decoupled strafe-based movement

3. **Scene Integration**
   - All scenes now accept control schemes via `set_control_scheme()`
   - Each scene can have its own control scheme

4. **Camera Updates**
   - Camera supports both `InputManager` (legacy) and `ControlScheme` (new) variants
   - `camera_.update(delta_time, *control_scheme_)`

5. **Engine Integration**
   - Engine initializes with a default control scheme (Minecraft)
   - Method: `engine.set_scene_control_scheme(scheme)` to switch schemes dynamically

## Usage Examples

### Setting a Control Scheme at Engine Initialization

```cpp
// In Engine::initialize() or where you load your scene
auto minecraft_scheme = std::make_shared<MinecraftControlScheme>();
engine.set_scene_control_scheme(minecraft_scheme);
```

### Switching Control Schemes at Runtime

```cpp
// In your update loop or event handler
if (user_pressed_key_1) {
    engine.set_scene_control_scheme(std::make_shared<MinecraftControlScheme>());
}
else if (user_pressed_key_2) {
    engine.set_scene_control_scheme(std::make_shared<FPSControlScheme>());
}
else if (user_pressed_key_3) {
    engine.set_scene_control_scheme(std::make_shared<DoomControlScheme>());
}
```

### Implementing a Custom Control Scheme

Create a new header `include/engine/input/my_custom_scheme.h`:

```cpp
#pragma once
#include "engine/input/control_scheme.h"
#include <unordered_map>
#include <string>

class MyCustomScheme : public engine::ControlScheme {
public:
    void update(const engine::InputManager& input_manager, float delta_time) override;
    glm::vec3 get_movement_direction() const override { return movement_; }
    glm::vec2 get_look_input() const override { return look_; }
    bool is_action_pressed(const std::string& action) const override;

private:
    glm::vec3 movement_;
    glm::vec2 look_;
    std::unordered_map<std::string, bool> actions_;
};
```

Implement in `src/engine/input/my_custom_scheme.cpp`:

```cpp
void MyCustomScheme::update(const InputManager& input, float dt) {
    // Your custom logic here
    movement_ = glm::vec3(0.0f);
    if (input.move_forward()) movement_.z -= 1.0f;
    // ... etc
}
```

## Control Scheme Details

### Minecraft-Style
- **Movement**: WASD for horizontal, Space for up, Ctrl for down
- **View**: Arrow keys or mouse look
- **Flying**: Vertical movement in all directions
- **Best for**: Open-world exploration, creative/building games

### FPS-Style  
- **Movement**: WASD relative to camera direction
- **View**: Mouse look
- **Sprint**: Shift key (TODO: needs InputManager extension)
- **Actions**: E (interact), Left-Click (primary), Right-Click (secondary)
- **Best for**: Tactical shooters, conventional FPS games

### Doom-Style
- **Movement**: WASD independent of view (true strafing)
- **View**: Arrow keys for turning, mouse for vertical look
- **Actions**: Space (jump), Ctrl (crouch), E (use), Left-Click (fire)
- **Best for**: Retro-style games, arcade-feel experiences

## Extending the System

### Adding More Actions to Control Schemes

1. Update the control scheme's `update()` method to detect new key presses
2. Store action states in the `actions_` map
3. Implement `is_action_pressed(action_name)` to query actions from scenes

### InputManager Enhancements Needed

The current `InputManager` could be extended to support:
- Mouse button detection (left, right, middle click)
- Shift/Alt/Ctrl modifier keys
- Custom key bindings
- Gamepad/controller input

Additional keys in InputManager:
```cpp
bool is_shift_pressed() const;
bool is_mouse_button_pressed(int button) const;
bool is_key_pressed(SDL_Scancode code) const;
```

## Integration with Scenes

Scenes already support control schemes through the base `Scene` class:

```cpp
class Scene {
    // ...
    void set_control_scheme(std::shared_ptr<ControlScheme> scheme) { control_scheme_ = scheme; }
    std::shared_ptr<ControlScheme> get_control_scheme() const { return control_scheme_; }
protected:
    std::shared_ptr<ControlScheme> control_scheme_;
};
```

Use in your scene:
```cpp
void MyScene::update(float dt) {
    if (control_scheme_) {
        auto movement = control_scheme_->get_movement_direction();
        bool is_jumping = control_scheme_->is_action_pressed("jump");
        // Apply movement/actions to your entities
    }
}
```

## Performance Considerations

- Control schemes are lightweight and update in O(1) time
- Movement direction calculation uses normalized GLM vectors
- Mouse input comes directly from SDL, not polled from InputManager
- Each control scheme can be customized for specific game needs

## Future Enhancements

1. **Gamepad Support**: Extend control schemes to support analog sticks and controllers
2. **Configurable Key Bindings**: Allow remapping keys per control scheme
3. **Combination Actions**: Support multi-key combinations (e.g., Shift+W for sprint)
4. **Context Sensitivity**: Different schemes for different game states (combat, exploration, vehicle)
5. **Input Smoothing**: Per-scheme input smoothing parameters
