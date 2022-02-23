#include "src/array/service/io_recover/i_recover.h"
#include "src/array/partition/stripe_partition.h"
#include "test/unit-tests/array/ft/raid1_mock.h"

#include <gtest/gtest.h>

namespace pos
{
TEST(IRecover, GetRecoverMethod_)
{
    // Given
    PartitionPhysicalSize physicalSize{
        .startLba = 0 /* not interesting */,
        .blksPerChunk = 10,
        .chunksPerStripe = 0 /* not interesting */,
        .stripesPerSegment = 20,
        .totalSegments = 30};
    MockRaid1* mockRaid1 = new MockRaid1(&physicalSize);
    vector<ArrayDevice*> devs;

    // When
    StripePartition* sPartition = new StripePartition("mock-array", 0, PartitionType::USER_DATA, physicalSize, devs, mockRaid1);
    IRecover* irecover = sPartition;

    // Then
    delete irecover;
}

} // namespace pos
