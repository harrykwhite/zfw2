#pragma once

#include "graphics.h"
#include "input.h"

namespace zfw2
{

class Scene
{
public:
    virtual void on_tick(const InputManager &inputManager, const Assets &assets) = 0;

    inline void draw(const InternalShaderProgs &internalShaderProgs, const Assets &assets, const zfw2_common::Vec2DInt windowSize) const
    {
        m_renderer.draw(internalShaderProgs, assets, windowSize);
    }

protected:
    Renderer m_renderer;
};

}
