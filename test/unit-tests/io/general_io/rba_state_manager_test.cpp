#include "src/io/general_io/rba_state_manager.h"

#include <gtest/gtest.h>

namespace pos
{
TEST(RBAStateManager, RBAStateManager_)
{
}

TEST(RBAStateManager, CreateRBAState_)
{
}

TEST(RBAStateManager, DeleteRBAState_)
{
}

TEST(RBAStateManager, AcquireOwnershipRbaList_DuplicatedRba)
{
    RBAStateManager rbaStateManager("POSArray", nullptr, nullptr);
    rbaStateManager.CreateRBAState(0, 100);

    VolumeIo::RbaList sectorRbaList;
    for (uint64_t i = 0; i < 10; i++)
    {
        RbaAndSize rbaAndSize = {i*8, 1};
        sectorRbaList.push_back(rbaAndSize);
    }
    RbaAndSize rbaAndSize = {16, 1};
    sectorRbaList.push_back(rbaAndSize);

    bool expected = true;
    bool actual = rbaStateManager.AcquireOwnershipRbaList(0, sectorRbaList);
    ASSERT_EQ(expected, actual);

    for (uint64_t i = 0; i < 10; i++)
    {
        sectorRbaList.clear();
        RbaAndSize rbaAndSize = {i*8, 1};
        sectorRbaList.push_back(rbaAndSize);
        expected = false;
        actual = rbaStateManager.AcquireOwnershipRbaList(0, sectorRbaList);
        ASSERT_EQ(expected, actual);
    }

    sectorRbaList.clear();
    for (uint64_t i = 0; i < 10; i++)
    {
        RbaAndSize rbaAndSize = {i*8, 1};
        sectorRbaList.push_back(rbaAndSize);
    }
    rbaAndSize = {16, 1};
    sectorRbaList.push_back(rbaAndSize);
    rbaStateManager.ReleaseOwnershipRbaList(0, sectorRbaList);

    for (uint64_t i = 0; i < 10; i++)
    {
        sectorRbaList.clear();
        RbaAndSize rbaAndSize = {i*8, 1};
        sectorRbaList.push_back(rbaAndSize);
        expected = true;
        actual = rbaStateManager.AcquireOwnershipRbaList(0, sectorRbaList);
        ASSERT_EQ(expected, actual);
    }
}

TEST(RBAStateManager, ReleaseOwnershipRbaList_)
{
}


TEST(RBAStateManager, AcquireOwnership_)
{
}

TEST(RBAStateManager, ReleaseOwnership_)
{
}

TEST(RBAStateManager, BulkAcquireOwnership_)
{
}

TEST(RBAStateManager, BulkReleaseOwnership_)
{
}

TEST(RBAStateManager, VolumeCreated_)
{
}

TEST(RBAStateManager, VolumeDeleted_)
{
}

TEST(RBAStateManager, VolumeMounted_)
{
}

TEST(RBAStateManager, VolumeUnmounted_)
{
}

TEST(RBAStateManager, VolumeLoaded_)
{
}

TEST(RBAStateManager, VolumeUpdated_)
{
}

TEST(RBAStateManager, VolumeDetached_)
{
}

} // namespace pos
