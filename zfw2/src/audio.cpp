#include <zfw2/audio.h>

namespace zfw2
{

SoundManager::~SoundManager()
{
    for (int i = 0; i < k_srcLimit; ++i)
    {
        if (m_srcALIDs[i])
        {
            alDeleteSources(1, &m_srcALIDs[i]);
        }
    }
}

void SoundManager::refresh_srcs()
{
    for (int i = 0; i < k_srcLimit; ++i)
    {
        if (!m_srcALIDs[i])
        {
            continue;
        }

        ALint srcState;
        alGetSourcei(m_srcALIDs[i], AL_SOURCE_STATE, &srcState);

        if (srcState == AL_STOPPED)
        {
            alDeleteSources(1, &m_srcALIDs[i]);
            m_srcALIDs[i] = 0;
        }
    }
}

void SoundManager::play(const int soundIndex, const Assets &assets, const float gain, const float pitch)
{
    for (int i = 0; i < k_srcLimit; ++i)
    {
        if (!m_srcALIDs[i])
        {
            // Set up the source.
            alGenSources(1, &m_srcALIDs[i]);
            alSourcei(m_srcALIDs[i], AL_BUFFER, assets.soundBufALIDs[soundIndex]);

            // Rewind it in case it is already playing.
            alSourceRewind(m_srcALIDs[i]);

            // Play the sound.
            alSourcef(m_srcALIDs[i], AL_GAIN, gain);
            alSourcef(m_srcALIDs[i], AL_PITCH, pitch);
            alSourcePlay(m_srcALIDs[i]);

            return;
        }
    }

    assert(false);
}

}
