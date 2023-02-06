#include <gtest/gtest.h>
#include "src/array/build/array_build_info.h"
#include "src/array/partition/stripe_partition.h"
 
namespace pos
{

TEST(ArrayBuildInfo, Dispose_testIfDevsAndPartitionsAreCleared)
{
    // Given
    ArrayBuildInfo buildInfo;
    ArrayDevice* d1 = new ArrayDevice(nullptr);
    ArrayDevice* d2 = new ArrayDevice(nullptr);
    buildInfo.devices = { d1, d2 };
    StripePartition* p1 = new StripePartition
        (PartitionType::USER_DATA, buildInfo.devices, RaidTypeEnum::RAID0); // not interesting
    StripePartition* p2 = new StripePartition(
        PartitionType::META_SSD, buildInfo.devices, RaidTypeEnum::RAID0); // not interesting
    buildInfo.partitions = { p1, p2 };

    // When
    buildInfo.Dispose();

    // Then
    ASSERT_EQ(0, buildInfo.devices.size());
    ASSERT_EQ(0, buildInfo.partitions.size());
}

}  // namespace pos
