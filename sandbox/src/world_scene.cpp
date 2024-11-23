#include "world_scene.h"

WorldScene::WorldScene(const zfw2::Assets &assets, const zfw2_common::Vec2DInt windowSize) : Scene(assets, windowSize, zfw2::Color::create_green())
{
    m_renderer.add_layer("Enemies", 128);
    m_renderer.add_layer("Player", 8);
    m_renderer.lock_layers();

    m_playerSpriteBatchSlotKey = m_renderer.take_any_sprite_batch_slot("Player", 0);

    m_playerPos = {windowSize.x / 2.0f, windowSize.y / 2.0f};
}

void WorldScene::on_tick(const zfw2::InputManager &inputManager, zfw2::SoundManager &soundManager, const zfw2::Assets &assets, zfw2::SceneFactory &sceneChangeFactory)
{
    m_renderer.write_to_sprite_batch_slot(m_playerSpriteBatchSlotKey, assets, m_playerPos, {0, 0, 16, 16}, {0.5f, 0.5f});
}
