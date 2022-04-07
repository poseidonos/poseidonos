#include <gtest/gtest.h>
#include "src/allocator/context_manager/block_allocation_status.h"

namespace pos
{
TEST(BlockAllocationStatus, BlockAllocationStatus_testIfConstructedSuccessfully)
{
    BlockAllocationStatus status;
    status.PermitBlockAllocation();

    for (int volumeId = 0; volumeId < MAX_VOLUME_COUNT; volumeId++)
    {
        EXPECT_EQ(status.IsBlockAllocationProhibited(volumeId), false);
        EXPECT_EQ(status.IsUserBlockAllocationProhibited(volumeId), false);
    }
}

TEST(BlockAllocationStatus, PermitUserBlockAllocation_testIfUserBlockAllocationIsPermited)
{
    BlockAllocationStatus status;

    status.ProhibitUserBlockAllocation();
    for (int volumeId = 0; volumeId < MAX_VOLUME_COUNT; volumeId++)
    {
        EXPECT_EQ(status.IsUserBlockAllocationProhibited(volumeId), true);
    }

    status.PermitUserBlockAllocation();
    for (int volumeId = 0; volumeId < MAX_VOLUME_COUNT; volumeId++)
    {
        EXPECT_EQ(status.IsUserBlockAllocationProhibited(volumeId), false);
    }
}

TEST(BlockAllocationStatus, PermitBlockAllocation_testIfBlockAllocationIsPermited)
{
    BlockAllocationStatus status;

    status.ProhibitBlockAllocation();
    for (int volumeId = 0; volumeId < MAX_VOLUME_COUNT; volumeId++)
    {
        EXPECT_EQ(status.IsBlockAllocationProhibited(volumeId), true);
        EXPECT_EQ(status.IsUserBlockAllocationProhibited(volumeId), true);
    }

    int testVolumeId = 20;
    status.PermitBlockAllocation(testVolumeId);
    for (int volumeId = 0; volumeId < MAX_VOLUME_COUNT; volumeId++)
    {
        if (volumeId == testVolumeId)
        {
            EXPECT_EQ(status.IsBlockAllocationProhibited(volumeId), false);
        }
        else
        {
            EXPECT_EQ(status.IsBlockAllocationProhibited(volumeId), true);
        }
    }
}

TEST(BlockAllocationStatus, TryProhibitBlockAllocation_testIfBlockAllocationProhibited)
{
    BlockAllocationStatus status;

    int testVolumeId = 30;

    bool ret = status.TryProhibitBlockAllocation(testVolumeId);
    EXPECT_EQ(ret, true);

    for (int volumeId = 0; volumeId < MAX_VOLUME_COUNT; volumeId++)
    {
        if (volumeId == testVolumeId)
        {
            EXPECT_EQ(status.IsBlockAllocationProhibited(volumeId), true);
        }
        else
        {
            EXPECT_EQ(status.IsBlockAllocationProhibited(volumeId), false);
        }
    }

    ret = status.TryProhibitBlockAllocation(testVolumeId);
    EXPECT_EQ(ret, false);
    EXPECT_EQ(status.IsBlockAllocationProhibited(testVolumeId), true);

    status.PermitBlockAllocation(testVolumeId);
    for (int volumeId = 0; volumeId < MAX_VOLUME_COUNT; volumeId++)
    {
        EXPECT_EQ(status.IsBlockAllocationProhibited(volumeId), false);
    }
}

TEST(BlockAllocationStatus, Lock_testIfAllRwlocksWorkWell)
{
    BlockAllocationStatus status;
    status.Lock();
    status.Unlock();
}

TEST(BlockAllocationStatus, TryRdLockWithVolumeId_testIfTheRwlockWorksWell)
{
    BlockAllocationStatus status;
    const int volumeId = 0;

    EXPECT_TRUE(status.TryRdLock(volumeId));
    EXPECT_TRUE(status.TryRdLock(volumeId));
    EXPECT_TRUE(status.Unlock(volumeId));
}
}  // namespace pos
