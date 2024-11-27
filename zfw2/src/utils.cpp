#include <zfw2/utils.h>

namespace zfw2
{

bool is_heap_bitset_full(const HeapBitset &bitset)
{
    for (int i = 0; i < bitset.byteCnt; ++i)
    {
        if (bitset.bytes[i] != 0xFF)
        {
            return false;
        }
    }

    return true;
}

int find_first_inactive_bit_in_heap_bitset(const HeapBitset &bitset)
{
    for (int i = 0; i < bitset.byteCnt; ++i)
    {
        if (bitset.bytes[i] == 0xFF)
        {
            continue;
        }

        for (int j = 0; j < 8; ++j)
        {
            const int bitIndex = (i * 8) + j;

            if (!is_heap_bitset_bit_active(bitset, bitIndex))
            {
                return bitIndex;
            }
        }
    }

    return -1;
}

}
