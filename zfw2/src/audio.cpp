#include <zfw2/audio.h>

#include <iostream>
#include <array>

namespace zfw2
{

static void release_sound_src_by_index(const int index, SoundSrcCollection &srcs)
{
    assert(index >= 0 && index < SoundSrcCollection::k_srcLimit);
    assert(srcs.alIDs[index]);
    
    alDeleteSources(1, &srcs.alIDs[index]);
    srcs.alIDs[index] = 0;
}

static void load_music_buf_data(const ALID bufALID, const MusicSrc &src, const Assets &assets, MemArena &memArena)
{
    assert(bufALID);

    const auto buf = memArena.alloc<zfw2_common::AudioSample>(MusicSrc::k_musicBufSampleCnt);
    const int bytesRead = std::fread(buf, 1, MusicSrc::k_musicBufSize, src.fs);

    if (gk_musicDebugMsgsEnabled)
    {
        std::cout << "Read " << bytesRead << " bytes from \"" + assets.musicFilenames[src.musicIndex] + "\"." << std::endl;
    }

    if (std::feof(src.fs))
    {
        // We've reached the end of the file, so rewind to the start for the next read.
        std::fseek(src.fs, 0, SEEK_SET);
    }

    const zfw2_common::AudioMetadata &metadata = assets.musicMetadatas[src.musicIndex];

    const ALenum format = metadata.channelCnt == 1 ? AL_FORMAT_MONO16 : AL_FORMAT_STEREO16;
    alBufferData(bufALID, format, buf, bytesRead, metadata.sampleRate);

    memArena.rewind();
}

void clean_sound_srcs(SoundSrcCollection &srcs)
{
    for (int i = 0; i < SoundSrcCollection::k_srcLimit; ++i)
    {
        if (srcs.alIDs[i])
        {
            alDeleteSources(1, &srcs.alIDs[i]);
        }
    }

    srcs = {};
}

SoundSrcID create_sound_src(const int soundIndex, SoundSrcCollection &srcs, const Assets &assets)
{
    for (int i = 0; i < SoundSrcCollection::k_srcLimit; ++i) // TODO: To speed this up, consider storing and accessing the index of the most recently released source.
    {
        if (!srcs.alIDs[i])
        {
            alGenSources(1, &srcs.alIDs[i]);
            alSourcei(srcs.alIDs[i], AL_BUFFER, assets.soundBufALIDs[soundIndex]);

            ++srcs.versions[i];

            return {i, srcs.versions[i]};
        }
    }

    assert(false);
}

void release_sound_src(const SoundSrcID srcID, SoundSrcCollection &srcs)
{
    assert(srcID.index >= 0 && srcID.index < SoundSrcCollection::k_srcLimit);
    assert(srcs.versions[srcID.index] == srcID.version);
    assert(srcs.alIDs[srcID.index]);

    release_sound_src_by_index(srcID.index, srcs);
}

void play_sound_src(const SoundSrcID srcID, const SoundSrcCollection &srcs, const Assets &assets, const float gain, const float pitch)
{
    assert(srcID.index >= 0 && srcID.index < SoundSrcCollection::k_srcLimit);
    assert(srcs.versions[srcID.index] == srcID.version);
    assert(srcs.alIDs[srcID.index]);

    alSourceRewind(srcs.alIDs[srcID.index]); // Restart if already playing.
    alSourcef(srcs.alIDs[srcID.index], AL_GAIN, gain);
    alSourcef(srcs.alIDs[srcID.index], AL_PITCH, pitch);
    alSourcePlay(srcs.alIDs[srcID.index]);
}

void create_and_play_sound_src(const int soundIndex, SoundSrcCollection &srcs, const Assets &assets, const float gain, const float pitch)
{
    const SoundSrcID srcID = create_sound_src(soundIndex, srcs, assets);
    play_sound_src(srcID, srcs, assets, gain, pitch);
    srcs.autoReleases.set(srcID.index); // No reference to this source is returned, so it needs to be automatically released once it is detected as finished.
}

void handle_auto_release_sound_srcs(SoundSrcCollection &srcs)
{
    for (int i = 0; i < SoundSrcCollection::k_srcLimit; ++i)
    {
        if (!srcs.autoReleases.test(i))
        {
            continue;
        }

        if (!srcs.alIDs[i])
        {
            srcs.autoReleases.reset(i);
            continue;
        }

        ALint srcState;
        alGetSourcei(srcs.alIDs[i], AL_SOURCE_STATE, &srcState);
        
        if (srcState == AL_STOPPED)
        {
            release_sound_src_by_index(i, srcs);
        }
    }
}

void clean_music_srcs(MusicSrcCollection &collection)
{
    for (int i = 0; i < MusicSrcCollection::k_srcLimit; ++i)
    {
        if (collection.activity.test(i))
        {
            alSourceStop(collection.srcs[i].alID);
            alSourcei(collection.srcs[i].alID, AL_BUFFER, 0);

            alDeleteBuffers(MusicSrc::k_musicBufCnt, collection.srcs[i].bufALIDs);
            alDeleteSources(1, &collection.srcs[i].alID);

            if (collection.srcs[i].fs)
            {
                std::fclose(collection.srcs[i].fs);
            }
        }
    }

    collection = {};
}

MusicSrcID add_music_src(const int musicIndex, MusicSrcCollection &collection, const Assets &assets, MemArena &memArena)
{
    for (int i = 0; i < MusicSrcCollection::k_srcLimit; ++i)
    {
        if (!collection.activity.test(i))
        {
            collection.srcs[i].musicIndex = musicIndex;
            alGenSources(1, &collection.srcs[i].alID);
            alGenBuffers(MusicSrc::k_musicBufCnt, collection.srcs[i].bufALIDs);

            collection.activity.set(i);
            ++collection.versions[i];
            
            return {i, collection.versions[i]};
        }
    }

    assert(false);
}

bool play_music_src(const MusicSrcID id, MusicSrcCollection &collection, const Assets &assets, MemArena &memArena)
{
    assert(id.index >= 0 && id.index < MusicSrcCollection::k_srcLimit);
    assert(collection.versions[id.index] == id.version);
    assert(collection.activity.test(id.index));

    MusicSrc &src = collection.srcs[id.index];

    // Open the music file.
    src.fs = std::fopen(assets.musicFilenames[src.musicIndex].c_str(), "rb");

    if (!src.fs)
    {
        std::cerr << "ERROR: Failed to open \"" << assets.musicFilenames[src.musicIndex] << "\"!" << std::endl;
        return false;
    }

    // Set up and queue buffers.
    for (int i = 0; i < MusicSrc::k_musicBufCnt; ++i)
    {
        load_music_buf_data(src.bufALIDs[i], src, assets, memArena);
    }

    alSourceQueueBuffers(src.alID, MusicSrc::k_musicBufCnt, src.bufALIDs);

    // Play the music.
    alSourcePlay(src.alID);

    return true;
}

void release_music_src(const MusicSrcID srcID, MusicSrcCollection &collection)
{
    assert(srcID.index >= 0 && srcID.index < MusicSrcCollection::k_srcLimit);
    assert(collection.versions[srcID.index] == srcID.version);
    assert(collection.activity.test(srcID.index));

    alDeleteBuffers(MusicSrc::k_musicBufCnt, collection.srcs[srcID.index].bufALIDs);
    alDeleteSources(1, &collection.srcs[srcID.index].alID);
    std::fclose(collection.srcs[srcID.index].fs);

    collection.srcs[srcID.index] = {};

    collection.activity.reset(srcID.index);
}

void refresh_music_srcs(MusicSrcCollection &collection, const Assets &assets, MemArena &memArena)
{
    for (int i = 0; i < MusicSrcCollection::k_srcLimit; ++i)
    {
        if (!collection.activity.test(i))
        {
            continue;
        }

        const MusicSrc &src = collection.srcs[i];

        // Retrieve all processed buffers, fill them with new data and queue them again.
        int processedBufCnt;
        alGetSourcei(src.alID, AL_BUFFERS_PROCESSED, &processedBufCnt);

        const zfw2_common::AudioMetadata &metadata = assets.musicMetadatas[src.musicIndex];

        while (processedBufCnt > 0)
        {
            ALID bufALID;
            alSourceUnqueueBuffers(src.alID, 1, &bufALID);

            load_music_buf_data(bufALID, src, assets, memArena);

            alSourceQueueBuffers(src.alID, 1, &bufALID);

            processedBufCnt--;
        }
    }
}

}
