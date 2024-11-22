#pragma once

#include <fstream>
#include <algorithm>
#include <cassert>
#include <glad/glad.h>
#include <zfw2_common/misc.h>

namespace zfw2
{

using GLID = GLuint;

constexpr inline int get_bit_to_byte_cnt(const int bitCnt)
{
    return (bitCnt + 7) & ~7;
}

class DynamicBitset
{
public:
    DynamicBitset() = default;
    DynamicBitset(const int bitCnt);

    void resize(const int bitCnt);
    int get_first_inactive_bit_index() const; // Returns -1 if all bits are active.
    bool is_full() const;
    bool is_clear() const;

    inline int get_bit_cnt() const
    {
        return m_bitCnt;
    }

    inline void fill()
    {
        std::fill(m_bytes.get(), m_bytes.get() + m_byteCnt, 0xFF);
    }

    inline void clear()
    {
        std::fill(m_bytes.get(), m_bytes.get() + m_byteCnt, 0);
    }

    inline void activate_bit(const int bit_index)
    {
        assert(bit_index >= 0 && bit_index < m_bitCnt);
        m_bytes[bit_index / 8] |= static_cast<unsigned char>(1) << (bit_index % 8);
    }

    inline void deactivate_bit(const int bit_index)
    {
        assert(bit_index >= 0 && bit_index < m_bitCnt);
        m_bytes[bit_index / 8] &= ~(static_cast<unsigned char>(1) << (bit_index % 8));
    }

    inline bool is_bit_active(const int bit_index) const
    {
        assert(bit_index >= 0 && bit_index < m_bitCnt);
        return m_bytes[bit_index / 8] & (static_cast<unsigned char>(1) << (bit_index % 8));
    }

private:
    std::unique_ptr<zfw2_common::Byte[]> m_bytes;
    int m_byteCnt = 0;
    int m_bitCnt = 0;
};

template<typename T>
T read_from_ifs(std::ifstream &ifs)
{
    T val;
    ifs.read(reinterpret_cast<char *>(&val), sizeof(T));
    return val;
}

}
