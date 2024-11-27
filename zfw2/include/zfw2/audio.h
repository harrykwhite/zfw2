#pragma once

#include <algorithm>
#include <vector>
#include <bitset>
#include "assets.h"
#include "memory.h"
#include "utils.h"

namespace zfw2
{

constexpr bool gk_musicDebugMsgsEnabled = true;

struct SoundSrcID
{
    int index;
    int version;
};

struct SoundSrcCollection
{
    static constexpr int k_srcLimit = 128;

    ALID alIDs[k_srcLimit];
    int versions[k_srcLimit];
    std::bitset<k_srcLimit> autoReleases; // Indicates which sources need to be automatically released when finished (due to not them not being referenced).
};

struct MusicSrcID
{
    int index;
    int version;
};

struct MusicSrc
{
    static constexpr int k_musicBufCnt = 4;
    static constexpr int k_musicBufSampleCnt = 44100;
    static constexpr int k_musicBufSize = k_musicBufSampleCnt * sizeof(zfw2_common::AudioSample);

    int musicIndex;

    ALID alID;
    ALID bufALIDs[k_musicBufCnt];

    std::FILE *fs;
};

struct MusicSrcCollection
{
    static constexpr int k_srcLimit = 16;

    MusicSrc srcs[k_srcLimit];
    std::bitset<k_srcLimit> activity;
    int versions[k_srcLimit];
};

void clean_sound_srcs(const SoundSrcCollection &srcs);
SoundSrcID create_sound_src(const int soundIndex, SoundSrcCollection &srcs, const Assets &assets);
void release_sound_src(const SoundSrcID srcID, SoundSrcCollection &srcs);
void play_sound_src(const SoundSrcID srcID, const SoundSrcCollection &srcs, const Assets &assets, const float gain = 1.0f, const float pitch = 1.0f);
void create_and_play_sound_src(const int soundIndex, SoundSrcCollection &srcs, const Assets &assets, const float gain = 1.0f, const float pitch = 1.0f);
void handle_auto_release_sound_srcs(SoundSrcCollection &srcs);

void clean_music_srcs(const MusicSrcCollection &collection);
MusicSrcID add_music_src(const int musicIndex, MusicSrcCollection &collection, const Assets &assets);
bool play_music_src(const MusicSrcID id, MusicSrcCollection &collection, const Assets &assets);
void release_music_src(const MusicSrcID srcID, MusicSrcCollection &collection);
void refresh_music_srcs(MusicSrcCollection &collection, const Assets &assets);

}
