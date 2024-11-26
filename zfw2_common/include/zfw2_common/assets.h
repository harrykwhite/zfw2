#pragma once

#include "math.h"
#include <string>

namespace zfw2_common
{

using AudioSample = short;

const std::string gk_assetsFileName = "assets.zfw2dat";

constexpr Vec2DInt gk_texSizeLimit = {1024, 1024};
constexpr int gk_texChannelCnt = 4;

constexpr int gk_fontCharRangeBegin = 32;
constexpr int gk_fontCharRangeSize = 95;
constexpr int gk_fontTexChannelCnt = 4;

struct FontCharData
{
    int horOffsets[gk_fontCharRangeSize];
    int verOffsets[gk_fontCharRangeSize];
    int horAdvances[gk_fontCharRangeSize];

    zfw2_common::Rect srcRects[gk_fontCharRangeSize];

    int kernings[gk_fontCharRangeSize * gk_fontCharRangeSize];
};

struct FontData
{
    int lineHeight;
    FontCharData chars;
    zfw2_common::Vec2DInt texSize;
};

struct AudioMetadata
{
    int channelCnt;
    int sampleCntPerChannel;
    unsigned int sampleRate;
};

}
