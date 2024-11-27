#pragma once

#include <memory>
#include <functional>
#include "graphics.h"
#include "input.h"
#include "assets.h"
#include "audio.h"

namespace zfw2
{

#if 0
class Scene;

using SceneFactory = std::function<std::unique_ptr<Scene>(const Assets &assets, const zfw2_common::Vec2DInt windowSize)>;

class Scene
{
public:
    Scene(const Assets &assets, const zfw2_common::Vec2DInt windowSize, MemArena &memArena, const int renderLayerCnt, const int camRenderLayerCnt, RenderLayerCreateInfo *renderLayerCreateInfos, const Color bgColor = Color::create_black()) : m_bgColor(bgColor), m_renderer(create_renderer(renderLayerCnt, camRenderLayerCnt, renderLayerCreateInfos, memArena))
    {
    }

    virtual void on_tick(const InputManager &inputManager, zfw2::SoundSrcCollection &soundSrcs, zfw2::MusicSrcCollection &musicSrcCollection, const Assets &assets, zfw2::SceneFactory &sceneChangeFactory) = 0;

    virtual void on_window_resize(const zfw2_common::Vec2DInt windowSize)
    {
    }

    inline void draw(const InternalShaderProgs &internalShaderProgs, const Assets &assets, const zfw2_common::Vec2DInt windowSize) const
    {
        render(m_renderer, assets, internalShaderProgs, m_cam, windowSize);
    }

protected:
    const Color m_bgColor;
    Renderer m_renderer;
    Camera m_cam;
};

template<typename T>
inline SceneFactory create_scene_factory()
{
    return [](const Assets &assets, const zfw2_common::Vec2DInt windowSize)
    {
        return std::make_unique<T>(assets, windowSize);
    };
}
#endif

}
