#pragma once

#include <vector>
#include <string>
#include <memory>
#include <unordered_map>
#include <zfw2_common/math.h>
#include "assets.h"
#include "utils.h"

namespace zfw2
{

struct Color
{
    float r, g, b, a;

    static constexpr Color create_white() { return {1.0f, 1.0f, 1.0f, 1.0f}; }
    static constexpr Color create_black() { return {0.0f, 0.0f, 0.0f, 1.0f}; }
    static constexpr Color create_red() { return {1.0f, 0.0f, 0.0f, 1.0f}; }
    static constexpr Color create_green() { return {0.0f, 1.0f, 0.0f, 1.0f}; }
    static constexpr Color create_blue() { return {0.0f, 0.0f, 1.0f, 1.0f}; }
    static constexpr Color create_yellow() { return {1.0f, 1.0f, 0.0f, 1.0f}; }
    static constexpr Color create_cyan() { return {0.0f, 1.0f, 1.0f, 1.0f}; }
    static constexpr Color create_magenta() { return {1.0f, 0.0f, 1.0f, 1.0f}; }
};

struct Camera
{
    zfw2_common::Vec2D pos;
    float scale;
};

struct SpriteBatchSlotKey
{
    int layerIndex;
    int batchIndex;
    int slotIndex;
};

constexpr int gk_texUnitLimit = 32;

class SpriteBatch
{
public:
    SpriteBatch(const int slotCnt);
    ~SpriteBatch();
    SpriteBatch(const SpriteBatch &other) = delete;
    SpriteBatch &operator=(const SpriteBatch &other) = delete;
    SpriteBatch(SpriteBatch &&other) noexcept;
    SpriteBatch &operator=(SpriteBatch &&other) noexcept;

    // Provide nullptr as the cam argument if not wanting to draw with a camera view matrix.
    void draw(const InternalShaderProgs &internalShaderProgs, const Assets &assets, const zfw2_common::Vec2DInt windowSize) const;

    int take_any_slot(const int texIndex);
    void release_slot(const int slotIndex);
    void write_to_slot(const int slotIndex, const Assets &assets, const zfw2_common::Vec2D pos, const zfw2_common::Rect &src_rect, const zfw2_common::Vec2D origin, const float rot, const zfw2_common::Vec2D scale, const float alpha) const;
    void clear_slot(const int slotIndex) const;

private:
    GLID m_vertArrayGLID = 0;
    GLID m_vertBufGLID = 0;
    GLID m_elemBufGLID = 0;

    int m_slotCnt;
    HeapBitset m_slotActivity;
    std::unique_ptr<int[]> m_slotTexUnits; // What texture unit each slot is mapped to.

    int m_texUnitTexIndexes[gk_texUnitLimit] = {}; // What texture index (of an actual texture asset) each unit maps to.
    int m_texUnitRefCnts[gk_texUnitLimit] = {}; // How many slots are mapped to each unit.

    int find_tex_unit_to_use(const int texIndex) const;
};

class RenderLayer
{
public:
    RenderLayer(const int defaultSpriteBatchSlotCnt) : m_defaultSpriteBatchSlotCnt(defaultSpriteBatchSlotCnt) {}

    void draw(const InternalShaderProgs &internalShaderProgs, const Assets &assets, const zfw2_common::Vec2DInt windowSize) const;
    void take_any_sprite_batch_slot(const int texIndex, int &batchIndex, int &slotIndex);

    inline void release_sprite_batch_slot(const int slotIndex)
    {
        m_spriteBatches[slotIndex].release_slot(slotIndex);
    }

    inline void write_to_sprite_batch_slot(const int slotIndex, const Assets &assets, const zfw2_common::Vec2D pos, const zfw2_common::Rect &srcRect, const zfw2_common::Vec2D origin, const float rot, const zfw2_common::Vec2D scale, const float alpha) const
    {
        m_spriteBatches[slotIndex].write_to_slot(slotIndex, assets, pos, srcRect, origin, rot, scale, alpha);
    }

    inline void clear_sprite_batch_slot(const int slotIndex) const
    {
        m_spriteBatches[slotIndex].clear_slot(slotIndex);
    }

private:
    const int m_defaultSpriteBatchSlotCnt;
    std::vector<SpriteBatch> m_spriteBatches;
};

class Renderer
{
public:
    void draw(const InternalShaderProgs &internalShaderProgs, const Assets &assets, const zfw2_common::Vec2DInt windowSize) const;
    SpriteBatchSlotKey take_any_sprite_batch_slot(const std::string &layerName, const int texIndex);

    inline void add_layer(const std::string &name, const int defaultSpriteBatchSlotCnt)
    {
        assert(!m_layersLocked);
        assert(m_layerNamesToIndexes.find(name) == m_layerNamesToIndexes.end());
        assert(defaultSpriteBatchSlotCnt > 0 && defaultSpriteBatchSlotCnt % 8 == 0);
        m_layerNamesToIndexes[name] = m_layers.size();
        m_layers.emplace_back(defaultSpriteBatchSlotCnt);
    }

    inline void lock_layers()
    {
        assert(!m_layersLocked);
        m_layersLocked = true;
    }

    inline void release_sprite_batch_slot(const SpriteBatchSlotKey &key)
    {
        assert(m_layersLocked);
        m_layers[key.layerIndex].release_sprite_batch_slot(key.batchIndex);
    }

    inline void write_to_sprite_batch_slot(const SpriteBatchSlotKey &key, const Assets &assets, const zfw2_common::Vec2D pos, const zfw2_common::Rect &srcRect, const zfw2_common::Vec2D origin = {}, const float rot = 0.0f, const zfw2_common::Vec2D scale = {1.0f, 1.0f}, const float alpha = 1.0f) const
    {
        assert(m_layersLocked);
        m_layers[key.layerIndex].write_to_sprite_batch_slot(key.batchIndex, assets, pos, srcRect, origin, rot, scale, alpha);
    }

    inline void clear_sprite_batch_slot(const SpriteBatchSlotKey &key) const
    {
        assert(m_layersLocked);
        m_layers[key.layerIndex].clear_sprite_batch_slot(key.batchIndex);
    }

private:
    std::vector<RenderLayer> m_layers;
    std::unordered_map<std::string, int> m_layerNamesToIndexes;
    bool m_layersLocked = false;
};

inline zfw2_common::Vec2D get_cam_to_screen_pos(const zfw2_common::Vec2D pos, const Camera &cam, const zfw2_common::Vec2DInt windowSize)
{
    return {
        ((pos.x - cam.pos.x) * cam.scale) + (windowSize.x / 2.0f),
        ((pos.y - cam.pos.y) * cam.scale) + (windowSize.y / 2.0f)
    };
}

inline zfw2_common::Vec2D get_screen_to_cam_pos(const zfw2_common::Vec2D pos, const Camera &cam, const zfw2_common::Vec2DInt windowSize)
{
    return {
        ((pos.x - (windowSize.x / 2.0f)) / cam.scale) + cam.pos.x,
        ((pos.y - (windowSize.y / 2.0f)) / cam.scale) + cam.pos.y
    };
}

}
