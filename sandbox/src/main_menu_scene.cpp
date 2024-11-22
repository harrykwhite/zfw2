#include "main_menu_scene.h"

#include "world_scene.h"

MainMenuScene::MainMenuScene(const zfw2::Assets &assets, const zfw2_common::Vec2DInt windowSize) : Scene(assets, windowSize)
{
    m_renderer.add_layer("All", 8);
    m_renderer.lock_layers();

    m_titleCharBatchKey = m_renderer.add_char_batch("All", 8, 2, {windowSize.x / 2.0f, windowSize.y / 2.0f});
    m_renderer.write_to_char_batch(m_titleCharBatchKey, "Sandbox", zfw2::FontAlignHor::Center, zfw2::FontAlignVer::Center, assets);
}

void MainMenuScene::on_tick(const zfw2::InputManager &inputManager, const zfw2::Assets &assets, zfw2::SceneFactory &sceneChangeFactory)
{
    if (inputManager.is_key_down(zfw2::KeyCode::Enter))
    {
        sceneChangeFactory = zfw2::create_scene_factory<WorldScene>();
    }
}
