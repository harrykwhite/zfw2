#include "main_menu_scene.h"

#include "world_scene.h"

MainMenuScene::MainMenuScene(const zfw2::Assets &assets, const zfw2_common::Vec2DInt windowSize) : Scene(assets, windowSize)
{
    m_renderer.add_layer("All", 8);
    m_renderer.lock_layers();

    m_titleTextCharBatchKey = m_renderer.add_char_batch("All", 8, 2, get_title_text_pos(windowSize));
    m_renderer.write_to_char_batch(m_titleTextCharBatchKey, "Sandbox", zfw2::FontAlignHor::Center, zfw2::FontAlignVer::Center, assets);

    m_startTextCharBatchKey = m_renderer.add_char_batch("All", 32, 1, get_start_text_pos(windowSize));
    m_renderer.write_to_char_batch(m_startTextCharBatchKey, "Press Enter to Start", zfw2::FontAlignHor::Center, zfw2::FontAlignVer::Center, assets);
}

void MainMenuScene::on_tick(const zfw2::InputManager &inputManager, zfw2::SoundSrcCollection &soundSrcs, zfw2::MusicSrcCollection &musicSrcCollection, const zfw2::Assets &assets, zfw2::SceneFactory &sceneChangeFactory)
{
    if (inputManager.is_key_pressed(zfw2::KeyCode::Enter))
    {
        sceneChangeFactory = zfw2::create_scene_factory<WorldScene>();
    }
}

void MainMenuScene::on_window_resize(const zfw2_common::Vec2DInt windowSize)
{
    m_renderer.set_char_batch_pos(m_titleTextCharBatchKey, get_title_text_pos(windowSize));
    m_renderer.set_char_batch_pos(m_startTextCharBatchKey, get_start_text_pos(windowSize));
}
