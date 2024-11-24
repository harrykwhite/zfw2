#include <zfw2/assets.h>

#include <fstream>
#include <array>
#include <iostream>

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
    float rot_cos = cos(u_rot);
    float rot_sin = sin(u_rot);

    mat4 model = mat4(
        vec4(rot_cos, rot_sin, 0.0f, 0.0f),
        vec4(-rot_sin, rot_cos, 0.0f, 0.0f),
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

Assets load_assets(bool &err)
{
    err = false;

    // Try opening the assets file.
    std::ifstream ifs(zfw2_common::gk_assetsFileName, std::ios::binary);

    if (!ifs.is_open())
    {
        std::cerr << "ERROR: Failed to open \"" << zfw2_common::gk_assetsFileName << "\"!" << std::endl;
        err = true;
        return {};
    }

    // Read asset counts from the header.
    const int texCnt = read_from_ifs<int>(ifs);
    const int shaderProgCnt = read_from_ifs<int>(ifs);
    const int fontCnt = read_from_ifs<int>(ifs);
    const int soundCnt = read_from_ifs<int>(ifs);
    const int musicCnt = read_from_ifs<int>(ifs);

    //
    // Textures
    //
    std::unique_ptr<GLID[]> texGLIDs = nullptr;
    std::unique_ptr<zfw2_common::Vec2DInt[]> texSizes = nullptr;

    if (texCnt > 0)
    {
        // Generate textures and store their IDs.
        texGLIDs = std::make_unique<GLID[]>(texCnt);
        glGenTextures(texCnt, texGLIDs.get());

        // Read the sizes and pixel data of textures and finish setting them up.
        texSizes = std::make_unique<zfw2_common::Vec2DInt[]>(texCnt);

        const int px_data_buf_size = zfw2_common::gk_texChannelCnt * zfw2_common::gk_texSizeLimit.x * zfw2_common::gk_texSizeLimit.y;
        const auto px_data_buf = std::make_unique<unsigned char[]>(px_data_buf_size); // This is working space for temporarily storing the pixel data of each texture.

        for (int i = 0; i < texCnt; ++i)
        {
            ifs.read(reinterpret_cast<char *>(&texSizes[i]), sizeof(zfw2_common::Vec2DInt));
            ifs.read(reinterpret_cast<char *>(px_data_buf.get()), zfw2_common::gk_texChannelCnt * texSizes[i].x * texSizes[i].y);

            glBindTexture(GL_TEXTURE_2D, texGLIDs[i]);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
            glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, texSizes[i].x, texSizes[i].y, 0, GL_RGBA, GL_UNSIGNED_BYTE, px_data_buf.get());
        }
    }

    //
    // Shader Programs
    //
    std::unique_ptr<GLID[]> shaderProgGLIDs = nullptr;

    if (shaderProgCnt > 0)
    {
        shaderProgGLIDs = std::make_unique<GLID[]>(shaderProgCnt);

        for (int i = 0; i < shaderProgCnt; ++i)
        {
            const auto vertShaderSrcSize = read_from_ifs<int>(ifs);
            std::string vertShaderSrc;
            vertShaderSrc.reserve(vertShaderSrcSize);
            ifs.read(vertShaderSrc.data(), vertShaderSrcSize);

            const auto fragShaderSrcSize = read_from_ifs<int>(ifs);
            std::string fragShaderSrc;
            fragShaderSrc.reserve(fragShaderSrcSize);
            ifs.read(fragShaderSrc.data(), fragShaderSrcSize);

            shaderProgGLIDs[i] = create_shader_prog_from_srcs(vertShaderSrc, fragShaderSrc);
        }
    }

    //
    // Fonts
    //
    std::unique_ptr<GLID[]> fontTexGLIDs = nullptr;
    std::unique_ptr<zfw2_common::FontData[]> fontDatas = nullptr;

    if (fontCnt > 0)
    {
        fontTexGLIDs = std::make_unique<GLID[]>(fontCnt);
        glGenTextures(fontCnt, fontTexGLIDs.get());

        fontDatas = std::make_unique<zfw2_common::FontData[]>(fontCnt);

        const int px_data_buf_size = zfw2_common::gk_texChannelCnt * zfw2_common::gk_texSizeLimit.x * zfw2_common::gk_texSizeLimit.y;
        const auto px_data_buf = std::make_unique<unsigned char[]>(px_data_buf_size); // This is working space for temporarily storing the pixel data of each font texture.

        for (int i = 0; i < fontCnt; ++i)
        {
            ifs.read(reinterpret_cast<char *>(&fontDatas[i]), sizeof(zfw2_common::FontData));
            ifs.read(reinterpret_cast<char *>(px_data_buf.get()), zfw2_common::gk_texChannelCnt * fontDatas[i].texSize.x * fontDatas[i].texSize.y);

            glBindTexture(GL_TEXTURE_2D, fontTexGLIDs[i]);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
            glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, fontDatas[i].texSize.x, fontDatas[i].texSize.y, 0, GL_RGBA, GL_UNSIGNED_BYTE, px_data_buf.get());
        }
    }

    //
    // Sounds
    //
    std::unique_ptr<ALID[]> soundBufALIDs = nullptr;

    if (soundCnt > 0)
    {
        soundBufALIDs = std::make_unique<ALID[]>(soundCnt);
        alGenBuffers(soundCnt, soundBufALIDs.get());

        for (int i = 0; i < soundCnt; ++i)
        {
            const auto channelCnt = read_from_ifs<int>(ifs);
            const auto sampleCntPerChannel = read_from_ifs<int>(ifs);
            const auto sampleRate = read_from_ifs<unsigned int>(ifs);

            const int sampleCnt = sampleCntPerChannel * channelCnt;
            const auto sampleData = std::make_unique<short[]>(sampleCnt);
            const int sampleDataSize = sampleCnt * sizeof(short);

            ifs.read(reinterpret_cast<char *>(sampleData.get()), sampleDataSize);

            const ALenum format = channelCnt == 1 ? AL_FORMAT_MONO16 : AL_FORMAT_STEREO16;
            alBufferData(soundBufALIDs[i], format, sampleData.get(), sampleDataSize, sampleRate);
        }
    }

    return {
        .texCnt = texCnt,
        .shaderProgCnt = shaderProgCnt,
        .fontCnt = fontCnt,
        .soundCnt = soundCnt,
        .musicCnt = musicCnt,

        .texGLIDs = std::move(texGLIDs),
        .texSizes = std::move(texSizes),

        .shaderProgGLIDs = std::move(shaderProgGLIDs),

        .fontTexGLIDs = std::move(fontTexGLIDs),
        .fontDatas = std::move(fontDatas),

        .soundBufALIDs = std::move(soundBufALIDs)
    };
}

void clean_assets(const Assets &assets)
{
    alDeleteBuffers(assets.soundCnt, assets.soundBufALIDs.get());

    glDeleteTextures(assets.fontCnt, assets.fontTexGLIDs.get());

    for (int i = 0; i < assets.shaderProgCnt; ++i)
    {
        glDeleteProgram(assets.shaderProgGLIDs[i]);
    }

    glDeleteTextures(assets.texCnt, assets.texGLIDs.get());
}

InternalShaderProgs load_internal_shader_progs()
{
    return {
        .spriteQuadProgGLID = create_shader_prog_from_srcs(gk_spriteQuadVertShaderSrc, gk_spriteQuadFragShaderSrc),
        .charQuadProgGLID = create_shader_prog_from_srcs(gk_charQuadVertShaderSrc, gk_charQuadFragShaderSrc)
    };
}

void clean_internal_shader_progs(const InternalShaderProgs &internalShaderProgs)
{
    glDeleteProgram(internalShaderProgs.charQuadProgGLID);
    glDeleteProgram(internalShaderProgs.spriteQuadProgGLID);
}

}
