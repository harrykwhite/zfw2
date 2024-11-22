#include "test_scene.h"

TestScene::TestScene()
{
    m_renderer.add_layer("Enemies", 128);
    m_renderer.add_layer("Player", 8);
    m_renderer.lock_layers();

    m_playerSpriteBatchSlotKey = m_renderer.take_any_sprite_batch_slot("Player", 0);
}

void TestScene::on_tick(const zfw2::InputManager &inputManager, const zfw2::Assets &assets)
{
    m_renderer.write_to_sprite_batch_slot(m_playerSpriteBatchSlotKey, assets, m_playerPos, {0, 0, 16, 16}, {0.5f, 0.5f});
}
