#pragma once

#include <string>
#include <GLFW/glfw3.h>
#include <zfw2/input.h>
#include <zfw2/assets.h>

namespace zfw2
{

class Game
{
public:
    Game(const std::string &windowTitle);
    ~Game();

    Game(const Game &other) = delete;
    Game &operator=(const Game &other) = delete;

    void run();

protected:
    virtual void on_init() = 0;
    virtual void on_tick() = 0;

    const InputManager &get_input_manager() const
    {
        return m_inputManager;
    }

    const Assets &get_assets() const
    {
        return m_assets;
    }

private:
    const std::string m_windowTitle;

    bool m_glfwInitialized = false;
    GLFWwindow *m_glfwWindow = nullptr;

    InputManager m_inputManager;

    Assets m_assets;
    InternalShaderProgs m_internalShaderProgs;

    static inline double calc_valid_frame_dur(const double frameTime, const double frameTimeLast);
    static inline void glfw_window_size_callback(GLFWwindow *const window, const int width, const int height);
    static inline void glfw_scroll_callback(GLFWwindow *const window, const double xOffs, const double yOffs);
};

constexpr int gk_glVersionMajor = 4;
constexpr int gk_glVersionMinor = 1;

constexpr int gk_targTicksPerSec = 60;
constexpr double gk_targTickDur = 1.0 / gk_targTicksPerSec;

}
