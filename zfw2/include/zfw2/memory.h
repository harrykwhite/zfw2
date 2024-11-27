#pragma once

#include <memory>
#include <cassert>
#include <zfw2_common/misc.h>

namespace zfw2
{

constexpr int gk_memArenaSize = (1 << 20) * 128;

extern zfw2_common::Byte g_memArena[gk_memArenaSize];
extern int g_memArenaOffs;

#if 0
struct MemArena
{
    zfw2_common::Byte *buf;
    int bufSize;
    int bufOffs;
};

inline bool init_mem_arena(MemArena &arena, const int size)
{
    assert(size > 0);

    arena = {};

    arena.buf = reinterpret_cast<zfw2_common::Byte *>(std::malloc(size));

    if (!arena.buf)
    {
        return false;
    }

    std::memset(arena.buf, 0, size);

    arena.bufSize = size;
}

inline void clean_mem_arena(MemArena &arena)
{
    std::free(arena.buf);
    arena = {};
}
#endif

template<typename T>
T *mem_arena_alloc(const int cnt)
{
    if (cnt > 0)
    {
        T *const ptr = reinterpret_cast<T *>(g_memArena + g_memArenaOffs);
        g_memArenaOffs += sizeof(T) * cnt;
        return ptr;
    }

    assert(!cnt);

    return nullptr;
}

#if 0
inline void clear_mem_arena(MemArena &arena)
{
    std::memset(arena.buf, 0, arena.bufSize);
    arena.bufOffs = 0;
}
#endif

}
