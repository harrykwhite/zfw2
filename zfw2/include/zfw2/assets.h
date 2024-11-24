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

    const std::unique_ptr<const ALID[]> musicBufALIDs;
    const std::unique_ptr<const std::string[]> musicFilenames;
    const std::unique_ptr<const zfw2_common::AudioMetadata[]> musicMetadatas;
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
