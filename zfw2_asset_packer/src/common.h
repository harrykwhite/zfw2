#pragma once

#include <string>
#include <vector>
#include <fstream>

struct TexPackingInfo
{
    std::string relFilePath;
};

struct ShaderProgPackingInfo
{
    std::string vertShaderRFP;
    std::string fragShaderRFP;
};

bool pack_textures(const std::vector<TexPackingInfo> &packingInfos, std::ofstream &assetsFileOS, const std::string &assetsDir);
bool pack_shader_progs(const std::vector<ShaderProgPackingInfo> &packingInfos, std::ofstream &assetsFileOS, const std::string &assetsDir);
