#include <zfw2/graphics.h>

#include <numeric>

namespace zfw2
{

SpriteBatch::SpriteBatch(const int slotCnt) : m_slotCnt(slotCnt), m_slotActivity(slotCnt), m_slotTexUnits(std::make_unique<int[]>(slotCnt))
{
    assert(slotCnt % 8 == 0);

    // Generate vertex array.
    glGenVertexArrays(1, &m_vertArrayGLID);
    glBindVertexArray(m_vertArrayGLID);

    // Generate vertex buffer.
    glGenBuffers(1, &m_vertBufGLID);
    glBindBuffer(GL_ARRAY_BUFFER, m_vertBufGLID);

    {
        const int vertsLen = gk_spriteQuadShaderProgVertCnt * 4 * slotCnt;
        const auto verts = std::make_unique<float[]>(vertsLen);
        std::fill(verts.get(), verts.get() + vertsLen, 0); // NOTE: This could be redundant.
        glBufferData(GL_ARRAY_BUFFER, sizeof(verts[0]) * vertsLen, verts.get(), GL_DYNAMIC_DRAW);
    }

    // Generate element buffer.
    glGenBuffers(1, &m_elemBufGLID);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_elemBufGLID);

    {
        // TODO: Have it so that the indices are only set up once; all batches use a copy of this same data, so recalculation isn't needed.
        const int indicesLen = 6 * slotCnt;
        const auto indices = std::make_unique<unsigned short[]>(indicesLen);

        for (int i = 0; i < slotCnt; i++)
        {
            indices[(i * 6) + 0] = (i * 4) + 0;
            indices[(i * 6) + 1] = (i * 4) + 1;
            indices[(i * 6) + 2] = (i * 4) + 2;
            indices[(i * 6) + 3] = (i * 4) + 2;
            indices[(i * 6) + 4] = (i * 4) + 3;
            indices[(i * 6) + 5] = (i * 4) + 0;
        }

        glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices[0]) * indicesLen, indices.get(), GL_STATIC_DRAW);
    }

    // Set vertex attribute pointers.
    const int vertsStride = sizeof(float) * gk_spriteQuadShaderProgVertCnt;

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

    glBindVertexArray(0);
}

SpriteBatch::~SpriteBatch()
{
    glDeleteBuffers(1, &m_elemBufGLID);
    glDeleteBuffers(1, &m_vertBufGLID);
    glDeleteVertexArrays(1, &m_vertArrayGLID);
}

SpriteBatch::SpriteBatch(SpriteBatch &&other) noexcept
    : m_vertArrayGLID(other.m_vertArrayGLID),
    m_vertBufGLID(other.m_vertBufGLID),
    m_elemBufGLID(other.m_elemBufGLID),
    m_slotCnt(other.m_slotCnt),
    m_slotActivity(std::move(other.m_slotActivity)),
    m_slotTexUnits(std::move(other.m_slotTexUnits))
{
    std::copy(std::begin(other.m_texUnitTexIndexes), std::end(other.m_texUnitTexIndexes), m_texUnitTexIndexes);
    std::copy(std::begin(other.m_texUnitRefCnts), std::end(other.m_texUnitRefCnts), m_texUnitRefCnts);

    other.m_vertArrayGLID = 0;
    other.m_vertBufGLID = 0;
    other.m_elemBufGLID = 0;
}

SpriteBatch &SpriteBatch::operator=(SpriteBatch &&other) noexcept
{
    if (this != &other)
    {
        m_vertArrayGLID = other.m_vertArrayGLID;
        m_vertBufGLID = other.m_vertBufGLID;
        m_elemBufGLID = other.m_elemBufGLID;
        m_slotCnt = other.m_slotCnt;
        m_slotActivity = std::move(other.m_slotActivity);
        m_slotTexUnits = std::move(other.m_slotTexUnits);

        std::copy(std::begin(other.m_texUnitTexIndexes), std::end(other.m_texUnitTexIndexes), m_texUnitTexIndexes);
        std::copy(std::begin(other.m_texUnitRefCnts), std::end(other.m_texUnitRefCnts), m_texUnitRefCnts);

        other.m_vertArrayGLID = 0;
        other.m_vertBufGLID = 0;
        other.m_elemBufGLID = 0;
    }

    return *this;
}

void SpriteBatch::draw(const InternalShaderProgs &internalShaderProgs, const Assets &assets, const zfw2_common::Vec2DInt windowSize) const
{
    const GLID progGLID = internalShaderProgs.spriteQuadProgGLID;

    glUseProgram(progGLID);

    // Set up the projection matrix.
    const auto projMat = zfw2_common::Matrix4x4::create_ortho(0.0f, windowSize.x, windowSize.y, 0.0f, -1.0f, 1.0f);
    glUniformMatrix4fv(glGetUniformLocation(progGLID, "u_proj"), 1, false, reinterpret_cast<const float *>(projMat.elems));

    // Set up the view matrix.
    const auto viewMat = zfw2_common::Matrix4x4::create_identity();
    glUniformMatrix4fv(glGetUniformLocation(progGLID, "u_view"), 1, false, reinterpret_cast<const float *>(viewMat.elems));

    // Set up texture units.
    int texUnits[gk_texUnitLimit];
    std::iota(texUnits, texUnits + gk_texUnitLimit, 0);

    for (int i = 0; i < gk_texUnitLimit; ++i)
    {
        glActiveTexture(GL_TEXTURE0 + i);
        glBindTexture(GL_TEXTURE_2D, m_texUnitRefCnts[i] > 0 ? assets.texGLIDs[m_texUnitTexIndexes[i]] : 0);
    }

    glUniform1iv(glGetUniformLocation(progGLID, "u_textures"), gk_texUnitLimit, texUnits);

    // Draw the batch.
    glBindVertexArray(m_vertArrayGLID);
    glDrawElements(GL_TRIANGLES, 6 * m_slotCnt, GL_UNSIGNED_SHORT, nullptr);
}

int SpriteBatch::take_any_slot(const int texIndex)
{
    // If there are no inactive slots, don't go any further.
    if (m_slotActivity.is_full())
    {
        return -1;
    }

    // Try to find a texture unit to use, and don't go any further if not found.
    const int texUnit = find_tex_unit_to_use(texIndex);

    if (texUnit == -1)
    {
        return -1;
    }

    // Use the first inactive slot.
    const int slotIndex = m_slotActivity.get_first_inactive_bit_index();

    // Mark the slot as active.
    m_slotActivity.activate_bit(slotIndex);

    // Update texture unit information.
    ++m_texUnitRefCnts[texUnit];
    m_texUnitTexIndexes[texUnit] = texIndex;
    m_slotTexUnits[slotIndex] = texUnit;

    return slotIndex;
}

void SpriteBatch::release_slot(const int slotIndex)
{
    // Mark the slot as inactive.
    m_slotActivity.deactivate_bit(slotIndex);

    // Update texture unit information.
    const int texUnit = m_slotTexUnits[slotIndex];
    --m_texUnitRefCnts[texUnit];

    // Clear the slot render data.
    clear_slot(slotIndex);
}

void SpriteBatch::write_to_slot(const int slotIndex, const Assets &assets, const zfw2_common::Vec2D pos, const zfw2_common::Rect &srcRect, const zfw2_common::Vec2D origin, const float rot, const zfw2_common::Vec2D scale, const float alpha) const
{
    const int texUnit = m_slotTexUnits[slotIndex];
    const zfw2_common::Vec2DInt texSize = assets.texSizes[m_texUnitTexIndexes[texUnit]];

    const float verts[gk_spriteQuadShaderProgVertCnt * 4] = {
        (0.0f - origin.x) * scale.x,
        (0.0f - origin.y) * scale.y,
        pos.x,
        pos.y,
        static_cast<float>(srcRect.width),
        static_cast<float>(srcRect.height),
        rot,
        static_cast<float>(texUnit),
        static_cast<float>(srcRect.x) / texSize.x,
        static_cast<float>(srcRect.y) / texSize.y,
        alpha,

        (1.0f - origin.x) * scale.x,
        (0.0f - origin.y) * scale.y,
        pos.x,
        pos.y,
        static_cast<float>(srcRect.width),
        static_cast<float>(srcRect.height),
        rot,
        static_cast<float>(texUnit),
        static_cast<float>(srcRect.x + srcRect.width) / texSize.x,
        static_cast<float>(srcRect.y) / texSize.y,
        alpha,

        (1.0f - origin.x) * scale.x,
        (1.0f - origin.y) * scale.y,
        pos.x,
        pos.y,
        static_cast<float>(srcRect.width),
        static_cast<float>(srcRect.height),
        rot,
        static_cast<float>(texUnit),
        static_cast<float>(srcRect.x + srcRect.width) / texSize.x,
        static_cast<float>(srcRect.y + srcRect.height) / texSize.y,
        alpha,

        (0.0f - origin.x) * scale.x,
        (1.0f - origin.y) * scale.y,
        pos.x,
        pos.y,
        static_cast<float>(srcRect.width),
        static_cast<float>(srcRect.height),
        rot,
        static_cast<float>(texUnit),
        static_cast<float>(srcRect.x) / texSize.x,
        static_cast<float>(srcRect.y + srcRect.height) / texSize.y,
        alpha
    };

    glBindVertexArray(m_vertArrayGLID);
    glBindBuffer(GL_ARRAY_BUFFER, m_vertBufGLID);
    glBufferSubData(GL_ARRAY_BUFFER, slotIndex * sizeof(verts), sizeof(verts), verts);
}

void SpriteBatch::clear_slot(const int slotIndex) const
{
    glBindVertexArray(m_vertArrayGLID);
    glBindBuffer(GL_ARRAY_BUFFER, m_vertBufGLID);

    const float verts[gk_spriteQuadShaderProgVertCnt * 4] = {};
    glBufferSubData(GL_ARRAY_BUFFER, slotIndex * sizeof(verts), sizeof(verts), verts);
}

int SpriteBatch::find_tex_unit_to_use(const int texIndex) const
{
    int freeTexUnit = -1;

    for (int i = 0; i < gk_texUnitLimit; ++i)
    {
        if (!m_texUnitRefCnts[i])
        {
            if (freeTexUnit == -1)
            {
                freeTexUnit = i;
            }

            continue;
        }

        if (m_texUnitTexIndexes[i] == texIndex)
        {
            return i;
        }
    }

    return freeTexUnit;
}

CharBatch::CharBatch(const int slotCnt, const int fontIndex, const zfw2_common::Vec2D pos) : m_slotCnt(slotCnt), m_fontIndex(fontIndex), m_pos(pos)
{
    assert(slotCnt > 0);

    // Generate vertex array.
    glGenVertexArrays(1, &m_vertArrayGLID);
    glBindVertexArray(m_vertArrayGLID);

    // Generate vertex buffer.
    glGenBuffers(1, &m_vertBufGLID);
    glBindBuffer(GL_ARRAY_BUFFER, m_vertBufGLID);

    {
        const int verts_len = gk_charQuadShaderProgVertCnt * 4 * slotCnt;
        const auto verts = std::make_unique<const float[]>(verts_len);
        glBufferData(GL_ARRAY_BUFFER, sizeof(verts[0]) * verts_len, verts.get(), GL_DYNAMIC_DRAW);
    }

    // Generate element buffer.
    glGenBuffers(1, &m_elemBufGLID);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_elemBufGLID);

    {
        // TODO: Have it so that the indices are only set up once; all batches use a copy of this same data, so recalculation isn't needed.
        const int indicesLen = 6 * slotCnt;
        const auto indices = std::make_unique<unsigned short[]>(indicesLen);

        for (int i = 0; i < slotCnt; i++)
        {
            indices[(i * 6) + 0] = (i * 4) + 0;
            indices[(i * 6) + 1] = (i * 4) + 1;
            indices[(i * 6) + 2] = (i * 4) + 2;
            indices[(i * 6) + 3] = (i * 4) + 2;
            indices[(i * 6) + 4] = (i * 4) + 3;
            indices[(i * 6) + 5] = (i * 4) + 0;
        }

        glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices[0]) * indicesLen, indices.get(), GL_STATIC_DRAW);
    }

    // Set vertex attribute pointers.
    const int vertsStride = sizeof(float) * gk_charQuadShaderProgVertCnt;

    glVertexAttribPointer(0, 2, GL_FLOAT, false, vertsStride, reinterpret_cast<void *>(sizeof(float) * 0));
    glEnableVertexAttribArray(0);

    glVertexAttribPointer(1, 2, GL_FLOAT, false, vertsStride, reinterpret_cast<void *>(sizeof(float) * 2));
    glEnableVertexAttribArray(1);

    glBindVertexArray(0);
}

CharBatch::~CharBatch()
{
    glDeleteBuffers(1, &m_elemBufGLID);
    glDeleteBuffers(1, &m_vertBufGLID);
    glDeleteVertexArrays(1, &m_vertArrayGLID);
}

CharBatch::CharBatch(CharBatch &&other) noexcept
    : m_pos(other.m_pos),
    m_rot(other.m_rot),
    m_blend(other.m_blend),
    m_vertArrayGLID(other.m_vertArrayGLID),
    m_vertBufGLID(other.m_vertBufGLID),
    m_elemBufGLID(other.m_elemBufGLID),
    m_activeSlotCnt(other.m_activeSlotCnt),
    m_slotCnt(other.m_slotCnt),
    m_fontIndex(other.m_fontIndex)
{
    other.m_vertArrayGLID = 0;
    other.m_vertBufGLID = 0;
    other.m_elemBufGLID = 0;
}

CharBatch &CharBatch::operator=(CharBatch &&other) noexcept
{
    if (this != &other)
    {
        m_pos = other.m_pos;
        m_rot = other.m_rot;
        m_blend = other.m_blend;
        m_vertArrayGLID = other.m_vertArrayGLID;
        m_vertBufGLID = other.m_vertBufGLID;
        m_elemBufGLID = other.m_elemBufGLID;
        m_activeSlotCnt = other.m_activeSlotCnt;
        m_slotCnt = other.m_slotCnt;
        m_fontIndex = other.m_fontIndex;

        other.m_vertArrayGLID = 0;
        other.m_vertBufGLID = 0;
        other.m_elemBufGLID = 0;
    }

    return *this;
}

void CharBatch::write(const std::string &text, const FontAlignHor alignHor, const FontAlignVer alignVer, const Assets &assets)
{
    assert(text.length() > 0 && text.length() <= m_slotCnt);

    m_activeSlotCnt = text.length();

    const zfw2_common::FontData &fontData = assets.fontDatas[m_fontIndex];

    // Determine the positions of text characters based on font information, alongside the overall dimensions of the text to be used when applying alignment.
    std::vector<zfw2_common::Vec2D> charDrawPositions(m_slotCnt);
    zfw2_common::Vec2D charDrawPosPen = {0};

    std::vector<int> textLineWidths(m_slotCnt);
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

    // Clear the batch then write the character render data.
    glBindVertexArray(m_vertArrayGLID);
    glBindBuffer(GL_ARRAY_BUFFER, m_vertBufGLID);

    {
        const std::vector<float> batchClearVerts((gk_charQuadShaderProgVertCnt * 4) * m_slotCnt);
        glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(batchClearVerts), batchClearVerts.data());
    }

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

void CharBatch::draw(const InternalShaderProgs &internalShaderProgs, const Assets &assets, const zfw2_common::Vec2DInt windowSize) const
{
    const GLID progGLID = internalShaderProgs.charQuadProgGLID;
    glUseProgram(progGLID);

    const auto projMat = zfw2_common::Matrix4x4::create_ortho(0.0f, windowSize.x, windowSize.y, 0.0f, -1.0f, 1.0f);
    glUniformMatrix4fv(glGetUniformLocation(progGLID, "u_proj"), 1, false, reinterpret_cast<const float *>(projMat.elems));

    glUniform2fv(glGetUniformLocation(progGLID, "u_pos"), 1, reinterpret_cast<const float *>(&m_pos));
    glUniform1f(glGetUniformLocation(progGLID, "u_rot"), m_rot);
    glUniform4fv(glGetUniformLocation(progGLID, "u_blend"), 1, reinterpret_cast<const float *>(&m_blend));

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, assets.fontTexGLIDs[m_fontIndex]);

    glBindVertexArray(m_vertArrayGLID);
    glDrawElements(GL_TRIANGLES, 6 * m_activeSlotCnt, GL_UNSIGNED_SHORT, nullptr);
}

void RenderLayer::draw(const InternalShaderProgs &internalShaderProgs, const Assets &assets, const zfw2_common::Vec2DInt windowSize) const
{
    for (const SpriteBatch &sb : m_spriteBatches)
    {
        sb.draw(internalShaderProgs, assets, windowSize);
    }

    for (int i = 0; i < m_charBatches.size(); ++i)
    {
        if (m_charBatchActivity.is_bit_active(i))
        {
            m_charBatches[i].draw(internalShaderProgs, assets, windowSize);
        }
    }
}

void RenderLayer::take_any_sprite_batch_slot(const int texIndex, int &batchIndex, int &slotIndex)
{
    for (int i = 0; i < m_spriteBatches.size(); ++i)
    {
        const int takenSlotIndex = m_spriteBatches[i].take_any_slot(texIndex);

        if (takenSlotIndex != -1)
        {
            batchIndex = i;
            slotIndex = takenSlotIndex;
            return;
        }
    }

    // A slot failed to be taken from an existing batch, so create a new one.
    SpriteBatch newBatch(m_defaultSpriteBatchSlotCnt);
    batchIndex = m_spriteBatches.size();
    slotIndex = newBatch.take_any_slot(texIndex);
    m_spriteBatches.emplace_back(std::move(newBatch));
}

int RenderLayer::add_char_batch(const int slotCnt, const int fontIndex, const zfw2_common::Vec2D pos)
{
    // Find an unused character batch.
    const int batchIndex = m_charBatchActivity.get_first_inactive_bit_index();

    if (batchIndex == -1)
    {
        // Failed to find an unused character batch, so create a new one.
        m_charBatches.emplace_back(slotCnt, fontIndex, pos);
        m_charBatchActivity.resize(m_charBatches.size());
        m_charBatchActivity.activate_bit(m_charBatches.size() - 1);
        return m_charBatches.size() - 1;
    }

    // Replace the old character batch with a new one.
    m_charBatches[batchIndex] = CharBatch(slotCnt, fontIndex, pos);
    m_charBatchActivity.activate_bit(batchIndex);

    return batchIndex;
}

void Renderer::draw(const InternalShaderProgs &internalShaderProgs, const Assets &assets, const zfw2_common::Vec2DInt windowSize) const
{
    glClearColor(m_bgColor.r, m_bgColor.g, m_bgColor.b, m_bgColor.a);
    glClear(GL_COLOR_BUFFER_BIT);

    for (const RenderLayer &layer : m_layers)
    {
        layer.draw(internalShaderProgs, assets, windowSize);
    }
}

SpriteBatchSlotKey Renderer::take_any_sprite_batch_slot(const std::string &layerName, const int texIndex)
{
    assert(m_layersLocked);

    SpriteBatchSlotKey key;
    key.layerIndex = m_layerNamesToIndexes[layerName];
    m_layers[key.layerIndex].take_any_sprite_batch_slot(texIndex, key.batchIndex, key.slotIndex);
    return key;
}

}
