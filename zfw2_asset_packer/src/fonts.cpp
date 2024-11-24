#include <iostream>
#include <zfw2_common/assets.h>
#include <zfw2_common/misc.h>

#include <ft2build.h>
#include FT_FREETYPE_H

#include "common.h"

struct FontDataWithPixels
{
    zfw2_common::FontData fd;
    std::unique_ptr<zfw2_common::Byte[]> texPxData;
};

static inline int get_line_height(const FT_Face ftFace)
{
    return ftFace->size->metrics.height >> 6;
}

static int calc_largest_bitmap_width(const FT_Face ftFace, const FT_Library ftLib)
{
    int width = 0;

    for (int i = 0; i < zfw2_common::gk_fontCharRangeSize; i++)
    {
        FT_Load_Glyph(ftFace, FT_Get_Char_Index(ftFace, zfw2_common::gk_fontCharRangeBegin + i), FT_LOAD_DEFAULT);
        FT_Render_Glyph(ftFace->glyph, FT_RENDER_MODE_NORMAL);

        if (ftFace->glyph->bitmap.width > width)
        {
            width = ftFace->glyph->bitmap.width;
        }
    }

    return width;
}

static zfw2_common::Vec2DInt calc_font_tex_size(const FT_Face ftFace, const FT_Library ftLib)
{
    const int largestGlyphBitmapWidth = calc_largest_bitmap_width(ftFace, ftLib);
    const int idealTexWidth = largestGlyphBitmapWidth * zfw2_common::gk_fontCharRangeSize;

    return {
        std::min(idealTexWidth, zfw2_common::gk_texSizeLimit.x),
        get_line_height(ftFace) * ((idealTexWidth / zfw2_common::gk_texSizeLimit.x) + 1)
    };
}

static std::unique_ptr<FontDataWithPixels> get_font_data_with_px(const std::filesystem::path &filePath, const int ptSize, const FT_Library ftLib)
{
    auto fontDataWithPx = std::make_unique<FontDataWithPixels>();

    // Set up the font face.
    FT_Face ftFace;

    if (FT_New_Face(ftLib, filePath.string().c_str(), 0, &ftFace))
    {
        std::cout << "ERROR: Failed to create a FreeType face object for font with file path " << filePath << "." << std::endl;
        return nullptr;
    }

    FT_Set_Char_Size(ftFace, ptSize << 6, 0, 96, 0);

    fontDataWithPx->fd.lineHeight = get_line_height(ftFace);

    // Initialise the font texture, setting all the pixels to transparent white.
    fontDataWithPx->fd.texSize = calc_font_tex_size(ftFace, ftLib);

    if (fontDataWithPx->fd.texSize.y > zfw2_common::gk_texSizeLimit.y)
    {
        std::cout << "ERROR: Font texture size is too large!" << std::endl;
        return nullptr;
    }

    const int texPxDataSize = fontDataWithPx->fd.texSize.x * fontDataWithPx->fd.texSize.y * zfw2_common::gk_fontTexChannelCnt;
    fontDataWithPx->texPxData = std::make_unique<zfw2_common::Byte[]>(texPxDataSize);

    for (int i = 0; i < fontDataWithPx->fd.texSize.x * fontDataWithPx->fd.texSize.y; ++i)
    {
        const int pxDataIndex = i * zfw2_common::gk_fontTexChannelCnt;

        fontDataWithPx->texPxData[pxDataIndex + 0] = 255;
        fontDataWithPx->texPxData[pxDataIndex + 1] = 255;
        fontDataWithPx->texPxData[pxDataIndex + 2] = 255;
        fontDataWithPx->texPxData[pxDataIndex + 3] = 0;
    }

    // Get and store information for all font characters.
    int charDrawX = 0;
    int charDrawY = 0;

    for (int i = 0; i < zfw2_common::gk_fontCharRangeSize; i++)
    {
        const FT_UInt ftCharIndex = FT_Get_Char_Index(ftFace, zfw2_common::gk_fontCharRangeBegin + i);

        FT_Load_Glyph(ftFace, ftCharIndex, FT_LOAD_DEFAULT);
        FT_Render_Glyph(ftFace->glyph, FT_RENDER_MODE_NORMAL);

        if (charDrawX + ftFace->glyph->bitmap.width > zfw2_common::gk_texSizeLimit.x)
        {
            charDrawX = 0;
            charDrawY += fontDataWithPx->fd.lineHeight;
        }

        fontDataWithPx->fd.chars.horOffsets[i] = ftFace->glyph->metrics.horiBearingX >> 6;
        fontDataWithPx->fd.chars.verOffsets[i] = (ftFace->size->metrics.ascender - ftFace->glyph->metrics.horiBearingY) >> 6;

        fontDataWithPx->fd.chars.horAdvances[i] = ftFace->glyph->metrics.horiAdvance >> 6;

        fontDataWithPx->fd.chars.srcRects[i].x = charDrawX;
        fontDataWithPx->fd.chars.srcRects[i].y = charDrawY;
        fontDataWithPx->fd.chars.srcRects[i].width = ftFace->glyph->bitmap.width;
        fontDataWithPx->fd.chars.srcRects[i].height = ftFace->glyph->bitmap.rows;

        // Set kernings (one per character combination).
        for (int j = 0; j < zfw2_common::gk_fontCharRangeSize; j++)
        {
            FT_Vector ftKerning;
            FT_Get_Kerning(ftFace, FT_Get_Char_Index(ftFace, zfw2_common::gk_fontCharRangeBegin + j), ftCharIndex, FT_KERNING_DEFAULT, &ftKerning);

            fontDataWithPx->fd.chars.kernings[(zfw2_common::gk_fontCharRangeSize * i) + j] = ftKerning.x >> 6;
        }

        // Update the font texture's pixel data with the character.
        for (int y = 0; y < fontDataWithPx->fd.chars.srcRects[i].height; y++)
        {
            for (int x = 0; x < fontDataWithPx->fd.chars.srcRects[i].width; x++)
            {
                const unsigned char pxAlpha = ftFace->glyph->bitmap.buffer[(y * ftFace->glyph->bitmap.width) + x];

                if (pxAlpha > 0)
                {
                    const int pxX = fontDataWithPx->fd.chars.srcRects[i].x + x;
                    const int pxY = fontDataWithPx->fd.chars.srcRects[i].y + y;
                    const int pxDataIndex = (pxY * fontDataWithPx->fd.texSize.x * zfw2_common::gk_fontTexChannelCnt) + (pxX * zfw2_common::gk_fontTexChannelCnt);

                    fontDataWithPx->texPxData[pxDataIndex + 3] = pxAlpha;
                }
            }
        }

        charDrawX += fontDataWithPx->fd.chars.srcRects[i].width;
    }

    FT_Done_Face(ftFace);

    return fontDataWithPx;
}

bool pack_fonts(const std::vector<FontPackingInfo> &packingInfos, std::ofstream &assetsFileOS, const std::filesystem::path &inputDir)
{
    FT_Library ftLib;

    if (FT_Init_FreeType(&ftLib))
    {
        std::cerr << "ERROR: Failed to initialise FreeType!" << std::endl;
        return false;
    }

    for (const FontPackingInfo &packingInfo : packingInfos)
    {
        const std::filesystem::path fontFilePath = inputDir / packingInfo.relFilePath;

        // Get the font data using FreeType.
        const std::unique_ptr<FontDataWithPixels> fontDataWithPx = get_font_data_with_px(fontFilePath.string(), packingInfo.ptSize, ftLib);

        if (!fontDataWithPx)
        {
            FT_Done_FreeType(ftLib);
            return false;
        }

        // Write the font data to the file.
        assetsFileOS.write(reinterpret_cast<const char *>(&fontDataWithPx->fd), sizeof(fontDataWithPx->fd));
        assetsFileOS.write(reinterpret_cast<const char *>(fontDataWithPx->texPxData.get()), fontDataWithPx->fd.texSize.x * fontDataWithPx->fd.texSize.y * zfw2_common::gk_fontTexChannelCnt);

        std::cout << "Successfully packed font with file path " << fontFilePath << " and point size " << packingInfo.ptSize << "." << std::endl;
    }

    FT_Done_FreeType(ftLib);

    return true;
}
