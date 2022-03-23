#include "src/array/ft/raid0.h"

#include <gtest/gtest.h>

#include <cmath>

#include "src/array_models/dto/partition_physical_size.h"
#include "src/include/address_type.h"

namespace pos
{
TEST(Raid0, Raid0_testWithHeapAllocation)
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
    Raid0* raid0 = new Raid0(&physicalSize);

    // Then
    ASSERT_NE(nullptr, raid0);
    delete raid0;
}

TEST(Raid0, Translate_ifDestinationIsFilledWithStripeIdAndOffset)
{
    // Given
    const PartitionPhysicalSize physicalSize{
        .startLba = 0,
        .lastLba = 0/* not interesting */,
        .blksPerChunk = 10,
        .chunksPerStripe = 5,
        .stripesPerSegment = 20,
        .totalSegments = 100};
    Raid0 raid0(&physicalSize);

    StripeId STRIPE_ID = 1234;
    uint32_t OFFSET = 4567;

    LogicalEntry src;
    src.addr.stripeId = STRIPE_ID;
    src.addr.offset = OFFSET;
    src.blkCnt = 1;
    list<FtEntry> dest;

    // When
    dest = raid0.Translate(src);

    // Then
    ASSERT_EQ(STRIPE_ID, dest.front().addr.stripeId);
    ASSERT_EQ(OFFSET, dest.front().addr.offset);
}

TEST(Raid0, GetRaidState_testIfRaid0IsFailure)
{
    // Given
    const PartitionPhysicalSize physicalSize{
        .startLba = 0,
        .lastLba = 0/* not interesting */,
        .blksPerChunk = 10,
        .chunksPerStripe = 4,
        .stripesPerSegment = 20,
        .totalSegments = 100};
    Raid0 raid0(&physicalSize);
    vector<ArrayDeviceState> devs;
    devs.push_back(ArrayDeviceState::NORMAL);
    devs.push_back(ArrayDeviceState::FAULT);
    devs.push_back(ArrayDeviceState::NORMAL);
    devs.push_back(ArrayDeviceState::NORMAL);

    // When
    RaidState actual = raid0.GetRaidState(devs);

    // Then
    ASSERT_EQ(RaidState::FAILURE, actual);
}

TEST(Raid0, GetRaidState_testIfRaid0IsNormal)
{
    // Given
    const PartitionPhysicalSize physicalSize{
        .startLba = 0,
        .lastLba = 0/* not interesting */,
        .blksPerChunk = 10,
        .chunksPerStripe = 4,
        .stripesPerSegment = 20,
        .totalSegments = 100};
    Raid0 raid0(&physicalSize);
    vector<ArrayDeviceState> devs;
    devs.push_back(ArrayDeviceState::NORMAL);
    devs.push_back(ArrayDeviceState::NORMAL);
    devs.push_back(ArrayDeviceState::NORMAL);
    devs.push_back(ArrayDeviceState::NORMAL);

    // When
    RaidState actual = raid0.GetRaidState(devs);

    // Then
    ASSERT_EQ(RaidState::NORMAL, actual);
}
} // namespace pos
