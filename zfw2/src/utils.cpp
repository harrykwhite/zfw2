#include <zfw2/utils.h>

namespace zfw2
{

int HeapBitset::get_first_inactive_bit_index() const
{
    for (int i = 0; i < m_byteCnt; ++i)
    {
        if (m_bytes[i] == 0xFF)
        {
            continue;
        }

        for (int j = 0; j < 8; ++j)
        {
            if (!is_bit_active(j))
            {
                return (i * 8) + j;
            }
        }
    }

    return -1;
}

bool HeapBitset::is_full() const
{
    for (int i = 0; i < m_byteCnt; ++i)
    {
        if (m_bytes[i] != 0xFF)
        {
            return false;
        }
    }

    return true;
}

bool HeapBitset::is_clear() const
{
    for (int i = 0; i < m_byteCnt; ++i)
    {
        if (m_bytes[i] != 0)
        {
            return false;
        }
    }

    return true;
}

}
