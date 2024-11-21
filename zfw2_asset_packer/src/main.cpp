#include <iostream>
#include <filesystem>
#include <nlohmann/json.hpp>
#include <zfw2_common/assets.h>
#include "common.h"

const std::string gk_assetPackingInfoFileName = "asset_packing_info.json";

static std::vector<TexPackingInfo> get_tex_packing_infos_from_json(const nlohmann::json &json)
{
    std::vector<TexPackingInfo> packingInfos;

    if (json.contains("tex_rfps"))
    {
        for (const auto &texRFP : json["tex_rfps"])
        {
            TexPackingInfo packingInfo;
            packingInfo.relFilePath = texRFP;
            packingInfos.emplace_back(packingInfo);
        }
    }

    return packingInfos;
}

static std::vector<ShaderProgPackingInfo> get_shader_prog_packing_infos_from_json(const nlohmann::json &json)
{
    std::vector<ShaderProgPackingInfo> packingInfos;

    if (json.contains("shader_progs"))
    {
        for (const auto &shaderProg : json["shader_progs"])
        {
            ShaderProgPackingInfo packingInfo;
            packingInfo.vertShaderRFP = shaderProg["vs_rfp"];
            packingInfo.fragShaderRFP = shaderProg["fs_rfp"];
            packingInfos.emplace_back(packingInfo);
        }
    }

    return packingInfos;
}

int main(const int argCnt, const char *const *const args)
{
    if (argCnt > 3)
    {
        std::cerr << "ERROR: Too many arguments provided!" << std::endl;
        return EXIT_FAILURE;
    }

    // Get the assets directory either from the command-line arguments or from user input.
    const std::string assetsDir = [argCnt, args]() -> std::string
    {
        if (argCnt > 1)
        {
            return std::string(args[1]);
        }

        std::string assetsDir;
        std::cout << "Enter the assets directory (i.e. the directory containing the asset packing information JSON file): ";
        std::getline(std::cin, assetsDir);
        return assetsDir;
    }();

    // Verify that the assets directory exists.
    if (!std::filesystem::exists(assetsDir))
    {
        std::cerr << "ERROR: The provided assets directory \"" << assetsDir << "\"does not exist!" << std::endl;
        return EXIT_FAILURE;
    }

    // Get the output directory for the assets file either from the command-line arguments or from user input.
    const std::string assetsFileOutputDir = [argCnt, args]() -> std::string
    {
        if (argCnt > 2)
        {
            return std::string(args[2]);
        }

        std::string outputDir;
        std::cout << "Enter the output directory for the assets file: ";
        std::getline(std::cin, outputDir);
        return outputDir;
    }();

    // Open the packing information JSON file.
    const std::string packingInfoFilePath = assetsDir + "/" + gk_assetPackingInfoFileName;
    std::ifstream packingInfoFileIS(packingInfoFilePath);

    if (!packingInfoFileIS.is_open())
    {
        std::cerr << "ERROR: Failed to open the asset packing information JSON file!" << std::endl;
        return EXIT_FAILURE;
    }

    nlohmann::json packingInfoJSON;

    try
    {
        packingInfoFileIS >> packingInfoJSON;
    }
    catch (const nlohmann::json::parse_error &err)
    {
        std::cerr << "ERROR: Failed to parse the asset packing information JSON file!" << std::endl;
        return EXIT_FAILURE;
    }

    // Get asset packing information from the JSON.
    const std::vector<TexPackingInfo> texPackingInfos = get_tex_packing_infos_from_json(packingInfoJSON);
    const std::vector<ShaderProgPackingInfo> shaderProgPackingInfos = get_shader_prog_packing_infos_from_json(packingInfoJSON);

    // Close the packing information JSON file. (TODO: Do this via scoping.)
    packingInfoFileIS.close();

    // Create the output assets file.
    const std::string assetsFilePath = assetsFileOutputDir + "/" + zfw2_common::gk_assetsFileName;
    std::ofstream assetsFileOS(assetsFilePath, std::ios::binary);

    if (!assetsFileOS.is_open())
    {
        std::cerr << "ERROR: Failed to create or replace the output assets file with path \"" << assetsFilePath << "\"!" << std::endl;
        return EXIT_FAILURE;
    }

    // Write the asset counts (the file header).
    const int texCnt = texPackingInfos.size();
    assetsFileOS.write(reinterpret_cast<const char *>(&texCnt), sizeof(texCnt));

    const int shaderProgCnt = shaderProgPackingInfos.size();
    assetsFileOS.write(reinterpret_cast<const char *>(&shaderProgCnt), sizeof(shaderProgCnt));

    // Pack the assets.
    if (!pack_textures(texPackingInfos, assetsFileOS, assetsDir)
        || !pack_shader_progs(shaderProgPackingInfos, assetsFileOS, assetsDir))
    {
        std::remove(zfw2_common::gk_assetsFileName.c_str());
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}
