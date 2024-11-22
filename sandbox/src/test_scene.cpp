#include "test_scene.h"

TestScene::TestScene(const zfw2::Assets &assets, const zfw2_common::Vec2DInt windowSize) : Scene(assets, windowSize)
{
    m_renderer.add_layer("Enemies", 128);
    m_renderer.add_layer("Player", 8);
    m_renderer.add_layer("Title", 8);
    m_renderer.lock_layers();

    m_playerSpriteBatchSlotKey = m_renderer.take_any_sprite_batch_slot("Player", 0);

    m_titleCharBatchKey = m_renderer.add_char_batch("Title", 32, 2, {windowSize.x / 2.0f, windowSize.y / 2.0f});
    m_renderer.write_to_char_batch(m_titleCharBatchKey, "Sandbox", zfw2::FontAlignHor::Center, zfw2::FontAlignVer::Center, assets);
}

void TestScene::on_tick(const zfw2::InputManager &inputManager, const zfw2::Assets &assets)
{
    m_renderer.write_to_sprite_batch_slot(m_playerSpriteBatchSlotKey, assets, m_playerPos, {0, 0, 16, 16}, {0.5f, 0.5f});

    const float titleCharBatchRot = m_renderer.get_char_batch_rot(m_titleCharBatchKey);
    m_renderer.set_char_batch_rot(m_titleCharBatchKey, titleCharBatchRot + ((2.0f * zfw2_common::gk_pi) / 80.0f));

    m_renderer.set_char_batch_blend(m_titleCharBatchKey, zfw2::Color::create_yellow());
}
