#pragma once

#include <GLFW/glfw3.h>
#include <zfw2_common/math.h>

namespace zfw2
{

enum class KeyCode
{
    Space,
    Num0, Num1, Num2, Num3, Num4, Num5, Num6, Num7, Num8, Num9,
    A, B, C, D, E, F, G, H, I, J, K, L, M, N, O, P, Q, R, S, T, U, V, W, X, Y, Z,
    Escape, Enter, Tab,
    Right, Left, Down, Up,
    F1, F2, F3, F4, F5, F6, F7, F8, F9, F10, F11, F12,
    LeftShift, LeftControl, LeftAlt,

    CNT
};

enum class MouseButtonCode
{
    Left,
    Right,
    Middle,

    CNT
};

enum class GamepadButtonCode
{
    A, B, X, Y,
    LeftBumper, RightBumper,
    Back, Start, Guide,
    LeftThumb, RightThumb,
    DpadUp, DpadRight, DpadDown, DpadLeft,

    CNT
};

enum class GamepadAxisCode
{
    LeftX, LeftY,
    RightX, RightY,
    LeftTrigger, RightTrigger,

    CNT
};

using KeysDownBits = unsigned long long;
using MouseButtonsDownBits = unsigned char;
using GamepadButtonsDownBits = unsigned short;

struct GamepadState
{
    bool connected; // Included to support zero-initialisation.
    int glfwJoystickIndex;
    GamepadButtonsDownBits buttonsDownBits;
    float axisValues[static_cast<int>(GamepadAxisCode::CNT)];
};

struct InputState
{
    KeysDownBits keysDownBits;

    MouseButtonsDownBits mouseButtonsDownBits;
    zfw2_common::Vec2D mousePos;
    int mouseScroll;

    GamepadState gamepadState;
};

class InputManager
{
public:
    void refresh(GLFWwindow *const glfwWindow, const int mouseScroll);

    inline bool is_key_down(const KeyCode keyCode) const
    {
        const auto keyBit = static_cast<KeysDownBits>(1) << static_cast<int>(keyCode);
        return m_state.keysDownBits & keyBit;
    }

    inline bool is_key_pressed(const KeyCode keyCode) const
    {
        const auto keyBit = static_cast<KeysDownBits>(1) << static_cast<int>(keyCode);
        return (m_state.keysDownBits & keyBit) && !(m_stateLast.keysDownBits & keyBit);
    }

    inline bool is_key_released(const KeyCode keyCode) const
    {
        const auto keyBit = static_cast<KeysDownBits>(1) << static_cast<int>(keyCode);
        return !(m_state.keysDownBits & keyBit) && (m_stateLast.keysDownBits & keyBit);
    }

    inline bool is_mouse_button_down(const MouseButtonCode buttonCode) const
    {
        const auto buttonBit = static_cast<MouseButtonsDownBits>(1) << static_cast<int>(buttonCode);
        return m_state.mouseButtonsDownBits & buttonBit;
    }

    inline bool is_mouse_button_pressed(const MouseButtonCode buttonCode) const
    {
        const auto buttonBit = static_cast<MouseButtonsDownBits>(1) << static_cast<int>(buttonCode);
        return (m_state.mouseButtonsDownBits & buttonBit) && !(m_stateLast.mouseButtonsDownBits & buttonBit);
    }

    inline bool is_mouse_button_released(const MouseButtonCode buttonCode) const
    {
        const auto buttonBit = static_cast<MouseButtonsDownBits>(1) << static_cast<int>(buttonCode);
        return !(m_state.mouseButtonsDownBits & buttonBit) && (m_stateLast.mouseButtonsDownBits & buttonBit);
    }

    inline zfw2_common::Vec2D get_mouse_pos() const
    {
        return m_state.mousePos;
    }

    inline int get_mouse_scroll() const
    {
        return m_state.mouseScroll;
    }

    inline bool is_gamepad_connected() const
    {
        return m_state.gamepadState.connected;
    }

    inline bool is_gamepad_button_down(const GamepadButtonCode buttonCode) const
    {
        const auto buttonBit = static_cast<GamepadButtonsDownBits>(1) << static_cast<int>(buttonCode);
        return m_state.gamepadState.buttonsDownBits & buttonBit;
    }

    inline bool is_gamepad_button_pressed(const GamepadButtonCode buttonCode) const
    {
        const auto buttonBit = static_cast<GamepadButtonsDownBits>(1) << static_cast<int>(buttonCode);
        return (m_state.gamepadState.buttonsDownBits & buttonBit) && !(m_stateLast.gamepadState.buttonsDownBits & buttonBit);
    }

    inline bool is_gamepad_button_released(const GamepadButtonCode buttonCode) const
    {
        const auto buttonBit = static_cast<GamepadButtonsDownBits>(1) << static_cast<int>(buttonCode);
        return !(m_state.gamepadState.buttonsDownBits & buttonBit) && (m_stateLast.gamepadState.buttonsDownBits & buttonBit);
    }

    inline float get_gamepad_axis_value(const GamepadAxisCode axisCode) const
    {
        return m_state.gamepadState.axisValues[static_cast<int>(axisCode)];
    }

private:
    InputState m_state = {};
    InputState m_stateLast = {};
};

InputState create_input_state(GLFWwindow *const glfwWindow, const int mouseScroll);

}
