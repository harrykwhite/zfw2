#pragma once

#include <zfw2/scenes.h>

class MainMenuScene : public zfw2::Scene
{
public:
    MainMenuScene(const zfw2::Assets &assets, const zfw2_common::Vec2DInt windowSize);
    virtual void on_tick(const zfw2::InputManager &inputManager, const zfw2::Assets &assets, zfw2::SceneFactory &sceneChangeFactory) override;

private:
    zfw2::CharBatchKey m_titleCharBatchKey = {};
};
