#include <zfw2/utils.h>

namespace zfw2
{

DynamicBitset::DynamicBitset(const int bitCnt) : m_bitCnt(bitCnt), m_byteCnt(get_bit_to_byte_cnt(bitCnt))
{
    assert(m_byteCnt > 0);
    m_bytes = std::make_unique<zfw2_common::Byte[]>(m_byteCnt);
}

void DynamicBitset::resize(const int bitCnt)
{
    assert(bitCnt > 0);

    m_bitCnt = bitCnt;

    const int byteCntLast = m_byteCnt;
    m_byteCnt = get_bit_to_byte_cnt(bitCnt);

    if (m_byteCnt != byteCntLast)
    {
        m_bytes = std::make_unique<zfw2_common::Byte[]>(m_byteCnt);
    }
}

int DynamicBitset::get_first_inactive_bit_index() const
{
    for (int i = 0; i < m_bitCnt; ++i)
    {
        if (!is_bit_active(i))
        {
            return i;
        }
    }

    return -1;
}

bool DynamicBitset::is_full() const
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

bool DynamicBitset::is_clear() const
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
