#pragma once

#include <zfw2/scenes.h>

class TestScene : public zfw2::Scene
{
public:
    TestScene(const zfw2::Assets &assets, const zfw2_common::Vec2DInt windowSize);
    virtual void on_tick(const zfw2::InputManager &inputManager, const zfw2::Assets &assets) override;

private:
    zfw2::SpriteBatchSlotKey m_playerSpriteBatchSlotKey = {};
    zfw2_common::Vec2D m_playerPos = {};

    zfw2::CharBatchKey m_titleCharBatchKey = {};
};
