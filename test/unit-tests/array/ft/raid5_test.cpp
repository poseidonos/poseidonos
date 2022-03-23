#include "src/array/ft/raid5.h"

#include <gtest/gtest.h>

#include "src/array_models/dto/partition_physical_size.h"
#include "src/include/array_config.h"
#include "test/unit-tests/resource_manager/buffer_pool_mock.h"
#include "test/unit-tests/resource_manager/memory_manager_mock.h"
#include "test/unit-tests/cpu_affinity/affinity_manager_mock.h"
#include "test/unit-tests/utils/mock_builder.h"

using ::testing::Return;
using ::testing::NiceMock;

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
        .startLba = 0/* not interesting */,
        .lastLba = 0/* not interesting */,
        .blksPerChunk = 1234,
        .chunksPerStripe = 4567,
        .stripesPerSegment = 0/* not interesting */,
        .totalSegments = 0/* not interesting */};
    MockAffinityManager mockAffMgr = BuildDefaultAffinityManagerMock();

    // When
    Raid5 raid5(&physicalSize);
}

TEST(Raid5, Raid5_testIfTranslateCalculatesDestinationOffsetProperly)
{
    // Given
    const PartitionPhysicalSize physicalSize{
        .startLba = 0/* not interesting */,
        .lastLba = 0/* not interesting */,
        .blksPerChunk = 27,
        .chunksPerStripe = 10,
        .stripesPerSegment = 0/* not interesting */,
        .totalSegments = 0/* not interesting */};
    uint32_t STRIPE_ID = 13;
    uint32_t OFFSET = 400;

    LogicalEntry src;
    src.addr.stripeId = STRIPE_ID;
    src.addr.offset = OFFSET;
    src.blkCnt = 1;

    MockAffinityManager mockAffMgr = BuildDefaultAffinityManagerMock();
    Raid5 raid5(&physicalSize);
    list<FtEntry> dest;

    // When
    dest = raid5.Translate(src);

    // Then
    int expectedChunkIndex = OFFSET / physicalSize.blksPerChunk;        // 400 / 27 == 14
    int expectedParityIndex = STRIPE_ID % physicalSize.chunksPerStripe; // 13 % 10 == 3
    int expectedDestOffset = OFFSET + physicalSize.blksPerChunk;        // adjusted since expected chunk index is larger
    ASSERT_EQ(expectedDestOffset, dest.front().addr.offset);
}

TEST(Raid5, MakeParity_testIfParityBufferIsProperlyCalculated)
{
    // Given
    const PartitionPhysicalSize physicalSize{
        .startLba = 0/* not interesting */,
        .lastLba = 0/* not interesting */,
        .blksPerChunk = 4,
        .chunksPerStripe = 4,
        .stripesPerSegment = 0/* not interesting */,
        .totalSegments = 0/* not interesting */};
    MockAffinityManager mockAffMgr = BuildDefaultAffinityManagerMock();
    EXPECT_CALL(mockAffMgr, GetNumaCount).WillRepeatedly(Return(1));
    EXPECT_CALL(mockAffMgr, GetNumaIdFromCurrentThread).WillRepeatedly(Return(0));
    BufferInfo info = {
        .owner = "Raid5Test_MakeParity",
        .size = 65535,
        .count = 1
    };
    char mem[65535];
    MockBufferPool mockBufferPool(info, 0, nullptr);
    EXPECT_CALL(mockBufferPool, TryGetBuffer).WillOnce(Return(&mem));
    MockMemoryManager mockMemoryManager;
    EXPECT_CALL(mockMemoryManager, CreateBufferPool).WillOnce(Return(&mockBufferPool));
    Raid5 raid5(&physicalSize);
    raid5.AllocParityPools(physicalSize.blksPerChunk * 2, &mockAffMgr, &mockMemoryManager);

    std::list<BufferEntry> buffers;
    int NUM_BLOCKS = physicalSize.blksPerChunk;
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
    int actual = raid5.MakeParity(dest, src);

    // Then: XORing 3 data + 1 parity should produce zero buffer
    ASSERT_EQ(0, actual);
    ASSERT_EQ(1, dest.size());
    ASSERT_EQ(1, dest.front().buffers.size()); // one parity
    const int bufSize = ArrayConfig::BLOCK_SIZE_BYTE * NUM_BLOCKS;
    dest.front().buffers.push_back(be1);
    dest.front().buffers.push_back(be2);
    dest.front().buffers.push_back(be3);
    verifyIfXORProducesZeroBuffer(dest.front().buffers, bufSize);
}

TEST(Raid5, GetRebuildGroup_testIfRebuildGroupDoesNotContainTargetFtBlockAddr)
{
    // Given
    const PartitionPhysicalSize physicalSize{
        .startLba = 0/* not interesting */,
        .lastLba = 0/* not interesting */,
        .blksPerChunk = 27,
        .chunksPerStripe = 100,
        .stripesPerSegment = 0/* not interesting */,
        .totalSegments = 0/* not interesting */};
    MockAffinityManager mockAffMgr = BuildDefaultAffinityManagerMock();
    Raid5 raid5(&physicalSize);
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
        .startLba = 0/* not interesting */,
        .lastLba = 0/* not interesting */,
        .blksPerChunk = 27,
        .chunksPerStripe = 100,
        .stripesPerSegment = 0/* not interesting */,
        .totalSegments = 0/* not interesting */};
    MockAffinityManager mockAffMgr = BuildDefaultAffinityManagerMock();
    Raid5 raid5(&physicalSize);

    // When & Then
    ASSERT_EQ(27, raid5.GetSizeInfo()->blksPerChunk);
    ASSERT_EQ(100, raid5.GetSizeInfo()->chunksPerStripe);
}

TEST(Raid5, AllocResetParityPools_testIfPoolCreateAndDeletedProperlyWithTwoNuma)
{
    // Given
    const PartitionPhysicalSize physicalSize{
        .startLba = 0, /* not interesting */
        .lastLba = 0/* not interesting */,
        .blksPerChunk = 27,
        .chunksPerStripe = 100,
        .stripesPerSegment = 0, /* not interesting */
        .totalSegments = 0}; /* not interesting */
    MockAffinityManager mockAffinityManager;
    MockMemoryManager mockMemoryManager;
    BufferInfo info = {
        .owner = "Raid5Test",
        .size = 4096,
        .count = 1,
    };
    unsigned int numaCount = 2;
    MockBufferPool mockBufferPool(info, 0, nullptr);
    EXPECT_CALL(mockAffinityManager, GetNumaCount).WillOnce(Return(numaCount));
    EXPECT_CALL(mockMemoryManager, CreateBufferPool)
        .Times(numaCount)
        .WillRepeatedly(Return(&mockBufferPool));
    EXPECT_CALL(mockMemoryManager, DeleteBufferPool).Times(numaCount);
    Raid5 raid5(&physicalSize);
    
    // When : Alloc Pools
    bool ret = raid5.AllocParityPools(0, &mockAffinityManager, &mockMemoryManager);

    // Then
    EXPECT_TRUE(ret);

    // When : Reset Pools
    raid5.ClearParityPools();

    // Then
    EXPECT_EQ(raid5.GetParityPoolSize(), 0);
}

TEST(Raid5, GetRaidState_testIfRaid5IsFailure)
{
    // Given
    vector<ArrayDeviceState> devs;
    devs.push_back(ArrayDeviceState::NORMAL);
    devs.push_back(ArrayDeviceState::FAULT);
    devs.push_back(ArrayDeviceState::FAULT);
    devs.push_back(ArrayDeviceState::NORMAL);

    const PartitionPhysicalSize physicalSize{
        .startLba = 0/* not interesting */,
        .lastLba = 0/* not interesting */,
        .blksPerChunk = 1234,
        .chunksPerStripe = 4567,
        .stripesPerSegment = 0/* not interesting */,
        .totalSegments = 0/* not interesting */};
    MockAffinityManager mockAffMgr = BuildDefaultAffinityManagerMock();
    Raid5 raid5(&physicalSize);

    // When
    RaidState actual = raid5.GetRaidState(devs);

    // Then
    ASSERT_EQ(RaidState::FAILURE, actual);
}

TEST(Raid5, GetRaidState_testIfRaid5IsDegraded)
{
    // Given
    vector<ArrayDeviceState> devs;
    devs.push_back(ArrayDeviceState::NORMAL);
    devs.push_back(ArrayDeviceState::FAULT);
    devs.push_back(ArrayDeviceState::NORMAL);
    devs.push_back(ArrayDeviceState::NORMAL);

    const PartitionPhysicalSize physicalSize{
        .startLba = 0/* not interesting */,
        .lastLba = 0/* not interesting */,
        .blksPerChunk = 1234,
        .chunksPerStripe = 4567,
        .stripesPerSegment = 0/* not interesting */,
        .totalSegments = 0/* not interesting */};
    MockAffinityManager mockAffMgr = BuildDefaultAffinityManagerMock();
    Raid5 raid5(&physicalSize);

    // When

    // When
    RaidState actual = raid5.GetRaidState(devs);

    // Then
    ASSERT_EQ(RaidState::DEGRADED, actual);
}

TEST(Raid5, GetRaidState_testIfRaid5IsNormal)
{
    // Given
    vector<ArrayDeviceState> devs;
    devs.push_back(ArrayDeviceState::NORMAL);
    devs.push_back(ArrayDeviceState::NORMAL);
    devs.push_back(ArrayDeviceState::NORMAL);
    devs.push_back(ArrayDeviceState::NORMAL);

    const PartitionPhysicalSize physicalSize{
        .startLba = 0/* not interesting */,
        .lastLba = 0/* not interesting */,
        .blksPerChunk = 1234,
        .chunksPerStripe = 4567,
        .stripesPerSegment = 0/* not interesting */,
        .totalSegments = 0/* not interesting */};
    MockAffinityManager mockAffMgr = BuildDefaultAffinityManagerMock();
    Raid5 raid5(&physicalSize);

    // When
    RaidState actual = raid5.GetRaidState(devs);

    // Then
    ASSERT_EQ(RaidState::NORMAL, actual);
}
} // namespace pos
