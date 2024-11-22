#include <zfw2/assets.h>

#include <fstream>
#include <array>

namespace zfw2
{

const std::string gk_spriteQuadVertShaderSrc = R"(#version 430 core

layout (location = 0) in vec2 a_vert;
layout (location = 1) in vec2 a_pos;
layout (location = 2) in vec2 a_size;
layout (location = 3) in float a_rot;
layout (location = 4) in float a_tex_index;
layout (location = 5) in vec2 a_tex_coord;
layout (location = 6) in float a_alpha;

out flat int v_tex_index;
out vec2 v_tex_coord;
out float v_alpha;

uniform mat4 u_view;
uniform mat4 u_proj;

void main()
{
    float rot_cos = cos(a_rot);
    float rot_sin = -sin(a_rot);

    mat4 model = mat4(
        vec4(a_size.x * rot_cos, a_size.x * rot_sin, 0.0f, 0.0f),
        vec4(a_size.y * -rot_sin, a_size.y * rot_cos, 0.0f, 0.0f),
        vec4(0.0f, 0.0f, 1.0f, 0.0f),
        vec4(a_pos.x, a_pos.y, 0.0f, 1.0f)
    );

    gl_Position = u_proj * u_view * model * vec4(a_vert, 0.0f, 1.0f);

    v_tex_index = int(a_tex_index);
    v_tex_coord = a_tex_coord;
    v_alpha = a_alpha;
}
)";

const std::string gk_spriteQuadFragShaderSrc = R"(#version 430 core

in flat int v_tex_index;
in vec2 v_tex_coord;
in float v_alpha;

out vec4 o_frag_color;

uniform sampler2D u_textures[32];

void main()
{
    vec4 tex_color = texture(u_textures[v_tex_index], v_tex_coord);
    o_frag_color = tex_color * vec4(1.0f, 1.0f, 1.0f, v_alpha);
}
)";

const std::string gk_charQuadVertShaderSrc = R"(#version 430 core

layout (location = 0) in vec2 a_vert;
layout (location = 1) in vec2 a_tex_coord;

out vec2 v_tex_coord;

uniform vec2 u_pos;
uniform float u_rot;
uniform mat4 u_proj;

void main()
{
    mat4 model = mat4(
        vec4(1.0f, 0.0f, 0.0f, 0.0f),
        vec4(0.0f, 1.0f, 0.0f, 0.0f),
        vec4(0.0f, 0.0f, 1.0f, 0.0f),
        vec4(u_pos.x, u_pos.y, 0.0f, 1.0f)
    );

    gl_Position = u_proj * model * vec4(a_vert, 0.0f, 1.0f);

    v_tex_coord = a_tex_coord;
}
)";

const std::string gk_charQuadFragShaderSrc = R"(#version 430 core

in vec2 v_tex_coord;

out vec4 o_frag_color;

uniform vec4 u_blend;
uniform sampler2D u_tex;

void main()
{
    vec4 tex_color = texture(u_tex, v_tex_coord);
    o_frag_color = tex_color * u_blend;
}
)";

static GLID create_shader_prog_from_srcs(const std::string &vertShaderSrc, const std::string &fragShaderSrc)
{
    const GLID vertShaderGLID = glCreateShader(GL_VERTEX_SHADER);
    const char *const vertShaderSrcPtr = vertShaderSrc.c_str();
    glShaderSource(vertShaderGLID, 1, &vertShaderSrcPtr, nullptr);
    glCompileShader(vertShaderGLID);

    const GLID fragShaderGLID = glCreateShader(GL_FRAGMENT_SHADER);
    const char *const fragShaderSrcPtr = fragShaderSrc.c_str();
    glShaderSource(fragShaderGLID, 1, &fragShaderSrcPtr, nullptr);
    glCompileShader(fragShaderGLID);

    // TODO: Check for shader compilation errors.

    const GLID progGLID = glCreateProgram();
    glAttachShader(progGLID, vertShaderGLID);
    glAttachShader(progGLID, fragShaderGLID);
    glLinkProgram(progGLID);

    glDeleteShader(fragShaderGLID);
    glDeleteShader(vertShaderGLID);

    return progGLID;
}

Assets::~Assets()
{
    if (m_loaded)
    {
        for (int i = 0; i < m_shaderProgCnt; ++i)
        {
            glDeleteProgram(m_shaderProgGLIDs[i]);
        }

        glDeleteTextures(m_texCnt, m_texGLIDs.get());
    }
}

bool Assets::load_all(const std::string &filename)
{
    assert(!m_loaded);

    // Try opening the assets file.
    std::ifstream ifs(filename, std::ios::binary);

    if (!ifs.is_open())
    {
        return false;
    }

    // Read asset counts from the header.
    m_texCnt = read_from_ifs<int>(ifs);
    m_shaderProgCnt = read_from_ifs<int>(ifs);
    m_fontCnt = read_from_ifs<int>(ifs);

    //
    // Textures
    //
    if (m_texCnt > 0)
    {
        // Generate textures and store their IDs.
        m_texGLIDs = std::make_unique<GLID[]>(m_texCnt);
        glGenTextures(m_texCnt, m_texGLIDs.get());

        // Read the sizes and pixel data of textures and finish setting them up.
        m_texSizes = std::make_unique<zfw2_common::Vec2DInt[]>(m_texCnt);

        {
            const int px_data_buf_size = zfw2_common::gk_texChannelCnt * zfw2_common::gk_texSizeLimit.x * zfw2_common::gk_texSizeLimit.y;
            const auto px_data_buf = std::make_unique<unsigned char[]>(px_data_buf_size); // This is working space for temporarily storing the pixel data of each texture.

            for (int i = 0; i < m_texCnt; ++i)
            {
                ifs.read(reinterpret_cast<char *>(&m_texSizes[i]), sizeof(zfw2_common::Vec2DInt));

                ifs.read(reinterpret_cast<char *>(px_data_buf.get()), zfw2_common::gk_texChannelCnt * m_texSizes[i].x * m_texSizes[i].y);

                glBindTexture(GL_TEXTURE_2D, m_texGLIDs[i]);
                glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
                glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
                glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, m_texSizes[i].x, m_texSizes[i].y, 0, GL_RGBA, GL_UNSIGNED_BYTE, px_data_buf.get());
            }
        }
    }

    //
    // Shader Programs
    //
    if (m_shaderProgCnt > 0)
    {
        m_shaderProgGLIDs = std::make_unique<GLID[]>(m_shaderProgCnt);

        for (int i = 0; i < m_shaderProgCnt; ++i)
        {
            const auto vertShaderSrcSize = read_from_ifs<int>(ifs);
            std::string vertShaderSrc;
            vertShaderSrc.reserve(vertShaderSrcSize);
            ifs.read(vertShaderSrc.data(), vertShaderSrcSize);

            const auto fragShaderSrcSize = read_from_ifs<int>(ifs);
            std::string fragShaderSrc;
            fragShaderSrc.reserve(fragShaderSrcSize);
            ifs.read(fragShaderSrc.data(), fragShaderSrcSize);

            m_shaderProgGLIDs[i] = create_shader_prog_from_srcs(vertShaderSrc, fragShaderSrc);
        }
    }

    //
    // Fonts
    //
    if (m_fontCnt > 0)
    {
        m_fontTexGLIDs = std::make_unique<GLID[]>(m_fontCnt);
        m_fontDatas = std::make_unique<zfw2_common::FontData[]>(m_fontCnt);

        const int px_data_buf_size = zfw2_common::gk_texChannelCnt * zfw2_common::gk_texSizeLimit.x * zfw2_common::gk_texSizeLimit.y;
        const auto px_data_buf = std::make_unique<unsigned char[]>(px_data_buf_size); // This is working space for temporarily storing the pixel data of each font texture.

        for (int i = 0; i < m_fontCnt; ++i)
        {
            ifs.read(reinterpret_cast<char *>(&m_fontDatas[i]), sizeof(zfw2_common::FontData));

            ifs.read(reinterpret_cast<char *>(px_data_buf.get()), zfw2_common::gk_texChannelCnt * m_texSizes[i].x * m_texSizes[i].y);

            glBindTexture(GL_TEXTURE_2D, m_fontTexGLIDs[i]);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
            glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, m_fontDatas[i].texSize.x, m_fontDatas[i].texSize.y, 0, GL_RGBA, GL_UNSIGNED_BYTE, px_data_buf.get());
        }
    }

    m_loaded = true;

    return true;
}

InternalShaderProgs::~InternalShaderProgs()
{
    glDeleteProgram(m_charQuadProgGLID);
    glDeleteProgram(m_spriteQuadProgGLID);
}

void InternalShaderProgs::load_all()
{
    m_spriteQuadProgGLID = create_shader_prog_from_srcs(gk_spriteQuadVertShaderSrc, gk_spriteQuadFragShaderSrc);
    m_charQuadProgGLID = create_shader_prog_from_srcs(gk_charQuadVertShaderSrc, gk_charQuadFragShaderSrc);
}

}
