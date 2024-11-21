#include <zfw2/input.h>

namespace zfw2
{

static int get_glfw_key_code(const KeyCode keyCode)
{
    switch (keyCode)
    {
        case KeyCode::Space: return GLFW_KEY_SPACE;

        case KeyCode::Num0: return GLFW_KEY_0;
        case KeyCode::Num1: return GLFW_KEY_1;
        case KeyCode::Num2: return GLFW_KEY_2;
        case KeyCode::Num3: return GLFW_KEY_3;
        case KeyCode::Num4: return GLFW_KEY_4;
        case KeyCode::Num5: return GLFW_KEY_5;
        case KeyCode::Num6: return GLFW_KEY_6;
        case KeyCode::Num7: return GLFW_KEY_7;
        case KeyCode::Num8: return GLFW_KEY_8;
        case KeyCode::Num9: return GLFW_KEY_9;

        case KeyCode::A: return GLFW_KEY_A;
        case KeyCode::B: return GLFW_KEY_B;
        case KeyCode::C: return GLFW_KEY_C;
        case KeyCode::D: return GLFW_KEY_D;
        case KeyCode::E: return GLFW_KEY_E;
        case KeyCode::F: return GLFW_KEY_F;
        case KeyCode::G: return GLFW_KEY_G;
        case KeyCode::H: return GLFW_KEY_H;
        case KeyCode::I: return GLFW_KEY_I;
        case KeyCode::J: return GLFW_KEY_J;
        case KeyCode::K: return GLFW_KEY_K;
        case KeyCode::L: return GLFW_KEY_L;
        case KeyCode::M: return GLFW_KEY_M;
        case KeyCode::N: return GLFW_KEY_N;
        case KeyCode::O: return GLFW_KEY_O;
        case KeyCode::P: return GLFW_KEY_P;
        case KeyCode::Q: return GLFW_KEY_Q;
        case KeyCode::R: return GLFW_KEY_R;
        case KeyCode::S: return GLFW_KEY_S;
        case KeyCode::T: return GLFW_KEY_T;
        case KeyCode::U: return GLFW_KEY_U;
        case KeyCode::V: return GLFW_KEY_V;
        case KeyCode::W: return GLFW_KEY_W;
        case KeyCode::X: return GLFW_KEY_X;
        case KeyCode::Y: return GLFW_KEY_Y;
        case KeyCode::Z: return GLFW_KEY_Z;

        case KeyCode::Escape: return GLFW_KEY_ESCAPE;
        case KeyCode::Enter: return GLFW_KEY_ENTER;
        case KeyCode::Tab: return GLFW_KEY_TAB;

        case KeyCode::Right: return GLFW_KEY_RIGHT;
        case KeyCode::Left: return GLFW_KEY_LEFT;
        case KeyCode::Down: return GLFW_KEY_DOWN;
        case KeyCode::Up: return GLFW_KEY_UP;

        case KeyCode::F1: return GLFW_KEY_F1;
        case KeyCode::F2: return GLFW_KEY_F2;
        case KeyCode::F3: return GLFW_KEY_F3;
        case KeyCode::F4: return GLFW_KEY_F4;
        case KeyCode::F5: return GLFW_KEY_F5;
        case KeyCode::F6: return GLFW_KEY_F6;
        case KeyCode::F7: return GLFW_KEY_F7;
        case KeyCode::F8: return GLFW_KEY_F8;
        case KeyCode::F9: return GLFW_KEY_F9;
        case KeyCode::F10: return GLFW_KEY_F10;
        case KeyCode::F11: return GLFW_KEY_F11;
        case KeyCode::F12: return GLFW_KEY_F12;

        case KeyCode::LeftShift: return GLFW_KEY_LEFT_SHIFT;
        case KeyCode::LeftControl: return GLFW_KEY_LEFT_CONTROL;
        case KeyCode::LeftAlt: return GLFW_KEY_LEFT_ALT;
    }

    return GLFW_KEY_UNKNOWN;
}

static KeysDownBits get_keys_down_bits(GLFWwindow *const glfwWindow)
{
    KeysDownBits keysDownBits = 0;

    for (int i = 0; i < static_cast<int>(KeyCode::CNT); ++i)
    {
        if (glfwGetKey(glfwWindow, get_glfw_key_code(static_cast<KeyCode>(i))) == GLFW_PRESS)
        {
            keysDownBits |= static_cast<KeysDownBits>(1) << i;
        }
    }

    return keysDownBits;
}

static MouseButtonsDownBits get_mouse_buttons_down_bits(GLFWwindow *const glfwWindow)
{
    MouseButtonsDownBits mouseButtonsDownBits = 0;

    for (int i = 0; i < static_cast<int>(MouseButtonCode::CNT); ++i)
    {
        if (glfwGetMouseButton(glfwWindow, i) == GLFW_PRESS)
        {
            mouseButtonsDownBits |= static_cast<MouseButtonsDownBits>(1) << i;
        }
    }

    return mouseButtonsDownBits;
}

static zfw2_common::Vec2D get_mouse_pos(GLFWwindow *const glfwWindow)
{
    double mouseXDbl, mouseYDbl;
    glfwGetCursorPos(glfwWindow, &mouseXDbl, &mouseYDbl);
    return {static_cast<float>(mouseXDbl), static_cast<float>(mouseYDbl)};
}

static GamepadState create_gamepad_state()
{
    GamepadState state = {};

    // Search for the first active gamepad and if found update the gamepad state using it.
    for (int i = 0; i <= GLFW_JOYSTICK_LAST; i++)
    {
        if (!glfwJoystickPresent(i) || !glfwJoystickIsGamepad(i))
        {
            continue;
        }

        GLFWgamepadstate glfwGamepadState;

        if (!glfwGetGamepadState(i, &glfwGamepadState))
        {
            break;
        }

        state.connected = true;
        state.glfwJoystickIndex = i;

        // Store which gamepad buttons are down.
        for (int j = 0; j < static_cast<int>(GamepadButtonCode::CNT); j++)
        {
            if (glfwGamepadState.buttons[j] == GLFW_PRESS)
            {
                state.buttonsDownBits |= static_cast<GamepadButtonsDownBits>(1) << j;
            }
        }

        // Store gamepad axis values.
        for (int j = 0; j < static_cast<int>(GamepadAxisCode::CNT); j++)
        {
            state.axisValues[j] = glfwGamepadState.axes[j];
        }

        break;
    }

    return state;
}

void InputManager::refresh(GLFWwindow *const glfwWindow, const int mouseScroll)
{
    m_stateLast = m_state;
    m_state = create_input_state(glfwWindow, mouseScroll);
}

InputState create_input_state(GLFWwindow *const glfwWindow, const int mouseScroll)
{
    InputState state;
    state.keysDownBits = get_keys_down_bits(glfwWindow);
    state.mouseButtonsDownBits = get_mouse_buttons_down_bits(glfwWindow);
    state.mousePos = get_mouse_pos(glfwWindow);
    state.mouseScroll = mouseScroll;
    state.gamepadState = create_gamepad_state();
    return state;
}

}
