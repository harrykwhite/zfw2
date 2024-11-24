#pragma once

#include <cassert>
#include <string>
#include <memory>
#include <zfw2_common/math.h>
#include <zfw2_common/assets.h>
#include "utils.h"

namespace zfw2
{

constexpr int gk_spriteQuadShaderProgVertCnt = 11;
constexpr int gk_charQuadShaderProgVertCnt = 4;

#if 0
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

    inline ALID get_sound_buf_al_id(const int index) const
    {
        assert(index >= 0 && index < m_soundCnt);
        return m_soundBufALIDs[index];
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

    std::unique_ptr<ALID[]> m_soundBufALIDs;
};
#endif

struct Assets
{
    const int texCnt;
    const int shaderProgCnt;
    const int fontCnt;
    const int soundCnt;
    const int musicCnt;

    const std::unique_ptr<const GLID[]> texGLIDs;
    const std::unique_ptr<const zfw2_common::Vec2DInt[]> texSizes;

    const std::unique_ptr<const GLID[]> shaderProgGLIDs;

    const std::unique_ptr<const GLID[]> fontTexGLIDs;
    const std::unique_ptr<const zfw2_common::FontData[]> fontDatas;

    const std::unique_ptr<const ALID[]> soundBufALIDs;
};

struct InternalShaderProgs
{
    const GLID spriteQuadProgGLID;
    const GLID charQuadProgGLID;
};

Assets load_assets(bool &err);
void clean_assets(const Assets &assets);

InternalShaderProgs load_internal_shader_progs();
void clean_internal_shader_progs(const InternalShaderProgs &internalShaderProgs);

}
