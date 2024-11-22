#include <zfw2/graphics.h>

#include <numeric>

namespace zfw2
{

SpriteBatch::SpriteBatch(const int slotCnt) : m_slotCnt(slotCnt), m_slotActivity(slotCnt / 8), m_slotTexUnits(std::make_unique<int[]>(slotCnt))
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
        const auto verts = std::make_unique<const float[]>(vertsLen);
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
    const GLID progGLID = internalShaderProgs.get_sprite_quad_prog_gl_id();

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
        glBindTexture(GL_TEXTURE_2D, m_texUnitRefCnts[i] > 0 ? assets.get_tex_gl_id(m_texUnitTexIndexes[i]) : 0);
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
    const zfw2_common::Vec2DInt texSize = assets.get_tex_size(m_texUnitTexIndexes[texUnit]);

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

void RenderLayer::draw(const InternalShaderProgs &internalShaderProgs, const Assets &assets, const zfw2_common::Vec2DInt windowSize) const
{
    for (const SpriteBatch &batch : m_spriteBatches)
    {
        batch.draw(internalShaderProgs, assets, windowSize);
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

void Renderer::draw(const InternalShaderProgs &internalShaderProgs, const Assets &assets, const zfw2_common::Vec2DInt windowSize) const
{
    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
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
