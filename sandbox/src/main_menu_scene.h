#pragma once

#include <zfw2/scenes.h>

class MainMenuScene : public zfw2::Scene
{
public:
    MainMenuScene(const zfw2::Assets &assets, const zfw2_common::Vec2DInt windowSize);

    virtual void on_tick(const zfw2::InputManager &inputManager, zfw2::SoundSrcCollection &soundSrcs, zfw2::MusicSrcCollection &musicSrcCollection, const zfw2::Assets &assets, zfw2::SceneFactory &sceneChangeFactory) override;
    virtual void on_window_resize(const zfw2_common::Vec2DInt windowSize) override;

private:
    zfw2::CharBatchKey m_titleTextCharBatchKey = {};
    zfw2::CharBatchKey m_startTextCharBatchKey = {};

    inline zfw2_common::Vec2D get_title_text_pos(const zfw2_common::Vec2DInt windowSize)
    {
        return {windowSize.x / 2.0f, (windowSize.y * 7.0f) / 16.0f};
    }

    inline zfw2_common::Vec2D get_start_text_pos(const zfw2_common::Vec2DInt windowSize)
    {
        return {windowSize.x / 2.0f, (windowSize.y * 9.0f) / 16.0f};
    }
};
