#include <iostream>
#include <stb_image.h>
#include <zfw2_common/math.h>
#include <zfw2_common/assets.h>
#include "common.h"

bool pack_textures(const std::vector<TexPackingInfo> &packingInfos, std::ofstream &assetsFileOS, const std::string &assetsDir)
{
    for (const TexPackingInfo &packingInfo : packingInfos)
    {
        const std::string texFilePath = assetsDir + "/" + packingInfo.relFilePath;

        zfw2_common::Vec2DInt texSize;
        stbi_uc *const pxData = stbi_load(texFilePath.c_str(), &texSize.x, &texSize.y, nullptr, zfw2_common::gk_texChannelCnt);

        if (!pxData)
        {
            std::cerr << "STB ERROR: " << stbi_failure_reason() << std::endl;
            return false;
        }

        if (texSize.x > zfw2_common::gk_texSizeLimit.x || texSize.y > zfw2_common::gk_texSizeLimit.y)
        {
            std::cerr << "ERROR: The texture \"" << texFilePath << "\" exceeds the size limit of " << zfw2_common::gk_texSizeLimit.x << " by " << zfw2_common::gk_texSizeLimit.y << "!" << std::endl;
            stbi_image_free(pxData);
            return false;
        }

        assetsFileOS.write(reinterpret_cast<const char *>(&texSize), sizeof(texSize));
        assetsFileOS.write(reinterpret_cast<const char *>(pxData), texSize.x * texSize.y * zfw2_common::gk_texChannelCnt);

        stbi_image_free(pxData);

        std::cout << "Successfully packed texture \"" << texFilePath << "\"." << std::endl;
    }

    return true;
}
