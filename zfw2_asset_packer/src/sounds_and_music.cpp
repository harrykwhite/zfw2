#include <zfw2_common/assets.h>
#include <AudioFile.h>
#include "common.h"

static bool load_audio_file_and_write_data(const std::filesystem::path filePath, std::ofstream &metadataOFS, std::ofstream &samplesOFS)
{
    // Open the audio file.
    AudioFile<zfw2_common::AudioSample> audioFile;
    audioFile.shouldLogErrorsToConsole(false);

    if (!audioFile.load(filePath.string()))
    {
        std::cerr << "ERROR: Failed to load audio file " << filePath << "." << std::endl;
        return false;
    }

    // Write the audio file metadata.
    const zfw2_common::AudioMetadata metadata = {
        .channelCnt = audioFile.getNumChannels(),
        .sampleCntPerChannel = audioFile.getNumSamplesPerChannel(),
        .sampleRate = audioFile.getSampleRate()
    };

    metadataOFS.write(reinterpret_cast<const char *>(&metadata), sizeof(metadata));

    // Write the sample data.
    if (metadata.channelCnt == 1)
    {
        for (int i = 0; i < metadata.sampleCntPerChannel; ++i)
        {
            const zfw2_common::AudioSample sample = audioFile.samples[0][i];
            samplesOFS.write(reinterpret_cast<const char *>(&sample), sizeof(sample));
        }
    }
    else
    {
        // The channel count is 2.
        for (int i = 0; i < metadata.sampleCntPerChannel; ++i)
        {
            const zfw2_common::AudioSample frame[2] = {
                audioFile.samples[0][i],
                audioFile.samples[1][i]
            };

            samplesOFS.write(reinterpret_cast<const char *>(frame), sizeof(frame));
        }
    }

    return true;
}

bool pack_sounds(const std::vector<SoundPackingInfo> &packingInfos, std::ofstream &assetsFileOS, const std::filesystem::path &inputDir)
{
    for (const SoundPackingInfo &packingInfo : packingInfos)
    {
        const std::filesystem::path soundFilePath = inputDir / packingInfo.relFilePath;

        if (!load_audio_file_and_write_data(soundFilePath, assetsFileOS, assetsFileOS))
        {
            return false;
        }

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

        if (outputMusicFileName.size() > 255)
        {
            std::cerr << "ERROR: The determined output music file name \"" << outputMusicFileName << "\" is too long." << std::endl;
            return false;
        }

        const std::filesystem::path outputMusicFilePath = outputDir / outputMusicFileName;

        // Write the name of the output music file to the input assets file.
        const unsigned char outputMusicFileNameLenUC = outputMusicFileName.size();
        assetsFileOS.write(reinterpret_cast<const char *>(&outputMusicFileNameLenUC), sizeof(outputMusicFileNameLenUC)); // First byte is the string length.

        assetsFileOS.write(outputMusicFileName.data(), outputMusicFileName.size()); // The rest of the bytes are the characters without a terminating '\0'.

        // Open the output music file.
        std::ofstream outputMusicFileOS(outputMusicFileName, std::ios::binary);

        if (!outputMusicFileOS.is_open())
        {
            std::cerr << "ERROR: Failed to create or replace " << outputMusicFilePath << " from music file " << musicFilePath << "." << std::endl;
            return false;
        }

        // Load the music file and write its metadata to the input assets file and samples to the output music file.
        if (!load_audio_file_and_write_data(musicFilePath, assetsFileOS, outputMusicFileOS))
        {
            return false;
        }

        std::cout << "Successfully packed music with file path " << musicFilePath << "." << std::endl;
    }

    return true;
}
