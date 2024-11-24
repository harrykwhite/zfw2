#pragma once

#include <string>
#include <vector>
#include <fstream>
#include <filesystem>

struct TexPackingInfo
{
    std::string relFilePath;
};

struct ShaderProgPackingInfo
{
    std::string vertShaderRFP;
    std::string fragShaderRFP;
};

struct FontPackingInfo
{
    std::string relFilePath;
    int ptSize;
};

struct SoundPackingInfo
{
    std::string relFilePath;
};

struct MusicPackingInfo
{
    std::string relFilePath;
};

bool pack_textures(const std::vector<TexPackingInfo> &packingInfos, std::ofstream &assetsFileOS, const std::filesystem::path &inputDir);
bool pack_shader_progs(const std::vector<ShaderProgPackingInfo> &packingInfos, std::ofstream &assetsFileOS, const std::filesystem::path &inputDir);
bool pack_fonts(const std::vector<FontPackingInfo> &packingInfos, std::ofstream &assetsFileOS, const std::filesystem::path &inputDir);
bool pack_sounds(const std::vector<SoundPackingInfo> &packingInfos, std::ofstream &assetsFileOS, const std::filesystem::path &inputDir);
bool pack_music(const std::vector<MusicPackingInfo> &packingInfos, std::ofstream &assetsFileOS, const std::filesystem::path &inputDir, const std::filesystem::path &outputDir);
