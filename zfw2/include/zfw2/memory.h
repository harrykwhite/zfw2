#pragma once

#include <memory>
#include <cassert>
#include <zfw2_common/misc.h>

namespace zfw2
{

class MemArena
{
public:
    MemArena(const int size) : m_buf(new zfw2_common::Byte[size])
    {
    }

    template<typename T>
    T *alloc(const int cnt)
    {
        assert(cnt > 0);
        const int size = cnt * sizeof(T);
        m_bufOffs += size;
        m_rewindSize = size;
        return reinterpret_cast<T *>(m_buf.get() + m_bufOffs);
    }

    template<typename T>
    T *alloc_and_clear(const int cnt)
    {
        assert(cnt > 0);

        T *const ptr = alloc<T>(cnt);
        std::fill(ptr, ptr + cnt, T());
        return ptr;
    }

    // Undos the most recent allocation. This is useful for temporary buffers.
    void rewind()
    {
        assert(m_rewindSize > 0);
        m_bufOffs -= m_rewindSize;
        m_rewindSize = 0;
    }

private:
    const std::unique_ptr<zfw2_common::Byte> m_buf;
    int m_bufOffs = 0;
    int m_rewindSize = 0;
};

}
