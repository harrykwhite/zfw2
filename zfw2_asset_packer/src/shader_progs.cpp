#include <iostream>
#include "common.h"

bool pack_shader_progs(const std::vector<ShaderProgPackingInfo> &packingInfos, std::ofstream &assetsFileOS, const std::filesystem::path &inputDir)
{
    for (const ShaderProgPackingInfo &packingInfo : packingInfos)
    {
        // Pack vertex shader then fragment shader.
        for (int i = 0; i < 2; i++)
        {
            const std::filesystem::path shaderFilePath = inputDir / (i == 0 ? packingInfo.vertShaderRFP : packingInfo.fragShaderRFP);

            // Open the shader file.
            std::ifstream shaderIS(shaderFilePath, std::ios::binary | std::ios::ate);

            if (!shaderIS.is_open())
            {
                std::cerr << "ERROR: Failed to open shader " << shaderFilePath << "!" << std::endl;
                return false;
            }

            // Get the shader file size.
            const int shaderFileSize = shaderIS.tellg();
            shaderIS.seekg(0, std::ios::beg);

            // Write shader source size to assets file.
            const int shaderSrcSize = shaderFileSize + 1; // Account for the '\0' we're going to write.
            assetsFileOS.write(reinterpret_cast<const char *>(&shaderSrcSize), sizeof(shaderSrcSize));

            // Allocate buffer for shader content and read it.
            std::unique_ptr<char[]> shaderSrc(new char[shaderSrcSize]);

            shaderIS.read(shaderSrc.get(), shaderFileSize);
            shaderSrc[shaderSrcSize - 1] = '\0';

            // Write the shader content to assets file.
            assetsFileOS.write(shaderSrc.get(), shaderSrcSize);

            std::cout << "Successfully packed shader " << shaderFilePath << "." << std::endl;
        }
    }

    return true;
}
