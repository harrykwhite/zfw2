#include <zfw2/graphics.h>

#include <numeric>

namespace zfw2
{

static int find_sprite_batch_tex_unit_to_use(const RenderLayer &renderLayer, const int batchIndex, const int texIndex)
{
    int freeTexUnit = -1;

    const int texUnitLimit = get_tex_unit_limit();

    for (int i = 0; i < texUnitLimit; ++i)
    {
        const SpriteBatchTexUnitInfo &texUnit = renderLayer.spriteBatches.texUnits[(batchIndex * texUnitLimit) + i];

        if (!texUnit.refCnt)
        {
            if (freeTexUnit == -1)
            {
                freeTexUnit = i;
            }

            continue;
        }

        if (texUnit.texIndex == texIndex)
        {
            return i;
        }
    }

    return freeTexUnit;
}

static void activate_any_sprite_batch_in_render_layer(const int layerIndex, Renderer &renderer)
{
    assert(layerIndex >= 0 && layerIndex < renderer.layerCnt);

    RenderLayer &layer = renderer.layers[layerIndex];

    const int batchIndex = find_first_inactive_bit_in_heap_bitset(layer.spriteBatchActivity);
    assert(batchIndex != -1);

    activate_heap_bitset_bit(layer.spriteBatchActivity, batchIndex);

    layer.spriteBatches.quadBufGLIDs[batchIndex] = make_quad_buf(layer.spriteBatchSlotCnt, true);
    std::memset(layer.spriteBatches.slotTexUnits + (batchIndex * layer.spriteBatchSlotCnt), 0, layer.spriteBatchSlotCnt * sizeof(int));
    std::memset(layer.spriteBatches.texUnits + (batchIndex * get_tex_unit_limit()), 0, get_tex_unit_limit() * sizeof(SpriteBatchTexUnitInfo));
    clear_heap_bitset(layer.spriteBatches.slotActivity[batchIndex]);
}

static inline zfw2_common::Matrix4x4 create_cam_view_mat(const Camera &cam, const zfw2_common::Vec2DInt windowSize)
{
    assert(cam.scale >= 1.0f);

    zfw2_common::Matrix4x4 mat = {};
    mat[0][0] = cam.scale;
    mat[1][1] = cam.scale;
    mat[3][3] = 1.0f;
    mat[3][0] = -(cam.pos.x - (windowSize.x / 2.0f));
    mat[3][1] = -(cam.pos.y - (windowSize.y / 2.0f));

    return mat;
}

inline int get_tex_unit_limit()
{
    int limit;
    glGetIntegerv(GL_MAX_TEXTURE_IMAGE_UNITS, &limit);
    return std::min(limit, gk_texUnitLimitCap);
}

Renderer make_renderer(const int layerCnt, const int camLayerCnt, const RenderLayerCreateInfo *const layerCreateInfos)
{
    assert(layerCnt > 0 && layerCnt <= gk_renderLayerLimit);
    assert(camLayerCnt >= 0 && camLayerCnt <= layerCnt);
    assert(layerCreateInfos);

    Renderer renderer = {
        .layerCnt = layerCnt,
        .camLayerCnt = camLayerCnt
    };

    for (int i = 0; i < layerCnt; ++i)
    {
        renderer.layers[i] = make_render_layer(layerCreateInfos[i]);
    }

    return renderer;
}

void clean_renderer(Renderer &renderer)
{
    for (int i = 0; i < renderer.layerCnt; ++i)
    {
        clean_render_layer(renderer.layers[i]);
    }

    renderer = {};
}

void render(const Renderer &renderer, const Color &bgColor, const Assets &assets, const InternalShaderProgs &internalShaderProgs, const Camera &cam, const zfw2_common::Vec2DInt windowSize)
{
    glClearColor(bgColor.r, bgColor.g, bgColor.b, bgColor.a);
    glClear(GL_COLOR_BUFFER_BIT);

    const int texUnitLimit = get_tex_unit_limit();

    int texUnits[gk_texUnitLimitCap];
    std::iota(texUnits, texUnits + texUnitLimit, 0);

    // Create the projection and view matrices.
    const auto projMat = zfw2_common::Matrix4x4::create_ortho(0.0f, windowSize.x, windowSize.y, 0.0f, -1.0f, 1.0f);

    // Define function for rendering a layer.
    auto renderLayer = [&](const RenderLayer &layer, const zfw2_common::Matrix4x4 &viewMat)
    {
        // Render sprite batches.
        glUseProgram(internalShaderProgs.spriteQuadProgGLID);

        glUniformMatrix4fv(glGetUniformLocation(internalShaderProgs.spriteQuadProgGLID, "u_proj"), 1, false, reinterpret_cast<const float *>(projMat.elems));
        glUniformMatrix4fv(glGetUniformLocation(internalShaderProgs.spriteQuadProgGLID, "u_view"), 1, false, reinterpret_cast<const float *>(viewMat.elems));

        glUniform1iv(glGetUniformLocation(internalShaderProgs.spriteQuadProgGLID, "u_textures"), texUnitLimit, texUnits);

        for (int i = 0; i < layer.spriteBatchCnt; ++i)
        {
            if (!is_heap_bitset_bit_active(layer.spriteBatchActivity, i))
            {
                continue;
            }

            // Bind texture GLIDs to units.
            for (int j = 0; j < texUnitLimit; ++j)
            {
                const SpriteBatchTexUnitInfo &texUnit = layer.spriteBatches.texUnits[(i * texUnitLimit) + j];
                glActiveTexture(GL_TEXTURE0 + j);
                glBindTexture(GL_TEXTURE_2D, texUnit.refCnt > 0 ? assets.texGLIDs[texUnit.texIndex] : 0);
            }

            // Draw the batch.
            glBindVertexArray(layer.spriteBatches.quadBufGLIDs[i].vertArrayGLID);
            glDrawElements(GL_TRIANGLES, 6 * layer.spriteBatchSlotCnt, GL_UNSIGNED_SHORT, nullptr);
        }

        // Render character batches.
        glUseProgram(internalShaderProgs.charQuadProgGLID);

        glUniformMatrix4fv(glGetUniformLocation(internalShaderProgs.charQuadProgGLID, "u_proj"), 1, false, reinterpret_cast<const float *>(projMat.elems));
        glUniformMatrix4fv(glGetUniformLocation(internalShaderProgs.charQuadProgGLID, "u_view"), 1, false, reinterpret_cast<const float *>(viewMat.elems));

        for (int i = 0; i < layer.charBatchCnt; ++i)
        {
            if (!is_heap_bitset_bit_active(layer.charBatchActivity, i))
            {
                continue;
            }

            const CharBatchDisplayProps &displayProps = layer.charBatches.displayProps[i];

            glUniform2fv(glGetUniformLocation(internalShaderProgs.charQuadProgGLID, "u_pos"), 1, reinterpret_cast<const float *>(&displayProps.pos));
            glUniform1f(glGetUniformLocation(internalShaderProgs.charQuadProgGLID, "u_rot"), displayProps.rot);
            glUniform4fv(glGetUniformLocation(internalShaderProgs.charQuadProgGLID, "u_blend"), 1, reinterpret_cast<const float *>(&displayProps.blend));

            // Bind font texture GLID.
            glActiveTexture(GL_TEXTURE0);
            glBindTexture(GL_TEXTURE_2D, assets.fontTexGLIDs[layer.charBatches.fontIndexes[i]]);

            // Draw the batch.
            glBindVertexArray(layer.charBatches.quadBufGLIDs[i].vertArrayGLID);
            glDrawElements(GL_TRIANGLES, 6 * layer.charBatches.slotCnts[i], GL_UNSIGNED_SHORT, nullptr);
        }
    };

    // Render camera layers then non-camera ones.
    const zfw2_common::Matrix4x4 camViewMat = create_cam_view_mat(cam, windowSize);

    for (int i = 0; i < renderer.camLayerCnt; ++i)
    {
        renderLayer(renderer.layers[i], camViewMat);
    }

    const zfw2_common::Matrix4x4 defaultViewMat = zfw2_common::Matrix4x4::create_identity();

    for (int i = renderer.camLayerCnt; i < renderer.layerCnt; ++i)
    {
        renderLayer(renderer.layers[i], defaultViewMat);
    }
}

RenderLayer make_render_layer(const RenderLayerCreateInfo &createInfo)
{
    assert(createInfo.spriteBatchCnt > 0 && createInfo.spriteBatchCnt <= gk_renderLayerSpriteBatchLimit);
    assert(createInfo.spriteBatchSlotCnt > 0 && createInfo.spriteBatchSlotCnt <= gk_spriteBatchSlotLimit);
    assert(createInfo.charBatchCnt > 0 && createInfo.charBatchCnt <= gk_renderLayerCharBatchLimit);

    RenderLayer layer = {
        .spriteBatchCnt = createInfo.spriteBatchCnt,
        .spriteBatchSlotCnt = createInfo.spriteBatchSlotCnt,
        .charBatchCnt = createInfo.charBatchCnt
    };

    layer.spriteBatches.quadBufGLIDs = mem_arena_alloc<QuadBufGLIDs>(createInfo.spriteBatchCnt);
    layer.spriteBatches.slotTexUnits = mem_arena_alloc<int>(createInfo.spriteBatchSlotCnt * createInfo.spriteBatchCnt);
    layer.spriteBatches.texUnits = mem_arena_alloc<SpriteBatchTexUnitInfo>(get_tex_unit_limit() * createInfo.spriteBatchCnt);

    for (int i = 0; i < createInfo.spriteBatchCnt; ++i)
    {
        layer.spriteBatches.slotActivity[i] = create_heap_bitset(createInfo.spriteBatchSlotCnt);
    }

    layer.charBatches.slotCnts = mem_arena_alloc<int>(createInfo.charBatchCnt);
    layer.charBatches.quadBufGLIDs = mem_arena_alloc<QuadBufGLIDs>(createInfo.charBatchCnt);
    layer.charBatches.fontIndexes = mem_arena_alloc<int>(createInfo.charBatchCnt);
    layer.charBatches.displayProps = mem_arena_alloc<CharBatchDisplayProps>(createInfo.charBatchCnt);

    layer.spriteBatchActivity = create_heap_bitset(createInfo.spriteBatchCnt);
    layer.charBatchActivity = create_heap_bitset(createInfo.charBatchCnt);

    return layer;
}

void clean_render_layer(RenderLayer &layer)
{
    for (int i = 0; i < layer.spriteBatchCnt; ++i)
    {
        if (!is_heap_bitset_bit_active(layer.spriteBatchActivity, i))
        {
            continue;
        }

        clean_quad_buf(layer.spriteBatches.quadBufGLIDs[i]);
    }

    for (int i = 0; i < layer.charBatchCnt; ++i)
    {
        if (!is_heap_bitset_bit_active(layer.charBatchActivity, i))
        {
            continue;
        }

        clean_quad_buf(layer.charBatches.quadBufGLIDs[i]);
    }

    layer = {};
}

SpriteBatchSlotKey take_any_sprite_batch_slot(const int layerIndex, const int texIndex, Renderer &renderer)
{
    assert(layerIndex >= 0 && layerIndex < renderer.layerCnt);

    RenderLayer &layer = renderer.layers[layerIndex];

    for (int i = 0; i < layer.spriteBatchCnt; ++i)
    {
        if (!is_heap_bitset_bit_active(layer.spriteBatchActivity, i))
        {
            continue;
        }

        // Continue if no slot is available.
        if (is_heap_bitset_full(layer.spriteBatches.slotActivity[i]))
        {
            continue;
        }

        // Find a texture unit to use, continue if none are suitable.
        const int texUnit = find_sprite_batch_tex_unit_to_use(layer, i, texIndex);

        if (texUnit == -1)
        {
            continue;
        }

        // Use the first inactive slot.
        const int slotIndex = find_first_inactive_bit_in_heap_bitset(layer.spriteBatches.slotActivity[i]);
        activate_heap_bitset_bit(layer.spriteBatches.slotActivity[i], slotIndex);

        // Update texture unit information.
        SpriteBatchTexUnitInfo &texUnitInfo = layer.spriteBatches.texUnits[(i * get_tex_unit_limit()) + texUnit];
        texUnitInfo.texIndex = texIndex;
        ++texUnitInfo.refCnt;

        layer.spriteBatches.slotTexUnits[(i * layer.spriteBatchSlotCnt) + slotIndex] = texUnit;

        return {
            .layerIndex = layerIndex,
            .batchIndex = i,
            .slotIndex = slotIndex
        };
    }

    // Failed to find a batch to use in the layer, so activate a new one and try this all again.
    activate_any_sprite_batch_in_render_layer(layerIndex, renderer);
    return take_any_sprite_batch_slot(layerIndex, texIndex, renderer); // TEMP: Way more work is done here than necessary.
}

void release_sprite_batch_slot(const SpriteBatchSlotKey &key, Renderer &renderer)
{
    RenderLayer &layer = renderer.layers[key.layerIndex];

    // Mark the slot as inactive.
    deactivate_heap_bitset_bit(layer.spriteBatches.slotActivity[key.batchIndex], key.slotIndex);

    // Update texture unit information.
    const int texUnit = layer.spriteBatches.slotTexUnits[(key.batchIndex * layer.spriteBatchSlotCnt) + key.slotIndex];
    SpriteBatchTexUnitInfo &texUnitInfo = layer.spriteBatches.texUnits[(key.batchIndex * get_tex_unit_limit()) + texUnit];
    --texUnitInfo.refCnt;

    // Clear the slot render data.
    clear_sprite_batch_slot(key, renderer);
}

void write_to_sprite_batch_slot(const SpriteBatchSlotKey &key, const SpriteBatchSlotWriteData &writeData, const Renderer &renderer, const Assets &assets)
{
    const RenderLayer &layer = renderer.layers[key.layerIndex];

    const int texUnit = layer.get_sprite_batch_slot_tex_unit(key.batchIndex, key.slotIndex);
    const SpriteBatchTexUnitInfo &texUnitInfo = layer.get_sprite_batch_tex_unit_info(key.batchIndex, texUnit);
    const zfw2_common::Vec2DInt texSize = assets.texSizes[texUnitInfo.texIndex];

    const float verts[gk_spriteQuadShaderProgVertCnt * 4] = {
        (0.0f - writeData.origin.x) * writeData.scale.x,
        (0.0f - writeData.origin.y) * writeData.scale.y,
        writeData.pos.x,
        writeData.pos.y,
        static_cast<float>(writeData.srcRect.width),
        static_cast<float>(writeData.srcRect.height),
        writeData.rot,
        static_cast<float>(texUnit),
        static_cast<float>(writeData.srcRect.x) / texSize.x,
        static_cast<float>(writeData.srcRect.y) / texSize.y,
        writeData.alpha,

        (1.0f - writeData.origin.x) * writeData.scale.x,
        (0.0f - writeData.origin.y) * writeData.scale.y,
        writeData.pos.x,
        writeData.pos.y,
        static_cast<float>(writeData.srcRect.width),
        static_cast<float>(writeData.srcRect.height),
        writeData.rot,
        static_cast<float>(texUnit),
        static_cast<float>(writeData.srcRect.x + writeData.srcRect.width) / texSize.x,
        static_cast<float>(writeData.srcRect.y) / texSize.y,
        writeData.alpha,

        (1.0f - writeData.origin.x) * writeData.scale.x,
        (1.0f - writeData.origin.y) * writeData.scale.y,
        writeData.pos.x,
        writeData.pos.y,
        static_cast<float>(writeData.srcRect.width),
        static_cast<float>(writeData.srcRect.height),
        writeData.rot,
        static_cast<float>(texUnit),
        static_cast<float>(writeData.srcRect.x + writeData.srcRect.width) / texSize.x,
        static_cast<float>(writeData.srcRect.y + writeData.srcRect.height) / texSize.y,
        writeData.alpha,

        (0.0f - writeData.origin.x) * writeData.scale.x,
        (1.0f - writeData.origin.y) * writeData.scale.y,
        writeData.pos.x,
        writeData.pos.y,
        static_cast<float>(writeData.srcRect.width),
        static_cast<float>(writeData.srcRect.height),
        writeData.rot,
        static_cast<float>(texUnit),
        static_cast<float>(writeData.srcRect.x) / texSize.x,
        static_cast<float>(writeData.srcRect.y + writeData.srcRect.height) / texSize.y,
        writeData.alpha
    };

    QuadBufGLIDs &quadBufGLIDs = renderer.layers[key.layerIndex].spriteBatches.quadBufGLIDs[key.batchIndex];

    glBindVertexArray(quadBufGLIDs.vertArrayGLID);
    glBindBuffer(GL_ARRAY_BUFFER, quadBufGLIDs.vertBufGLID);
    glBufferSubData(GL_ARRAY_BUFFER, key.slotIndex * sizeof(verts), sizeof(verts), verts);
}

void clear_sprite_batch_slot(const SpriteBatchSlotKey &key, const Renderer &renderer)
{
    QuadBufGLIDs &quadBufGLIDs = renderer.layers[key.layerIndex].spriteBatches.quadBufGLIDs[key.batchIndex];

    glBindVertexArray(quadBufGLIDs.vertArrayGLID);
    glBindBuffer(GL_ARRAY_BUFFER, quadBufGLIDs.vertBufGLID);

    const float verts[gk_spriteQuadShaderProgVertCnt * 4] = {};
    glBufferSubData(GL_ARRAY_BUFFER, key.slotIndex * sizeof(verts), sizeof(verts), verts);
}

CharBatchKey activate_any_char_batch_in_render_layer(const int layerIndex, const int slotCnt, const int fontIndex, const zfw2_common::Vec2D pos, Renderer &renderer, const Assets &assets)
{
    assert(layerIndex >= 0 && layerIndex < renderer.layerCnt);
    assert(slotCnt > 0 && slotCnt <= gk_charBatchSlotLimit);
    assert(fontIndex >= 0 && fontIndex < assets.fontCnt);

    RenderLayer &layer = renderer.layers[layerIndex];

    const int batchIndex = find_first_inactive_bit_in_heap_bitset(layer.charBatchActivity);
    assert(batchIndex != -1);

    activate_heap_bitset_bit(layer.charBatchActivity, batchIndex);

    layer.charBatches.slotCnts[batchIndex] = slotCnt;
    layer.charBatches.quadBufGLIDs[batchIndex] = make_quad_buf(slotCnt, false);
    layer.charBatches.fontIndexes[batchIndex] = fontIndex;
    layer.charBatches.displayProps[batchIndex] = {
        .pos = pos,
        .rot = 0.0f,
        .blend = Colors::make_white()
    };

    return {
        .layerIndex = layerIndex,
        .batchIndex = batchIndex
    };
}

void deactivate_char_batch(const CharBatchKey &key, Renderer &renderer)
{
    RenderLayer &layer = renderer.layers[key.layerIndex];
    deactivate_heap_bitset_bit(layer.charBatchActivity, key.batchIndex);
    clear_char_batch(key, renderer);
}

void write_to_char_batch(const CharBatchKey &key, const std::string &text, const FontAlignHor alignHor, const FontAlignVer alignVer, Renderer &renderer, const Assets &assets)
{
    RenderLayer &layer = renderer.layers[key.layerIndex];
    const int slotCnt = layer.charBatches.slotCnts[key.batchIndex];

    assert(text.length() > 0 && text.length() <= slotCnt);

    const zfw2_common::FontData &fontData = assets.fontDatas[layer.charBatches.fontIndexes[key.batchIndex]];

    // Determine the positions of text characters based on font information, alongside the overall dimensions of the text to be used when applying alignment.
    const auto charDrawPositions = mem_arena_alloc<zfw2_common::Vec2D>(slotCnt);
    zfw2_common::Vec2D charDrawPosPen = {};

    const auto textLineWidths = mem_arena_alloc<int>(slotCnt + 1);
    int textFirstLineMinOffs = 0;
    bool textFirstLineMinOffsUpdated = false;
    int textLastLineMaxHeight = 0;
    bool textLastLineMaxHeightUpdated = false;
    int textLineCnter = 0;

    for (int i = 0; i < text.length(); i++)
    {
        if (text[i] == '\n')
        {
            textLineWidths[textLineCnter] = charDrawPosPen.x;

            if (!textFirstLineMinOffsUpdated)
            {
                // Set the first line minimum offset to the vertical offset of the space character.
                textFirstLineMinOffs = fontData.chars.verOffsets[0];
                textFirstLineMinOffsUpdated = true;
            }

            // Set the last line maximum height to the height of a space.
            textLastLineMaxHeight = fontData.chars.verOffsets[0] + fontData.chars.srcRects[0].height;

            textLastLineMaxHeightUpdated = false;

            textLineCnter++;

            // Move the pen to a new line.
            charDrawPosPen.x = 0.0f;
            charDrawPosPen.y += fontData.lineHeight;

            continue;
        }

        const int textCharIndex = text[i] - zfw2_common::gk_fontCharRangeBegin;

        // If we are on the first line, update the first line minimum offset.
        if (textLineCnter == 0)
        {
            if (!textFirstLineMinOffsUpdated)
            {
                textFirstLineMinOffs = fontData.chars.verOffsets[textCharIndex];
                textFirstLineMinOffsUpdated = true;
            }
            else
            {
                textFirstLineMinOffs = std::min(fontData.chars.verOffsets[textCharIndex], textFirstLineMinOffs);
            }
        }

        if (!textLastLineMaxHeightUpdated)
        {
            textLastLineMaxHeight = fontData.chars.verOffsets[textCharIndex] + fontData.chars.srcRects[textCharIndex].height;
            textLastLineMaxHeightUpdated = true;
        }
        else
        {
            textLastLineMaxHeight = std::max(fontData.chars.verOffsets[textCharIndex] + fontData.chars.srcRects[textCharIndex].height, textLastLineMaxHeight);
        }

        if (i > 0)
        {
            // Apply kerning based on the previous character.
            const int textCharIndexLast = text[i - 1] - zfw2_common::gk_fontCharRangeBegin;
            charDrawPosPen.x += fontData.chars.kernings[(textCharIndex * zfw2_common::gk_fontCharRangeSize) + textCharIndexLast];
        }

        charDrawPositions[i].x = charDrawPosPen.x + fontData.chars.horOffsets[textCharIndex];
        charDrawPositions[i].y = charDrawPosPen.y + fontData.chars.verOffsets[textCharIndex];

        charDrawPosPen.x += fontData.chars.horAdvances[textCharIndex];
    }

    textLineWidths[textLineCnter] = charDrawPosPen.x;
    textLineCnter = 0;

    const int textHeight = textFirstLineMinOffs + charDrawPosPen.y + textLastLineMaxHeight;

    // Clear the batch so it can have only the new characters.
    clear_char_batch(key, renderer);

    // Bind the vertex array and buffer.
    const QuadBufGLIDs &quadBufGLIDs = layer.charBatches.quadBufGLIDs[key.batchIndex];
    glBindVertexArray(quadBufGLIDs.vertArrayGLID);
    glBindBuffer(GL_ARRAY_BUFFER, quadBufGLIDs.vertBufGLID);

    // Write character render data.
    for (int i = 0; i < text.length(); i++)
    {
        if (text[i] == '\n')
        {
            textLineCnter++;
            continue;
        }

        if (text[i] == ' ')
        {
            continue;
        }

        const int charIndex = text[i] - zfw2_common::gk_fontCharRangeBegin;

        const zfw2_common::Vec2D charDrawPos = {
            charDrawPositions[i].x - (textLineWidths[textLineCnter] * (static_cast<int>(alignHor) / 2.0f)),
            charDrawPositions[i].y - (textHeight * (static_cast<int>(alignVer) / 2.0f))
        };

        const zfw2_common::RectFloat charTexCoordsRect = {
            static_cast<float>(fontData.chars.srcRects[charIndex].x) / fontData.texSize.x,
            static_cast<float>(fontData.chars.srcRects[charIndex].y) / fontData.texSize.y,
            static_cast<float>(fontData.chars.srcRects[charIndex].width) / fontData.texSize.x,
            static_cast<float>(fontData.chars.srcRects[charIndex].height) / fontData.texSize.y
        };

        const float verts[gk_charQuadShaderProgVertCnt * 4] = {
            charDrawPos.x,
            charDrawPos.y,
            charTexCoordsRect.x,
            charTexCoordsRect.y,

            charDrawPos.x + fontData.chars.srcRects[charIndex].width,
            charDrawPos.y,
            charTexCoordsRect.x + charTexCoordsRect.width,
            charTexCoordsRect.y,

            charDrawPos.x + fontData.chars.srcRects[charIndex].width,
            charDrawPos.y + fontData.chars.srcRects[charIndex].height,
            charTexCoordsRect.x + charTexCoordsRect.width,
            charTexCoordsRect.y + charTexCoordsRect.height,

            charDrawPos.x,
            charDrawPos.y + fontData.chars.srcRects[charIndex].height,
            charTexCoordsRect.x,
            charTexCoordsRect.y + charTexCoordsRect.height
        };

        glBufferSubData(GL_ARRAY_BUFFER, i * sizeof(verts), sizeof(verts), verts);
    }
}

void clear_char_batch(const CharBatchKey &key, const Renderer &renderer)
{
    const RenderLayer &layer = renderer.layers[key.layerIndex];
    const QuadBufGLIDs &quadBufGLIDs = layer.charBatches.quadBufGLIDs[key.batchIndex];

    glBindVertexArray(quadBufGLIDs.vertArrayGLID);
    glBindBuffer(GL_ARRAY_BUFFER, quadBufGLIDs.vertBufGLID);

    const int vertsLen = (gk_charQuadShaderProgVertCnt * 4) * layer.charBatches.slotCnts[key.batchIndex];
    const auto verts = mem_arena_alloc<const float>(vertsLen);
    glBufferData(GL_ARRAY_BUFFER, sizeof(verts[0]) * vertsLen, verts, GL_DYNAMIC_DRAW);
}

QuadBufGLIDs make_quad_buf(const int quadCnt, const bool isSprite)
{
    assert(quadCnt > 0);

    QuadBufGLIDs glIDs = {};

    const int vertCnt = isSprite ? gk_spriteQuadShaderProgVertCnt : gk_charQuadShaderProgVertCnt;

    // Generate vertex array.
    glGenVertexArrays(1, &glIDs.vertArrayGLID);
    glBindVertexArray(glIDs.vertArrayGLID);

    // Generate vertex buffer.
    glGenBuffers(1, &glIDs.vertBufGLID);
    glBindBuffer(GL_ARRAY_BUFFER, glIDs.vertBufGLID);

    {
        const int vertsLen = vertCnt * 4 * quadCnt;
        const auto verts = mem_arena_alloc<float>(vertsLen);
        glBufferData(GL_ARRAY_BUFFER, sizeof(verts[0]) * vertsLen, verts, GL_DYNAMIC_DRAW);
    }

    // Generate element buffer.
    glGenBuffers(1, &glIDs.elemBufGLID);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, glIDs.elemBufGLID);

    {
        // TODO: Have it so that the indices are only set up once; all batches use a copy of this same data, so recalculation isn't needed.
        const int indicesLen = 6 * quadCnt;
        const auto indices = mem_arena_alloc<unsigned short>(indicesLen);

        for (int i = 0; i < quadCnt; i++)
        {
            indices[(i * 6) + 0] = (i * 4) + 0;
            indices[(i * 6) + 1] = (i * 4) + 1;
            indices[(i * 6) + 2] = (i * 4) + 2;
            indices[(i * 6) + 3] = (i * 4) + 2;
            indices[(i * 6) + 4] = (i * 4) + 3;
            indices[(i * 6) + 5] = (i * 4) + 0;
        }

        glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices[0]) * indicesLen, indices, GL_STATIC_DRAW);
    }

    // Set vertex attribute pointers.
    const int vertsStride = sizeof(float) * vertCnt;

    if (isSprite)
    {
        glVertexAttribPointer(0, 2, GL_FLOAT, false, vertsStride, reinterpret_cast<void *>(sizeof(float) * 0));
        glEnableVertexAttribArray(0);

        glVertexAttribPointer(1, 2, GL_FLOAT, false, vertsStride, reinterpret_cast<void *>(sizeof(float) * 2));
        glEnableVertexAttribArray(1);

        glVertexAttribPointer(2, 2, GL_FLOAT, false, vertsStride, reinterpret_cast<void *>(sizeof(float) * 4));
        glEnableVertexAttribArray(2);

        glVertexAttribPointer(3, 1, GL_FLOAT, false, vertsStride, reinterpret_cast<void *>(sizeof(float) * 6));
        glEnableVertexAttribArray(3);

        glVertexAttribPointer(4, 1, GL_FLOAT, false, vertsStride, reinterpret_cast<void *>(sizeof(float) * 7));
        glEnableVertexAttribArray(4);

        glVertexAttribPointer(5, 2, GL_FLOAT, false, vertsStride, reinterpret_cast<void *>(sizeof(float) * 8));
        glEnableVertexAttribArray(5);

        glVertexAttribPointer(6, 1, GL_FLOAT, false, vertsStride, reinterpret_cast<void *>(sizeof(float) * 10));
        glEnableVertexAttribArray(6);
    }
    else
    {
        glVertexAttribPointer(0, 2, GL_FLOAT, false, vertsStride, reinterpret_cast<void *>(sizeof(float) * 0));
        glEnableVertexAttribArray(0);

        glVertexAttribPointer(1, 2, GL_FLOAT, false, vertsStride, reinterpret_cast<void *>(sizeof(float) * 2));
        glEnableVertexAttribArray(1);
    }

    // Unbind.
    glBindVertexArray(0);

    return glIDs;
}

void clean_quad_buf(QuadBufGLIDs &glIDs)
{
    glDeleteBuffers(1, &glIDs.vertBufGLID);
    glDeleteBuffers(1, &glIDs.elemBufGLID);
    glDeleteVertexArrays(1, &glIDs.vertArrayGLID);

    glIDs = {};
}

}
