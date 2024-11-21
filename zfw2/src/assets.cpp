#include <zfw2/assets.h>

#include <fstream>
#include <array>
#include <zfw2_common/assets.h>

namespace zfw2
{

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

    //
    // Textures
    //

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

    //
    // Shader Programs
    //
    m_shaderProgGLIDs = std::make_unique<GLID[]>(m_shaderProgCnt);

    for (int i = 0; i < m_shaderProgCnt; ++i)
    {
        // Create the shaders using the sources in the file.
        const std::array<GLID, 2> shaderGLIDs = {
            glCreateShader(GL_VERTEX_SHADER),
            glCreateShader(GL_FRAGMENT_SHADER)
        };

        for (int j = 0; j < shaderGLIDs.size(); ++j)
        {
            const auto srcSize = read_from_ifs<int>(ifs);

            const auto src = std::make_unique<char[]>(srcSize);
            ifs.read(src.get(), srcSize);

            const char *const srcPtr = src.get();
            glShaderSource(shaderGLIDs[j], 1, &srcPtr, nullptr);

            glCompileShader(shaderGLIDs[j]);

            // TODO: Check for a shader compilation error.
        }

        // Create the shader program using the shaders.
        m_shaderProgGLIDs[i] = glCreateProgram();

        for (int j = 0; j < shaderGLIDs.size(); ++j)
        {
            glAttachShader(m_shaderProgGLIDs[i], shaderGLIDs[j]);
        }

        glLinkProgram(m_shaderProgGLIDs[i]);

        // Delete the shaders as they're no longer needed.
        for (int j = 0; j < shaderGLIDs.size(); ++j)
        {
            glDeleteShader(shaderGLIDs[j]);
        }
    }

    m_loaded = true;

    return true;
}

}
