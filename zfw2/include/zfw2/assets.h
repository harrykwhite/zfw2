#pragma once

#include <cassert>
#include <string>
#include <memory>
#include <zfw2_common/math.h>
#include <zfw2_common/assets.h>
#include "utils.h"

namespace zfw2
{

class Assets
{
public:
    Assets() = default;
    ~Assets();
    Assets(const Assets &other) = delete;
    Assets &operator=(const Assets &other) = delete;

    bool load_all(const std::string &filename);

    inline GLID get_tex_gl_id(const int index) const
    {
        assert(index >= 0 && index < m_texCnt);
        return m_texGLIDs[index];
    }

    inline zfw2_common::Vec2DInt get_tex_size(const int index) const
    {
        assert(index >= 0 && index < m_texCnt);
        return m_texSizes[index];
    }

    inline GLID get_shader_prog_gl_id(const int index) const
    {
        assert(index >= 0 && index < m_shaderProgCnt);
        return m_shaderProgGLIDs[index];
    }

    inline GLID get_font_tex_gl_id(const int index) const
    {
        assert(index >= 0 && index < m_fontCnt);
        return m_fontTexGLIDs[index];
    }

    inline const zfw2_common::FontData &get_font_data(const int index) const
    {
        assert(index >= 0 && index < m_fontCnt);
        return m_fontDatas[index];
    }

private:
    bool m_loaded = false;

    int m_texCnt = 0;
    int m_shaderProgCnt = 0;
    int m_fontCnt = 0;
    int m_soundCnt = 0;

    std::unique_ptr<GLID[]> m_texGLIDs;
    std::unique_ptr<zfw2_common::Vec2DInt[]> m_texSizes;

    std::unique_ptr<GLID[]> m_shaderProgGLIDs;

    std::unique_ptr<GLID[]> m_fontTexGLIDs;
    std::unique_ptr<zfw2_common::FontData[]> m_fontDatas;
};

class InternalShaderProgs
{
public:
    InternalShaderProgs() = default;
    ~InternalShaderProgs();
    InternalShaderProgs(const InternalShaderProgs &other) = delete;
    InternalShaderProgs &operator=(const InternalShaderProgs &other) = delete;

    void load_all();

    GLID get_sprite_quad_prog_gl_id() const
    {
        return m_spriteQuadProgGLID;
    }

    GLID get_char_quad_prog_gl_id() const
    {
        return m_charQuadProgGLID;
    }

private:
    GLID m_spriteQuadProgGLID = 0;
    GLID m_charQuadProgGLID = 0;
};

constexpr int gk_spriteQuadShaderProgVertCnt = 11;
constexpr int gk_charQuadShaderProgVertCnt = 4;

}
