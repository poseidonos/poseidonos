#include "src/array/ft/raid5.h"

#include <gtest/gtest.h>

#include "src/array_models/dto/partition_physical_size.h"
#include "src/include/array_config.h"

namespace pos
{
static BufferEntry
generateRandomBufferEntry(int numBlocks, bool isParity)
{
    int bufSize = ArrayConfig::BLOCK_SIZE_BYTE * numBlocks;
    char* buffer = new char[bufSize];
    unsigned int seed = time(NULL);
    for (int i = 0; i < bufSize; i += 1)
    {
        buffer[i] = rand_r(&seed);
    }
    return BufferEntry(buffer, numBlocks, isParity);
}

static void
verifyIfXORProducesZeroBuffer(std::list<BufferEntry> buffers, int bufSize)
{
    char* xorBuffer = new char[bufSize];
    for (int i = 0; i < bufSize; i += 1)
    {
        xorBuffer[i] = 0;
    }

    // If there is a valid parity buffer, XORing all those buffers should produce a buffer filled with zeroes
    auto itor = buffers.begin();
    while (itor != buffers.end())
    {
        BufferEntry be = *itor;
        char* bufPtr = static_cast<char*>(be.GetBufferPtr());
        for (int i = 0; i < bufSize; i += 1)
        {
            xorBuffer[i] ^= bufPtr[i];
        }
        itor++;
    }

    // verify whether all bytes are zeroed out
    for (int i = 0; i < bufSize; i += 1)
    {
        ASSERT_EQ(0, xorBuffer[i]);
    }

    delete xorBuffer;
}
TEST(Raid5, Raid5_testIfConstructorIsInvoked)
{
    // Given
    const PartitionPhysicalSize physicalSize{
        .blksPerChunk = 1234,
        .chunksPerStripe = 4567};

    // When
    Raid5 raid5(&physicalSize, 10 /* meaningless in this UT file */);
}

TEST(Raid5, Raid5_testIfTranslateCalculatesDestinationOffsetProperly)
{
    // Given
    const PartitionPhysicalSize physicalSize{
        .blksPerChunk = 27,
        .chunksPerStripe = 10};
    uint32_t STRIPE_ID = 13;
    uint32_t OFFSET = 400;

    const LogicalBlkAddr src{
        .stripeId = STRIPE_ID,
        .offset = OFFSET};

    Raid5 raid5(&physicalSize, 10);
    FtBlkAddr dest;

    // When
    int actual = raid5.Translate(dest, src);

    // Then
    ASSERT_EQ(0, actual);
    int expectedChunkIndex = OFFSET / physicalSize.blksPerChunk;        // 400 / 27 == 14
    int expectedParityIndex = STRIPE_ID % physicalSize.chunksPerStripe; // 13 % 10 == 3
    int expectedDestOffset = OFFSET + physicalSize.blksPerChunk;        // adjusted since expected chunk index is larger
    ASSERT_EQ(expectedDestOffset, dest.offset);
}

TEST(Raid5, Convert_testIfParityBufferIsProperlyCalculated)
{
    // Given
    const PartitionPhysicalSize physicalSize{
        .blksPerChunk = 4,
        .chunksPerStripe = 4};
    Raid5 raid5(&physicalSize, physicalSize.blksPerChunk * 2 /* just to be large enough */);

    std::list<BufferEntry> buffers;
    int NUM_BLOCKS = 2;
    BufferEntry be1 = generateRandomBufferEntry(NUM_BLOCKS, false);
    BufferEntry be2 = generateRandomBufferEntry(NUM_BLOCKS, false);
    BufferEntry be3 = generateRandomBufferEntry(NUM_BLOCKS, false);
    buffers.push_back(be1);
    buffers.push_back(be2);
    buffers.push_back(be3);

    LogicalBlkAddr lBlkAddr{
        .stripeId = 1234,
        .offset = 0};
    const LogicalWriteEntry& src{
        .addr = lBlkAddr,
        .blkCnt = 3,
        .buffers = &buffers};

    list<FtWriteEntry> dest;

    // When
    int actual = raid5.Convert(dest, src);

    // Then: XORing 3 data + 1 parity should produce zero buffer
    ASSERT_EQ(0, actual);
    ASSERT_EQ(1, dest.size());
    ASSERT_EQ(4, dest.front().buffers.size()); // 3 + 1
    const int bufSize = ArrayConfig::BLOCK_SIZE_BYTE * NUM_BLOCKS;
    verifyIfXORProducesZeroBuffer(dest.front().buffers, bufSize);
}

TEST(Raid5, GetRebuildGroup_testIfRebuildGroupDoesNotContainTargetFtBlockAddr)
{
    // Given
    const PartitionPhysicalSize physicalSize{
        .blksPerChunk = 27,
        .chunksPerStripe = 100};
    Raid5 raid5(&physicalSize, 10);
    StripeId STRIPE_ID = 1234;
    BlkOffset BLK_OFFSET = 400;

    FtBlkAddr fba{
        .stripeId = STRIPE_ID,
        .offset = BLK_OFFSET};

    // When
    list<FtBlkAddr> actual = raid5.GetRebuildGroup(fba);

    // Then
    int expectedChunkIndex = BLK_OFFSET / physicalSize.blksPerChunk; // 400 / 27 == 14
    int expectedRebuildGroupSize = physicalSize.chunksPerStripe - 1;
    ASSERT_EQ(expectedRebuildGroupSize, actual.size());
}

TEST(Raid5, Getters_testIfGettersAreInvoked)
{
    // Given
    const PartitionPhysicalSize physicalSize{
        .blksPerChunk = 27,
        .chunksPerStripe = 100};
    Raid5 raid5(&physicalSize, 10);

    // When & Then
    ASSERT_EQ(RaidTypeEnum::RAID5, raid5.GetRaidType());
    ASSERT_EQ(27, raid5.GetSizeInfo()->blksPerChunk);
    ASSERT_EQ(100, raid5.GetSizeInfo()->chunksPerStripe);
}

} // namespace pos
