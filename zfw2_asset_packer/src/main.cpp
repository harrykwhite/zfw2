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

static std::vector<FontPackingInfo> get_font_packing_infos_from_json(const nlohmann::json &json)
{
    std::vector<FontPackingInfo> packingInfos;

    if (json.contains("fonts"))
    {
        for (const auto &fonts : json["fonts"])
        {
            FontPackingInfo packingInfo;
            packingInfo.relFilePath = fonts["rfp"];
            packingInfo.ptSize = fonts["pt_size"];
            packingInfos.emplace_back(packingInfo);
        }
    }

    return packingInfos;
}

static std::vector<SoundPackingInfo> get_sound_packing_infos_from_json(const nlohmann::json &json)
{
    std::vector<SoundPackingInfo> packingInfos;

    if (json.contains("sound_rfps"))
    {
        for (const auto &soundRFP : json["sound_rfps"])
        {
            SoundPackingInfo packingInfo;
            packingInfo.relFilePath = soundRFP;
            packingInfos.emplace_back(packingInfo);
        }
    }

    return packingInfos;
}

static std::vector<MusicPackingInfo> get_music_packing_infos_from_json(const nlohmann::json &json)
{
    std::vector<MusicPackingInfo> packingInfos;

    if (json.contains("music_rfps"))
    {
        for (const auto &musicRFP : json["music_rfps"])
        {
            MusicPackingInfo packingInfo;
            packingInfo.relFilePath = musicRFP;
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

    // Get the input directory either from the command-line arguments or from user input.
    const std::filesystem::path inputDir = [argCnt, args]()
    {
        if (argCnt > 1)
        {
            return std::string(args[1]);
        }

        std::string dir;
        std::cout << "Enter the input directory (i.e. the directory containing the asset packing information JSON file): ";
        std::getline(std::cin, dir);
        return dir;
    }();

    // Verify that the input directory exists.
    if (!std::filesystem::exists(inputDir))
    {
        std::cerr << "ERROR: The provided input directory " << inputDir << " does not exist!" << std::endl;
        return EXIT_FAILURE;
    }

    // Get the output directory either from the command-line arguments or from user input.
    const std::filesystem::path outputDir = [argCnt, args]()
    {
        if (argCnt > 2)
        {
            return std::string(args[2]);
        }

        std::string outputDir;
        std::cout << "Enter the output directory: ";
        std::getline(std::cin, outputDir);
        return outputDir;
    }();

    // Verify that the output directory exists.
    if (!std::filesystem::exists(outputDir))
    {
        std::cerr << "ERROR: The provided output directory " << outputDir << " does not exist!" << std::endl;
        return EXIT_FAILURE;
    }

    // Open the packing information JSON file.
    const std::filesystem::path packingInfoFilePath = inputDir / gk_assetPackingInfoFileName;
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

    // Get asset packing information from the JSON header.
    const std::vector<TexPackingInfo> texPackingInfos = get_tex_packing_infos_from_json(packingInfoJSON);
    const std::vector<ShaderProgPackingInfo> shaderProgPackingInfos = get_shader_prog_packing_infos_from_json(packingInfoJSON);
    const std::vector<FontPackingInfo> fontPackingInfos = get_font_packing_infos_from_json(packingInfoJSON);
    const std::vector<SoundPackingInfo> soundPackingInfos = get_sound_packing_infos_from_json(packingInfoJSON);
    const std::vector<MusicPackingInfo> musicPackingInfos = get_music_packing_infos_from_json(packingInfoJSON);

    // Close the packing information JSON file.
    packingInfoFileIS.close();

    // Create the output assets file.
    const std::filesystem::path outputAssetsFilePath = outputDir / zfw2_common::gk_assetsFileName;
    std::ofstream outputAssetsFileOS(outputAssetsFilePath, std::ios::binary);

    if (!outputAssetsFileOS.is_open())
    {
        std::cerr << "ERROR: Failed to create or replace the output assets file with path " << outputAssetsFilePath << "!" << std::endl;
        return EXIT_FAILURE;
    }

    // Write the asset counts (the header).
    const int texCnt = texPackingInfos.size();
    outputAssetsFileOS.write(reinterpret_cast<const char *>(&texCnt), sizeof(texCnt));

    const int shaderProgCnt = shaderProgPackingInfos.size();
    outputAssetsFileOS.write(reinterpret_cast<const char *>(&shaderProgCnt), sizeof(shaderProgCnt));

    const int fontCnt = fontPackingInfos.size();
    outputAssetsFileOS.write(reinterpret_cast<const char *>(&fontCnt), sizeof(fontCnt));

    const int soundCnt = soundPackingInfos.size();
    outputAssetsFileOS.write(reinterpret_cast<const char *>(&soundCnt), sizeof(soundCnt));

    const int musicCnt = musicPackingInfos.size();
    outputAssetsFileOS.write(reinterpret_cast<const char *>(&musicCnt), sizeof(musicCnt));

    // Pack the assets.
    if (!pack_textures(texPackingInfos, outputAssetsFileOS, inputDir)
        || !pack_shader_progs(shaderProgPackingInfos, outputAssetsFileOS, inputDir)
        || !pack_fonts(fontPackingInfos, outputAssetsFileOS, inputDir)
        || !pack_sounds(soundPackingInfos, outputAssetsFileOS, inputDir)
        || !pack_music(musicPackingInfos, outputAssetsFileOS, inputDir, outputDir))
    {
        std::remove(zfw2_common::gk_assetsFileName.c_str());
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}
