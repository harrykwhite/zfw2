#pragma once

#include <fstream>
#include <algorithm>
#include <cassert>
#include <glad/glad.h>
#include <zfw2_common/misc.h>
#include <AL/al.h>
#include "memory.h"

namespace zfw2
{

using GLID = GLuint;
using ALID = ALuint;

struct HeapBitset
{
    zfw2_common::Byte *bytes;
    int byteCnt;
    int bitCnt;
};

bool is_heap_bitset_full(const HeapBitset &bitset);
int find_first_inactive_bit_in_heap_bitset(const HeapBitset &bitset);

inline HeapBitset create_heap_bitset(HeapBitset &bitset, const int bitCnt)
{
    const int byteCnt = (bitCnt + 7) & ~7;

    return {
        .bytes = mem_arena_alloc<zfw2_common::Byte>(byteCnt),
        .byteCnt = byteCnt,
        .bitCnt = bitCnt
    };
}

inline HeapBitset create_heap_bitset(const int bitCnt)
{
    const int byteCnt = (bitCnt + 7) & ~7;

    return {
        .bytes = mem_arena_alloc<zfw2_common::Byte>(byteCnt),
        .byteCnt = byteCnt,
        .bitCnt = bitCnt
    };
}

inline void activate_heap_bitset_bit(HeapBitset &bitset, const int bitIndex)
{
    assert(bitIndex >= 0 && bitIndex < bitset.bitCnt);
    bitset.bytes[bitIndex / 8] |= static_cast<zfw2_common::Byte>(1) << (bitIndex % 8);
}

inline void deactivate_heap_bitset_bit(HeapBitset &bitset, const int bitIndex)
{
    assert(bitIndex >= 0 && bitIndex < bitset.bitCnt);
    bitset.bytes[bitIndex / 8] &= ~(static_cast<zfw2_common::Byte>(1) << (bitIndex % 8));
}

inline bool is_heap_bitset_bit_active(const HeapBitset &bitset, const int bitIndex)
{
    assert(bitIndex >= 0 && bitIndex < bitset.bitCnt);
    return bitset.bytes[bitIndex / 8] & (static_cast<zfw2_common::Byte>(1) << (bitIndex % 8));
}

inline void clear_heap_bitset(HeapBitset &bitset)
{
    std::memset(bitset.bytes, 0, bitset.byteCnt);
}

constexpr inline int get_bit_to_byte_cnt(const int bitCnt)
{
    return (bitCnt + 7) & ~7;
}

template<typename T>
inline T read_from_ifs(std::ifstream &ifs)
{
    T val;
    ifs.read(reinterpret_cast<char *>(&val), sizeof(T));
    return val;
}

}
