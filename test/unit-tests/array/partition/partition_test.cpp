#include "src/array/partition/partition.h"
#include "src/array/partition/stripe_partition.h"
#include "src/array/partition/nvm_partition.h"
#include "src/array/ft/raid5.h"
#include "test/unit-tests/array/ft/raid5_mock.h"
#include "src/cpu_affinity/cpu_set_generator.h"
#include "test/unit-tests/cpu_affinity/affinity_manager_mock.h"

#include <gtest/gtest.h>

namespace pos
{
TEST(Partition, Partition_testConstructorAndDestructor)
{
    // Given
    string mockArrayName = "mockArray";
    uint32_t mockArrayIndex = 0;
    PartitionType mockPartitionType = PartitionType::USER_DATA;
    const PartitionPhysicalSize mockPhysicalSize{
        .startLba = 0,
        .blksPerChunk = 10,
        .chunksPerStripe = 5,
        .stripesPerSegment = 20,
        .totalSegments = 100};
    vector<ArrayDevice*> devs;
    CpuSetArray cpuSetArray;
    MockAffinityManager mockAffMgr(8, cpuSetArray);
    Method* mockMethod = new Raid5(&mockPhysicalSize, 10, &mockAffMgr, nullptr);
    // When
    Partition* partition = new StripePartition(mockArrayName, mockArrayIndex, mockPartitionType, mockPhysicalSize, devs, mockMethod);
    // Then
    delete partition;
}

TEST(Partition, GetRaidState_testConstructorAndDestructor)
{
    // Given
    string mockArrayName = "mockArray";
    uint32_t mockArrayIndex = 0;
    PartitionType mockPartitionType = PartitionType::META_NVM;
    const PartitionPhysicalSize mockPhysicalSize{
        .startLba = 0,
        .blksPerChunk = 100,
        .chunksPerStripe = 10,
        .stripesPerSegment = 5,
        .totalSegments = 2};
    vector<ArrayDevice*> devs;
    CpuSetArray cpuSetArray;
    Partition* partition = new NvmPartition(mockArrayName, mockArrayIndex, mockPartitionType, mockPhysicalSize, devs);
    // When
    RaidState ret = partition->GetRaidState();

    // Then
    ASSERT_EQ(RaidState::NORMAL, ret);
    delete partition;
}

TEST(Partition, Create_)
{
}

TEST(Partition, GetLogicalSize_)
{
}

TEST(Partition, GetPhysicalSize_)
{
}

TEST(Partition, IsValidLba_)
{
}

TEST(Partition, FindDevice_)
{
}

TEST(Partition, GetMethod_)
{
}

TEST(Partition, Format_)
{
}

TEST(Partition, _IsValidAddress_)
{
}

TEST(Partition, _IsValidEntry_)
{
}

} // namespace pos
