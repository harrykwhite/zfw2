#pragma once

#include <cassert>
#include <string>
#include <memory>
#include <zfw2_common/math.h>
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

    inline GLID get_tex_glid(const int index) const
    {
        assert(index >= 0 && index < m_texCnt);
        return m_texGLIDs[index];
    }

    inline zfw2_common::Vec2DInt get_tex_size(const int index) const
    {
        assert(index >= 0 && index < m_texCnt);
        return m_texSizes[index];
    }

    inline GLID get_shader_prog_glid(const int index) const
    {
        assert(index >= 0 && index < m_shaderProgCnt);
        return m_shaderProgGLIDs[index];
    }

private:
    bool m_loaded = false;

    int m_texCnt = 0;
    int m_shaderProgCnt = 0;

    std::unique_ptr<GLID[]> m_texGLIDs;
    std::unique_ptr<zfw2_common::Vec2DInt[]> m_texSizes;

    std::unique_ptr<GLID[]> m_shaderProgGLIDs;
};

}
