#include <zfw2/game.h>

#include <iostream>
#include <array>
#include <glad/glad.h>
#include <zfw2_common/assets.h>

namespace zfw2
{

struct GameCleanupInfo
{
    bool glfwInitialized;
    GLFWwindow *glfwWindow;
    ALCdevice *alDevice;
    ALCcontext *alContext;
    const Assets *assets;
    const InternalShaderProgs *internalShaderProgs;
    Renderer *renderer;
    const SoundSrcCollection *soundSrcs;
    const MusicSrcCollection *musicSrcCollection;
};

static void clean_game(GameCleanupInfo &cleanupInfo)
{
    if (cleanupInfo.musicSrcCollection)
    {
        clean_music_srcs(*cleanupInfo.musicSrcCollection);
    }

    if (cleanupInfo.soundSrcs)
    {
        clean_sound_srcs(*cleanupInfo.soundSrcs);
    }

    if (cleanupInfo.renderer)
    {
        clean_renderer(*cleanupInfo.renderer);
    }

    if (cleanupInfo.internalShaderProgs)
    {
        clean_internal_shader_progs(*cleanupInfo.internalShaderProgs);
    }

    if (cleanupInfo.assets)
    {
        clean_assets(*cleanupInfo.assets);
    }

    if (cleanupInfo.alContext)
    {
        alcMakeContextCurrent(nullptr);
        alcDestroyContext(cleanupInfo.alContext);
    }

    if (cleanupInfo.alDevice)
    {
        alcCloseDevice(cleanupInfo.alDevice);
    }

    if (cleanupInfo.glfwWindow)
    {
        glfwDestroyWindow(cleanupInfo.glfwWindow);
    }

    if (cleanupInfo.glfwInitialized)
    {
        glfwTerminate();
    }
}

static inline double calc_valid_frame_dur(const double frameTime, const double frameTimeLast)
{
    const double dur = frameTime - frameTimeLast;
    return dur >= 0.0 && dur <= gk_targTickDur * 8.0 ? dur : 0.0;
}

static inline zfw2_common::Vec2DInt get_glfw_window_size(GLFWwindow *const window)
{
    int width, height;
    glfwGetWindowSize(window, &width, &height);
    return {width, height};
}

static inline void glfw_scroll_callback(GLFWwindow *const window, const double xOffs, const double yOffs)
{
    int *const scroll = static_cast<int *>(glfwGetWindowUserPointer(window));
    *scroll = static_cast<int>(yOffs);
}

void run_game()
{
    GameCleanupInfo cleanupInfo = {};

    //
    // Initialisation
    //

    // Initialise GLFW.
    if (!glfwInit())
    {
        return;
    }

    cleanupInfo.glfwInitialized = true;

    // Create the GLFW window.
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, gk_glVersionMajor);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, gk_glVersionMinor);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    glfwWindowHint(GLFW_VISIBLE, false); // Show the window later once other systems have been set up.

    GLFWwindow *const glfwWindow = glfwCreateWindow(1280, 720, "Untitled", nullptr, nullptr);

    if (!glfwWindow)
    {
        clean_game(cleanupInfo);
        return;
    }

    cleanupInfo.glfwWindow = glfwWindow;

    glfwMakeContextCurrent(glfwWindow);

    // Set GLFW window callbacks.
    glfwSetScrollCallback(glfwWindow, glfw_scroll_callback);

    // Hide the cursor.
    glfwSetInputMode(glfwWindow, GLFW_CURSOR, GLFW_CURSOR_HIDDEN);

    // Initialise OpenGL function pointers.
    if (!gladLoadGLLoader(reinterpret_cast<GLADloadproc>(glfwGetProcAddress)))
    {
        clean_game(cleanupInfo);
        return;
    }

    // Open a playback device for OpenAL.
    ALCdevice *const alDevice = alcOpenDevice(nullptr);

    if (!alDevice)
    {
        clean_game(cleanupInfo);
        return;
    }

    cleanupInfo.alDevice = alDevice;

    // Create an OpenAL context.
    ALCcontext *const alContext = alcCreateContext(alDevice, nullptr);

    if (!alContext)
    {
        clean_game(cleanupInfo);
        return;
    }

    cleanupInfo.alContext = alContext;

    alcMakeContextCurrent(alContext);

    // Load assets.
    bool assetsLoadErr;
    const Assets assets = load_assets(assetsLoadErr);

    if (assetsLoadErr)
    {
        clean_game(cleanupInfo);
        return;
    }

    cleanupInfo.assets = &assets;

    const InternalShaderProgs internalShaderProgs = load_internal_shader_progs();
    cleanupInfo.internalShaderProgs = &internalShaderProgs;

    // Set up rendering.
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    Camera cam = {
        .pos = {},
        .scale = 2.0f
    };

    constexpr int renderLayerCnt = 2;
    const RenderLayerCreateInfo renderLayerCreateInfos[renderLayerCnt] = {
        {1, 1024, 1},
        {1, 1024, 1}
    };

    Renderer renderer = make_renderer(renderLayerCnt, 1, renderLayerCreateInfos);
    cleanupInfo.renderer = &renderer;

    // Set up input.
    InputManager inputManager;

    int glfwCallbackMouseScroll = 0; // This is an axis representing the scroll wheel movement. It is updated by the GLFW scroll callback and gets reset after a new input state is generated.
    glfwSetWindowUserPointer(glfwWindow, &glfwCallbackMouseScroll);

    // Set up audio.
    SoundSrcCollection soundSrcs = {};
    cleanupInfo.soundSrcs = &soundSrcs;

    MusicSrcCollection musicSrcCollection = {};
    cleanupInfo.musicSrcCollection = &musicSrcCollection;

    // Show the window now that things have been set up.
    glfwShowWindow(glfwWindow);

    //
    // Main Loop
    //
    double frameTime = glfwGetTime();
    double frameDurAccum = 0.0;

    while (!glfwWindowShouldClose(glfwWindow))
    {
        const zfw2_common::Vec2DInt windowSizeBeforePoll = get_glfw_window_size(glfwWindow);

        glfwPollEvents();

        const zfw2_common::Vec2DInt windowSize = get_glfw_window_size(glfwWindow);

        if (windowSize != windowSizeBeforePoll)
        {
            glViewport(0, 0, windowSize.x, windowSize.y);
        }

        const double frameTimeLast = frameTime;
        frameTime = glfwGetTime();

        const double frameDur = calc_valid_frame_dur(frameTime, frameTimeLast);
        frameDurAccum += frameDur;

        const int tickCnt = frameDurAccum / gk_targTickDur;

        if (tickCnt > 0)
        {
            // Update input.
            inputManager.refresh(glfwWindow, glfwCallbackMouseScroll);
            glfwCallbackMouseScroll = 0;

            // Update audio.
            handle_auto_release_sound_srcs(soundSrcs);
            refresh_music_srcs(musicSrcCollection, assets);

            // Execute ticks.
            int i = 0;

            do
            {
                frameDurAccum -= gk_targTickDur;
                ++i;
            }
            while (i < tickCnt);
        }

        render(renderer, Colors::make_black(), assets, internalShaderProgs, cam, windowSize);
        glfwSwapBuffers(glfwWindow);
    }

    clean_game(cleanupInfo);
}

}
