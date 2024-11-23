#include "common.h"

#include <AudioFile.h>

bool pack_sounds(const std::vector<SoundPackingInfo> &packingInfos, std::ofstream &assetsFileOS, const std::string &assetsDir)
{
    for (const SoundPackingInfo &packingInfo : packingInfos)
    {
        const std::string soundFilePath = assetsDir + "/" + packingInfo.relFilePath;

        // Load the sound file.
        AudioFile<short> audioFile;
        audioFile.shouldLogErrorsToConsole(false);

        if (!audioFile.load(soundFilePath))
        {
            std::cerr << "ERROR: Failed to load sound file \"" << soundFilePath << "\"." << std::endl;
            return false;
        }

        // Write sound metadata.
        const int channelCnt = audioFile.getNumChannels();
        assetsFileOS.write(reinterpret_cast<const char *>(&channelCnt), sizeof(channelCnt));

        const int sampleCntPerChannel = audioFile.getNumSamplesPerChannel();
        assetsFileOS.write(reinterpret_cast<const char *>(&sampleCntPerChannel), sizeof(sampleCntPerChannel));

        const unsigned int sampleRate = audioFile.getSampleRate();
        assetsFileOS.write(reinterpret_cast<const char *>(&sampleRate), sizeof(sampleRate));

        // Write the sample data.
        if (channelCnt == 1)
        {
            for (int i = 0; i < sampleCntPerChannel; ++i) {
                const short sample = audioFile.samples[0][i];
                assetsFileOS.write(reinterpret_cast<const char *>(&sample), sizeof(sample));
		    }
        }
        else
        {
            // The channel count is 2.
            for (int i = 0; i < sampleCntPerChannel * 2; i += 2)
            {
                const short sample1 = audioFile.samples[0][i];
                assetsFileOS.write(reinterpret_cast<const char *>(&sample1), sizeof(sample1));

                const short sample2 = audioFile.samples[1][i];
                assetsFileOS.write(reinterpret_cast<const char *>(&sample2), sizeof(sample2));
            }
        }

        std::cout << "Successfully packed sound with file path \"" << packingInfo.relFilePath << "\"." << std::endl;
    }

    return true;
}
