#pragma once

#include <zfw2/scenes.h>

class WorldScene : public zfw2::Scene
{
public:
    WorldScene(const zfw2::Assets &assets, const zfw2_common::Vec2DInt windowSize);
    virtual void on_tick(const zfw2::InputManager &inputManager, zfw2::SoundManager &soundManager, const zfw2::Assets &assets, zfw2::SceneFactory &sceneChangeFactory) override;

private:
    zfw2::SpriteBatchSlotKey m_playerSpriteBatchSlotKey = {};
    zfw2_common::Vec2D m_playerPos = {};
};
