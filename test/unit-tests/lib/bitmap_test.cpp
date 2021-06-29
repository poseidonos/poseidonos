#include "src/lib/bitmap.h"

#include <gtest/gtest.h>

namespace pos
{
TEST(BitMap, BitMap_)
{
}

TEST(BitMap, GetNumBits_)
{
}

TEST(BitMap, GetNumBitsSet_)
{
}

TEST(BitMap, SetNumBitsSet_)
{
}

TEST(BitMap, GetNumEntry_)
{
}

TEST(BitMap, GetMapAddr_)
{
}

TEST(BitMap, PrintMap_)
{
}

TEST(BitMap, SetBit_)
{
}

TEST(BitMap, ClearBit_)
{
}

TEST(BitMap, ClearBits_)
{
}

TEST(BitMap, ResetBitmap_)
{
}

TEST(BitMap, IsSetBit_)
{
}

TEST(BitMap, FlipBit_)
{
}

TEST(BitMap, FindFirstSet_)
{
}

TEST(BitMap, FindFirstZero_)
{
}

TEST(BitMap, FindNextZero_CheckFirstCall)
{
    // Given
    uint64_t bit = 0;
    BitMap bitMapSUT(16);

    // When
    bit = bitMapSUT.FindNextZero();

    // Then
    EXPECT_EQ(bit, 0);
}

TEST(BitMap, FindNextZero_ReturnRightNextPosition)
{
    // Given
    uint64_t bit = 0;
    BitMap bitMapSUT(16);
    bitMapSUT.SetBit(7);

    // When
    bit = bitMapSUT.FindNextZero();

    // Then
    EXPECT_EQ(bit, 8);
}

TEST(BitMap, FindNextZero_FullSet)
{
    // Given
    uint64_t bit_first = 0, bit_second = 0;
    BitMap bitMapSUT(16);

    for (int i = 0; i <= 14; ++i)
    {
        bitMapSUT.SetBit(i);
    }
    bit_first = bitMapSUT.FindNextZero();
    bitMapSUT.SetBit(bit_first);

    // When
    bit_second = bitMapSUT.FindNextZero();

    // Then
    EXPECT_EQ(bit_first, 15);
    EXPECT_EQ(bit_second, 16);
}

TEST(BitMap, IsValidBit_)
{
}

TEST(BitMap, Set_)
{
}

TEST(BitMap, _GetMask_)
{
}

} // namespace pos

namespace pos
{
TEST(BitMapMutex, BitMapMutex_)
{
}

TEST(BitMapMutex, IsValidBit_)
{
}

TEST(BitMapMutex, FindFirstSetBit_)
{
}

TEST(BitMapMutex, SetFirstZeroBit_)
{
}

TEST(BitMapMutex, SetNextZeroBit_)
{
}

TEST(BitMapMutex, SetBit_)
{
}

TEST(BitMapMutex, ClearBit_)
{
}

TEST(BitMapMutex, ClearBits_)
{
}

TEST(BitMapMutex, ResetBitmap_)
{
}

TEST(BitMapMutex, IsSetBit_)
{
}

TEST(BitMapMutex, FlipBit_)
{
}

TEST(BitMapMutex, GetNumEntry_)
{
}

TEST(BitMapMutex, GetNumBits_)
{
}

TEST(BitMapMutex, SetNumBitsSet_)
{
}

TEST(BitMapMutex, GetMapAddr_)
{
}

TEST(BitMapMutex, PrintMap_)
{
}

TEST(BitMapMutex, GetNumBitsSet_)
{
}

TEST(BitMapMutex, GetNumBitsSetWoLock_)
{
}

TEST(BitMapMutex, SetBitMap_)
{
}

TEST(BitMapMutex, GetLock_)
{
}

} // namespace pos
