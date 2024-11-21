#include <zfw2/game.h>

#include <glad/glad.h>

namespace zfw2
{

Game::Game(const std::string &windowTitle) : m_windowTitle(windowTitle)
{
}

Game::~Game()
{
    if (m_glfwWindow)
    {
        glfwDestroyWindow(m_glfwWindow);
    }

    if (m_glfwInitialized)
    {
        glfwTerminate();
    }
}

void Game::run()
{
    //
    // Initialisation
    //

    // Initialise GLFW.
    if (!glfwInit())
    {
        return;
    }

    m_glfwInitialized = true;

    // Create the GLFW window.
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, gk_glVersionMajor);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, gk_glVersionMinor);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    glfwWindowHint(GLFW_VISIBLE, false); // Show the window later once other systems have been set up.

    m_glfwWindow = glfwCreateWindow(1280, 720, m_windowTitle.c_str(), nullptr, nullptr);

    if (!m_glfwWindow)
    {
        return;
    }

    glfwMakeContextCurrent(m_glfwWindow);

    // Set GLFW window callbacks.
    glfwSetWindowSizeCallback(m_glfwWindow, glfw_window_size_callback);
    glfwSetScrollCallback(m_glfwWindow, glfw_scroll_callback);

    // Hide the cursor.
    glfwSetInputMode(m_glfwWindow, GLFW_CURSOR, GLFW_CURSOR_HIDDEN);

    // Initialise OpenGL function pointers.
    if (!gladLoadGLLoader(reinterpret_cast<GLADloadproc>(glfwGetProcAddress)))
    {
        return;
    }

    // Load assets.
    if (!m_assets.load_all("assets.zfw2dat"))
    {
        return;
    }

    // Enable blending.
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    // Set up input.
    int glfwCallbackMouseScroll = 0; // This is an axis representing the scroll wheel movement. It is updated by the GLFW scroll callback and gets reset after a new input state is generated.
    glfwSetWindowUserPointer(m_glfwWindow, &glfwCallbackMouseScroll);

    // Run the user-defined initialisation function.
    on_init();

    // Show the window now that things have been set up.
    glfwShowWindow(m_glfwWindow);

    //
    // Main Loop
    //
    double frameTime = glfwGetTime();
    double frameDurAccum = 0.0;

    while (!glfwWindowShouldClose(m_glfwWindow))
    {
        glfwPollEvents();

        const double frameTimeLast = frameTime;
        frameTime = glfwGetTime();

        const double frameDur = calc_valid_frame_dur(frameTime, frameTimeLast);
        frameDurAccum += frameDur;

        const int tickCnt = frameDurAccum / gk_targTickDur;

        if (tickCnt > 0)
        {
            m_inputManager.refresh(m_glfwWindow, glfwCallbackMouseScroll);
            glfwCallbackMouseScroll = 0;

            // Execute ticks.
            int i = 0;

            do
            {
                on_tick();
                frameDurAccum -= gk_targTickDur;
                ++i;
            }
            while (i < tickCnt);
        }

        // Render.
        glfwSwapBuffers(m_glfwWindow);
    }
}

inline double Game::calc_valid_frame_dur(const double frameTime, const double frameTimeLast)
{
    const double dur = frameTime - frameTimeLast;
    return dur >= 0.0 && dur <= gk_targTickDur * 8.0 ? dur : 0.0;
}

inline void Game::glfw_window_size_callback(GLFWwindow *const window, const int width, const int height)
{
    glViewport(0, 0, width, height);
}

inline void Game::glfw_scroll_callback(GLFWwindow *const window, const double xOffs, const double yOffs)
{
}

}
