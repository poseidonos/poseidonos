#include "src/array/ft/raid_none.h"

#include <gtest/gtest.h>

#include <cmath>

#include "src/array_models/dto/partition_physical_size.h"
#include "src/include/address_type.h"

namespace pos
{
TEST(RaidNone, RaidNone_testWithHeapAllocation)
{
    // Given a set of constructor params
    const PartitionPhysicalSize physicalSize{
        .startLba = 0,
        .lastLba = 0/* not interesting */,
        .blksPerChunk = 10,
        .chunksPerStripe = 5,
        .stripesPerSegment = 20,
        .totalSegments = 100};

    // When
    RaidNone* raidnone = new RaidNone(&physicalSize);

    // Then
    ASSERT_NE(nullptr, raidnone);
    delete raidnone;
}

TEST(RaidNone, Translate_ifDestinationIsFilledWithStripeIdAndOffset)
{
    // Given
    const PartitionPhysicalSize physicalSize{
        .startLba = 0,
        .lastLba = 0/* not interesting */,
        .blksPerChunk = 10,
        .chunksPerStripe = 5,
        .stripesPerSegment = 20,
        .totalSegments = 100};
    RaidNone raidnone(&physicalSize);

    StripeId STRIPE_ID = 1234;
    uint32_t OFFSET = 4567;

    LogicalEntry src;
    src.addr.stripeId = STRIPE_ID;
    src.addr.offset = OFFSET;
    src.blkCnt = 1;
    list<FtEntry> dest;

    // When
    dest = raidnone.Translate(src);

    // Then
    ASSERT_EQ(STRIPE_ID, dest.front().addr.stripeId);
    ASSERT_EQ(OFFSET, dest.front().addr.offset);
}

TEST(RaidNone, GetRaidState_testIfRaidNoneIsFailure)
{
    // Given
    const PartitionPhysicalSize physicalSize{
        .startLba = 0,
        .lastLba = 0/* not interesting */,
        .blksPerChunk = 10,
        .chunksPerStripe = 4,
        .stripesPerSegment = 20,
        .totalSegments = 100};
    RaidNone raidnone(&physicalSize);
    vector<ArrayDeviceState> devs;
    devs.push_back(ArrayDeviceState::FAULT);

    // When
    RaidState actual = raidnone.GetRaidState(devs);

    // Then
    ASSERT_EQ(RaidState::FAILURE, actual);
}

TEST(RaidNone, GetRaidState_testIfRaidNoneIsNormal)
{
    // Given
    const PartitionPhysicalSize physicalSize{
        .startLba = 0,
        .lastLba = 0/* not interesting */,
        .blksPerChunk = 10,
        .chunksPerStripe = 4,
        .stripesPerSegment = 20,
        .totalSegments = 100};
    RaidNone raidnone(&physicalSize);
    vector<ArrayDeviceState> devs;
    devs.push_back(ArrayDeviceState::NORMAL);

    // When
    RaidState actual = raidnone.GetRaidState(devs);

    // Then
    ASSERT_EQ(RaidState::NORMAL, actual);
}
} // namespace pos
