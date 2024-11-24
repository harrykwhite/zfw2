#include <zfw2_common/assets.h>
#include <AudioFile.h>
#include "common.h"

static bool load_audio_file_and_write_data(const std::filesystem::path filePath, std::ofstream &metadataOFS, std::ofstream &samplesOFS)
{
    // Open the audio file.
    AudioFile<short> audioFile;
    audioFile.shouldLogErrorsToConsole(false);

    if (!audioFile.load(filePath.string()))
    {
        std::cerr << "ERROR: Failed to load audio file " << filePath << "." << std::endl;
        return false;
    }

    // Write the audio file metadata.
    const zfw2_common::AudioMetadata metadata = {
        audioFile.getNumChannels(),
        audioFile.getNumSamplesPerChannel(),
        audioFile.getSampleRate()
    };

    metadataOFS.write(reinterpret_cast<const char *>(&metadata), sizeof(metadata));
    
    // Write the sample data.
    if (metadata.channelCnt == 1)
    {
        for (int i = 0; i < metadata.sampleCntPerChannel; ++i)
        {
            const short sample = audioFile.samples[0][i];
            samplesOFS.write(reinterpret_cast<const char *>(&sample), sizeof(sample));
        }
    }
    else
    {
        // The channel count is 2.
        for (int i = 0; i < metadata.sampleCntPerChannel; ++i)
        {
            const short sample1 = audioFile.samples[0][i];
            samplesOFS.write(reinterpret_cast<const char *>(&sample1), sizeof(sample1));

            const short sample2 = audioFile.samples[1][i];
            samplesOFS.write(reinterpret_cast<const char *>(&sample2), sizeof(sample2));
        }
    }
}

bool pack_sounds(const std::vector<SoundPackingInfo> &packingInfos, std::ofstream &assetsFileOS, const std::filesystem::path &inputDir)
{
    for (const SoundPackingInfo &packingInfo : packingInfos)
    {
        const std::filesystem::path soundFilePath = inputDir / packingInfo.relFilePath;
        load_audio_file_and_write_data(soundFilePath, assetsFileOS, assetsFileOS);
        std::cout << "Successfully packed sound with file path " << soundFilePath << "." << std::endl;
    }

    return true;
}

bool pack_music(const std::vector<MusicPackingInfo> &packingInfos, std::ofstream &assetsFileOS, const std::filesystem::path &inputDir, const std::filesystem::path &outputDir)
{
    for (const MusicPackingInfo &packingInfo : packingInfos)
    {
        const std::filesystem::path musicFilePath = inputDir / packingInfo.relFilePath;

        // Determine the output music file name and path.
        const std::string outputMusicFileName = musicFilePath.stem().string() + ".zfw2dat";
        const int outputMusicFileNameLen = outputMusicFileName.size();

        if (outputMusicFileNameLen > 255)
        {
            std::cerr << "ERROR: The determined output music file name \"" << outputMusicFileName << "\" is too long." << std::endl;
            return false;
        }

        const std::filesystem::path outputMusicFilePath = outputDir / outputMusicFileName;

        // Open the output music file.
        std::ofstream outputMusicFileOS(outputMusicFileName, std::ios::binary);

        if (!outputMusicFileOS.is_open())
        {
            std::cerr << "ERROR: Failed to create or replace " << outputMusicFilePath << " from music file " << musicFilePath << "." << std::endl;
            return false;
        }

        // Write the name of the output music file to the input assets file.
        const unsigned char outputMusicFileNameLenUC = outputMusicFileName.size();
        outputMusicFileOS.write(reinterpret_cast<const char *>(&outputMusicFileNameLenUC), sizeof(outputMusicFileNameLenUC)); // First byte is the string length.
        outputMusicFileOS.write(outputMusicFileName.c_str(), outputMusicFileName.size()); // The rest of the bytes are the characters without a terminating '\0'.

        // Load the music file and write its metadata to the input assets file, and samples to the output music file.
        load_audio_file_and_write_data(musicFilePath, assetsFileOS, outputMusicFileOS);

        std::cout << "Successfully packed music with file path " << musicFilePath << "." << std::endl;
    }

    return true;
}
