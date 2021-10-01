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
    // Given
    uint64_t bit = 0;
    BitMap bitMapSUT(16);

    // When: Set all bits and then clear bit 7
    for (int i = 0; i <= 15; ++i)
    {
        bitMapSUT.SetBit(i);
    }
    bitMapSUT.ClearBit(7);

    // Then: First zero found should be 7
    bit = bitMapSUT.FindFirstZero();
    EXPECT_EQ(bit, 7);
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

TEST(BitMapMutex, SetFirstZeroBit_ZeroArgument)
{
    // Given: set all bits and then clear bit 7
    uint64_t bit = 0;
    BitMapMutex bitMapSUT(16);

    for (int i = 0; i <= 15; ++i)
    {
        bitMapSUT.SetBit(i);
    }
    bitMapSUT.ClearBit(7);

    // When: set first zero bit
    bit = bitMapSUT.SetFirstZeroBit();

    // Then: First zero bit set should be 7
    EXPECT_EQ(bit, 7);
    EXPECT_TRUE(bitMapSUT.IsSetBit(7));
}

TEST(BitMapMutex, SetFirstZeroBit_SingleArgument)
{
    // Given: set all bits and then clear bit 1 and 7
    uint64_t bit = 0;
    BitMapMutex bitMapSUT(16);

    for (int i = 0; i <= 15; ++i)
    {
        bitMapSUT.SetBit(i);
    }
    bitMapSUT.ClearBit(1);
    bitMapSUT.ClearBit(7);

    // When: set first zero bit starting from bit 3
    bit = bitMapSUT.SetFirstZeroBit(3);

    // Then: First zero bit set should be 7
    EXPECT_EQ(bit, 7);
    EXPECT_TRUE(bitMapSUT.IsSetBit(7));
    // Then: bit 1 remains untouched
    EXPECT_FALSE(bitMapSUT.IsSetBit(1));
}

TEST(BitMapMutex, SetFirstZeroBit_TwoArguments)
{
    // Given: set all bits and then clear bit 1, 4, and 7
    uint64_t bit = 0;
    BitMapMutex bitMapSUT(16);

    for (int i = 0; i <= 15; ++i)
    {
        bitMapSUT.SetBit(i);
    }
    bitMapSUT.ClearBit(1);
    bitMapSUT.ClearBit(4);
    bitMapSUT.ClearBit(7);

    // When: set first zero bit ranging from bit 3 to 5
    bit = bitMapSUT.SetFirstZeroBit(3, 5);

    // Then: First zero bit set should be 7
    EXPECT_EQ(bit, 4);
    EXPECT_TRUE(bitMapSUT.IsSetBit(4));
    // Then: bit 1 and 7 remain untouched
    EXPECT_FALSE(bitMapSUT.IsSetBit(1));
    EXPECT_FALSE(bitMapSUT.IsSetBit(7));
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
    //Given
    BitMapMutex bitMapSUT(16);

    //When: set 3 bits
    bitMapSUT.SetBit(1);
    bitMapSUT.SetBit(2);
    bitMapSUT.SetBit(15);

    //Then: 3 bits should be set
    EXPECT_EQ(bitMapSUT.GetNumBitsSet(), 3);

    //When: clear two bits
    bitMapSUT.ClearBits(1, 2);

    //Then: only 1 bit should be set
    EXPECT_EQ(bitMapSUT.GetNumBitsSet(), 1);
    EXPECT_FALSE(bitMapSUT.IsSetBit(1));
    EXPECT_FALSE(bitMapSUT.IsSetBit(2));
    EXPECT_TRUE(bitMapSUT.IsSetBit(15));
}

TEST(BitMapMutex, ResetBitmap_)
{
    //Given
    BitMapMutex bitMapSUT(16);

    //When: set 3 bits
    bitMapSUT.SetBit(1);
    bitMapSUT.SetBit(2);
    bitMapSUT.SetBit(15);

    //Then: 3 bits should be set
    EXPECT_EQ(bitMapSUT.GetNumBitsSet(), 3);

    //When: reset all bits
    bitMapSUT.ResetBitmap();

    //Then: no bit should be set
    for (int i = 0; i < 16; i++)
    {
        EXPECT_FALSE(bitMapSUT.IsSetBit(i));
    }
}

TEST(BitMapMutex, IsSetBit_)
{
}

TEST(BitMapMutex, FlipBit_)
{
    // Given: Set bit 7
    BitMapMutex bitMapSUT(16);
    bitMapSUT.SetBit(7);
    ASSERT_TRUE(bitMapSUT.IsSetBit(7));

    // When: Flip bit 7
    bitMapSUT.FlipBit(7);

    // Then: Bit 7 should be cleared
    EXPECT_FALSE(bitMapSUT.IsSetBit(7));

    // When: Flit bit 7 again
    bitMapSUT.FlipBit(7);

    // Then: Bit 7 should be set
    EXPECT_TRUE(bitMapSUT.IsSetBit(7));
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
    // Given: 7 bits of Bitmap are set
    BitMapMutex bitMapSUT(16);

    for (int i = 0; i <= 7; ++i)
    {
        bitMapSUT.SetBit(i);
    }

    // When: Execute PrintMap
    bitMapSUT.PrintMap();

    // Then: Do nothing
}

TEST(BitMapMutex, GetNumBitsSet_)
{
    // Given
    BitMapMutex bitMapSUT(16);

    //When: Set 3 bits
    bitMapSUT.SetBit(1);
    bitMapSUT.SetBit(2);
    bitMapSUT.SetBit(15);

    //Then: Num of bits set should be 3
    EXPECT_EQ(bitMapSUT.GetNumBitsSet(), 3);

    //Then: Num of bits set between bit 2 and 14 should be 3
    EXPECT_EQ(bitMapSUT.GetNumBitsSet(2, 14), 1);
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
