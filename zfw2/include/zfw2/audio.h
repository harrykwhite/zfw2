#pragma once

#include <bitset>
#include "assets.h"
#include "utils.h"

namespace zfw2
{

class SoundManager
{
public:
    ~SoundManager();

    void refresh_srcs();
    void play(const int soundIndex, const Assets &assets, const float gain = 1.0f, const float pitch = 1.0f);

private:
    static constexpr int k_srcLimit = 256;

    ALID m_srcALIDs[k_srcLimit] = {};
};

}
