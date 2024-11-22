#pragma once

#include <string>
#include <functional>
#include <GLFW/glfw3.h>
#include <zfw2/input.h>
#include <zfw2/assets.h>
#include <zfw2/graphics.h>
#include <zfw2/scenes.h>

namespace zfw2
{

constexpr int gk_targTicksPerSec = 60;
constexpr double gk_targTickDur = 1.0 / gk_targTicksPerSec;

using SceneFactory = std::function<std::unique_ptr<Scene>()>;

template<typename T>
SceneFactory create_scene_factory()
{
    return []() { return std::make_unique<T>(); };
}

class Game
{
public:
    ~Game();
    Game(const Game &other) = delete;
    Game &operator=(const Game &other) = delete;

    Game::Game(const std::string &windowTitle) : m_windowTitle(windowTitle)
    {
    }

    void run(const SceneFactory initSceneFactory);

private:
    const std::string m_windowTitle;

    bool m_glfwInitialized = false;
    GLFWwindow *m_glfwWindow = nullptr;

    InputManager m_inputManager;

    Assets m_assets;
    InternalShaderProgs m_internalShaderProgs;

    std::unique_ptr<Scene> m_scene;

    static inline double Game::calc_valid_frame_dur(const double frameTime, const double frameTimeLast)
    {
        const double dur = frameTime - frameTimeLast;
        return dur >= 0.0 && dur <= gk_targTickDur * 8.0 ? dur : 0.0;
    }

    static inline void Game::glfw_window_size_callback(GLFWwindow *const window, const int width, const int height)
    {
        glViewport(0, 0, width, height);
    }

    static inline void Game::glfw_scroll_callback(GLFWwindow *const window, const double xOffs, const double yOffs)
    {
    }
};

constexpr int gk_glVersionMajor = 4;
constexpr int gk_glVersionMinor = 1;

}
