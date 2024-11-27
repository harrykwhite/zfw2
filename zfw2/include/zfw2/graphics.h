#pragma once

#include <vector>
#include <string>
#include <memory>
#include <unordered_map>
#include <functional>
#include <zfw2_common/math.h>
#include "assets.h"
#include "utils.h"
#include "memory.h"

namespace zfw2
{

constexpr int gk_texUnitLimitCap = 32;

constexpr int gk_renderLayerLimit = 16;
constexpr int gk_renderLayerSpriteBatchLimit = 10;
constexpr int gk_renderLayerCharBatchLimit = 24;
constexpr int gk_spriteBatchSlotLimit = 1024;
constexpr int gk_charBatchSlotLimit = 1024;

enum class FontAlignHor
{
    Left,
    Center,
    Right
};

enum class FontAlignVer
{
    Top,
    Center,
    Bottom
};

struct Color
{
    float r, g, b, a;
};

struct QuadBufGLIDs
{
    GLID vertArrayGLID;
    GLID vertBufGLID;
    GLID elemBufGLID;
};

struct SpriteBatchTexUnitInfo
{
    int texIndex;
    int refCnt;
};

struct SpriteBatchSlotKey
{
    int layerIndex;
    int batchIndex;
    int slotIndex;
};

struct SpriteBatchSlotWriteData
{
    const zfw2_common::Vec2D pos;
    const zfw2_common::Rect srcRect;
    const zfw2_common::Vec2D origin;
    const float rot;
    const zfw2_common::Vec2D scale;
    const float alpha;
};

struct CharBatchDisplayProps
{
    zfw2_common::Vec2D pos;
    float rot;
    Color blend;
};

struct CharBatchKey
{
    int layerIndex;
    int batchIndex;
};

struct SpriteBatches
{
    QuadBufGLIDs *quadBufGLIDs;

    int *slotTexUnits;
    SpriteBatchTexUnitInfo *texUnits;

    HeapBitset slotActivity[gk_renderLayerSpriteBatchLimit];
};

struct CharBatches
{
    int *slotCnts;
    QuadBufGLIDs *quadBufGLIDs;
    int *fontIndexes;
    CharBatchDisplayProps *displayProps;
};

inline int get_tex_unit_limit();

struct RenderLayer
{
    int spriteBatchCnt;
    int spriteBatchSlotCnt; // All sprite batches in the same layer have the same slot count.

    int charBatchCnt;

    SpriteBatches spriteBatches;
    CharBatches charBatches;

    HeapBitset spriteBatchActivity;
    HeapBitset charBatchActivity;

    inline int get_sprite_batch_slot_tex_unit(const int batchIndex, const int slotIndex) const
    {
        return spriteBatches.slotTexUnits[(batchIndex * spriteBatchSlotCnt) + slotIndex];
    }

    inline SpriteBatchTexUnitInfo get_sprite_batch_tex_unit_info(const int batchIndex, const int texUnit) const
    {
        return spriteBatches.texUnits[(batchIndex * get_tex_unit_limit()) + texUnit];
    }
};

struct RenderLayerCreateInfo
{
    int spriteBatchCnt;
    int spriteBatchSlotCnt;
    int charBatchCnt;
};

struct Renderer
{
    int layerCnt;
    int camLayerCnt;

    RenderLayer layers[gk_renderLayerLimit];
};

struct Camera
{
    zfw2_common::Vec2D pos;
    float scale;
};

Renderer make_renderer(const int layerCnt, const int camLayerCnt, const RenderLayerCreateInfo *const layerCreateInfos);
void clean_renderer(Renderer &renderer);
void render(const Renderer &renderer, const Color &bgColor, const Assets &assets, const InternalShaderProgs &internalShaderProgs, const Camera &cam, const zfw2_common::Vec2DInt windowSize);

RenderLayer make_render_layer(const RenderLayerCreateInfo &createInfo);
void clean_render_layer(RenderLayer &layer);

SpriteBatchSlotKey take_any_sprite_batch_slot(const int layerIndex, const int texIndex, Renderer &renderer);
void release_sprite_batch_slot(const SpriteBatchSlotKey &key, Renderer &renderer);
void write_to_sprite_batch_slot(const SpriteBatchSlotKey &key, const SpriteBatchSlotWriteData &writeData, const Renderer &renderer, const Assets &assets);
void clear_sprite_batch_slot(const SpriteBatchSlotKey &key, const Renderer &renderer);

CharBatchKey activate_any_char_batch_in_render_layer(const int layerIndex, const int slotCnt, const int fontIndex, const zfw2_common::Vec2D pos, Renderer &renderer, const Assets &assets);
void deactivate_char_batch(const CharBatchKey &key, Renderer &renderer);
void write_to_char_batch(const CharBatchKey &key, const std::string &text, const FontAlignHor alignHor, const FontAlignVer alignVer, Renderer &renderer, const Assets &assets);
void clear_char_batch(const CharBatchKey &key, const Renderer &renderer);

QuadBufGLIDs make_quad_buf(const int quadCnt, const bool isSprite);
void clean_quad_buf(QuadBufGLIDs &glIDs);

namespace Colors
{

constexpr Color make_white() { return {1.0f, 1.0f, 1.0f, 1.0f}; }
constexpr Color make_black() { return {0.0f, 0.0f, 0.0f, 1.0f}; }
constexpr Color make_red() { return {1.0f, 0.0f, 0.0f, 1.0f}; }
constexpr Color make_green() { return {0.0f, 1.0f, 0.0f, 1.0f}; }
constexpr Color make_blue() { return {0.0f, 0.0f, 1.0f, 1.0f}; }
constexpr Color make_yellow() { return {1.0f, 1.0f, 0.0f, 1.0f}; }
constexpr Color make_cyan() { return {0.0f, 1.0f, 1.0f, 1.0f}; }
constexpr Color make_magenta() { return {1.0f, 0.0f, 1.0f, 1.0f}; }

}

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
