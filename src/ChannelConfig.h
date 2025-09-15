#pragma once
#include <SDL.h>
#include <variant>

enum class InputType {
    None,
    Keyboard,
    JoystickButton,
    JoystickAxis
};

struct JoystickButton {
    Uint8 button;
    SDL_JoystickID joystick_id;
};

struct JoystickAxis {
    Uint8 axis;
    SDL_JoystickID joystick_id;
};

enum class ChannelModes {
    NONE,
    RAW,
    TAP,
    HOLD,
    RELEASE,
    INCREMENT,
    TOGGLE,
    TOGGLE_SYMETRIC,
};

struct ChannelConfig {
    int channel = -1;
    InputType type = InputType::None;
    SDL_Event raw_event;

    using InputVariant = std::variant<std::monostate, SDL_Keycode, JoystickButton, JoystickAxis>;

    InputVariant input_data;
    int offset;
    ChannelModes mode = ChannelModes::NONE;
};
