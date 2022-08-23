/*
 *   BSD LICENSE
 *   Copyright (c) 2021 Samsung Electronics Corporation
 *   All rights reserved.
 *
 *   Redistribution and use in source and binary forms, with or without
 *   modification, are permitted provided that the following conditions
 *   are met:
 *
 *     * Redistributions of source code must retain the above copyright
 *       notice, this list of conditions and the following disclaimer.
 *     * Redistributions in binary form must reproduce the above copyright
 *       notice, this list of conditions and the following disclaimer in
 *       the documentation and/or other materials provided with the
 *       distribution.
 *     * Neither the name of Samsung Electronics Corporation nor the names of
 *       its contributors may be used to endorse or promote products derived
 *       from this software without specific prior written permission.
 *
 *   THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 *   "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 *   LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
 *   A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
 *   OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
 *   SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
 *   LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 *   DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
 *   THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 *   (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 *   OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include "gtest/gtest.h"
#include "src/bio/volume_io.h"
#include "src/include/pos_event_id.hpp"
#include "src/io/general_io/rba_state_service.h"
#include "src/sys_event/volume_event.h"

using namespace pos;
using namespace std;
using ::testing::_;
using ::testing::NiceMock;
using ::testing::Return;
using EventIdWithLevel = std::pair<POS_EVENT_ID, EventLevel>;

namespace pos
{
class RBAStateManagerFixture : public testing::Test
{
public:
    virtual void
    SetUp(void)
    {
        rbaStateManager = new RBAStateManager(arrayName, arrayId);
    }

    virtual void
    TearDown(void)
    {
        delete rbaStateManager;
    }

protected:
    RBAStateManager* rbaStateManager;
    std::string arrayName = "POSArray1";
    int arrayId{0};
    const uint32_t VOLUME_ID{8};
    const uint64_t RBA_AMOUNT{10000};
    const BlkAddr RBA{RBA_AMOUNT / 2};
};

TEST_F(RBAStateManagerFixture, RBAStateManager_Constructor_TwoArguments)
{
    //When: create RbaStateManager (heap)
    RBAStateManager rbaStateManager("POSArray2", arrayId);

    //Then: do nothing
}

TEST_F(RBAStateManagerFixture, CreateAndDeleteRBAStateTest)
{
    //When: create rba state with invalid size
    rbaStateManager->CreateRBAState(VOLUME_ID, 0);
    //Then: AcquireOwnership fails
    EXPECT_FALSE(rbaStateManager->BulkAcquireOwnership(VOLUME_ID, 0, 1));

    //When: create rba state with valid size
    rbaStateManager->CreateRBAState(VOLUME_ID, RBA_AMOUNT);
    //Then: AcquireOwnership succeeds
    EXPECT_TRUE(rbaStateManager->BulkAcquireOwnership(VOLUME_ID, 0, 1));

    //When: create rba state with different size on the existing volume
    rbaStateManager->CreateRBAState(VOLUME_ID, RBA_AMOUNT * 2);
    //Then: AcquireOwnership fails since no change has been made
    EXPECT_FALSE(rbaStateManager->BulkAcquireOwnership(VOLUME_ID, RBA_AMOUNT, 1));

    //When: delete rba state
    rbaStateManager->DeleteRBAState(VOLUME_ID);
    //Then: AcquireOwnership fails since RBA state has been deleted
    EXPECT_FALSE(rbaStateManager->BulkAcquireOwnership(VOLUME_ID, 1, 1));

    //When: delete rba state which does not exist
    rbaStateManager->DeleteRBAState(VOLUME_ID + 1);
    //Then: Do nothing
}

TEST_F(RBAStateManagerFixture, AcquireAndReleaseOwnershipRbaListTest)
{
    //Given: create rba state
    rbaStateManager->CreateRBAState(VOLUME_ID, RBA_AMOUNT);

    //When: acquire ownership with empty rba range
    VolumeIo::RbaList emptySectorRbas;
    //Then: returns success
    EXPECT_TRUE(rbaStateManager->AcquireOwnershipRbaList(VOLUME_ID, emptySectorRbas));

    //When: acquire ownership with invalid rba range
    VolumeIo::RbaList invalidSectorRbas{{0, ChangeBlockToSector(RBA_AMOUNT)},
        {ChangeBlockToSector(RBA_AMOUNT), ChangeBlockToByte(RBA_AMOUNT)}};
    //Then: returns failure
    EXPECT_FALSE(rbaStateManager->AcquireOwnershipRbaList(VOLUME_ID, invalidSectorRbas));

    //When: acquire ownership with valid rba range
    VolumeIo::RbaList validSectorRbas{{0, ChangeBlockToSector(RBA_AMOUNT / 2)},
        {ChangeBlockToSector(RBA_AMOUNT / 2), ChangeBlockToByte(RBA_AMOUNT / 2)}};
    //Then: owership is aquired
    EXPECT_TRUE(rbaStateManager->AcquireOwnershipRbaList(VOLUME_ID, validSectorRbas));

    //When: try to acquire ownership again
    //Then: returns failure
    EXPECT_FALSE(rbaStateManager->AcquireOwnershipRbaList(VOLUME_ID, validSectorRbas));
}

TEST_F(RBAStateManagerFixture, BulkAcquireOwnershipTest)
{
    //Given: create rba state
    rbaStateManager->CreateRBAState(VOLUME_ID, RBA_AMOUNT);

    //When: acquire ownership with invalid size
    //Then: returns failure
    EXPECT_FALSE(rbaStateManager->BulkAcquireOwnership(VOLUME_ID, 0, RBA_AMOUNT + 1));

    //When: acquire ownership with invalid start rba
    //Then: returns failure
    EXPECT_FALSE(rbaStateManager->BulkAcquireOwnership(VOLUME_ID, RBA_AMOUNT, 1));

    //When: acquire ownership with valid range
    //Then: owership is aquired
    EXPECT_TRUE(rbaStateManager->BulkAcquireOwnership(VOLUME_ID, 0, RBA_AMOUNT));

    //When: try to acquire ownership again
    //Then: returns failure
    EXPECT_FALSE(rbaStateManager->BulkAcquireOwnership(VOLUME_ID, 0, RBA_AMOUNT));

    //When: acquire ownership with invalid volume
    //Then: exception is thrown
    EXPECT_THROW(rbaStateManager->BulkAcquireOwnership(MAX_VOLUME_COUNT, 0, 1), EventIdWithLevel);
}

TEST_F(RBAStateManagerFixture, BulkReleaseOwnershipTest)
{
    //Given: create rba state
    rbaStateManager->CreateRBAState(VOLUME_ID, RBA_AMOUNT);

    //When: acquire ownership with valid range
    //Then: owership is aquired
    EXPECT_TRUE(rbaStateManager->BulkAcquireOwnership(VOLUME_ID, 0, RBA_AMOUNT));

    //When: release ownership with invalid range
    rbaStateManager->BulkReleaseOwnership(VOLUME_ID, 0, RBA_AMOUNT + 1);
    //When: try to acquire ownership again
    //Then: returns failure since the previous ownership has not been released yet
    EXPECT_FALSE(rbaStateManager->BulkAcquireOwnership(VOLUME_ID, 0, RBA_AMOUNT));

    //When: release ownership with invalid volume
    rbaStateManager->BulkReleaseOwnership(MAX_VOLUME_COUNT, 0, 1);
    //Then: do nothing

    //When: release ownership with valid range
    rbaStateManager->BulkReleaseOwnership(VOLUME_ID, 0, RBA_AMOUNT);
    //When: try to acquire ownership again
    //Then: returns success since the previous ownership has been released successfully
    EXPECT_TRUE(rbaStateManager->BulkAcquireOwnership(VOLUME_ID, 0, RBA_AMOUNT));
}

TEST_F(RBAStateManagerFixture, VolumeEventTest)
{
    VolumeEventBase volumeEventBase;
    rbaStateManager->SetVolumeBase(&volumeEventBase, VOLUME_ID, ChangeBlockToByte(RBA_AMOUNT), "", "", "", "");
    VolumeEventPerf volumeMountPerf;
    rbaStateManager->SetVolumePerf(&volumeMountPerf, 0, 0);
    VolumeArrayInfo volumeArrayInfo;
    rbaStateManager->SetVolumeArrayInfo(&volumeArrayInfo, arrayId, arrayName);

    //When: receive volume creation event
    EXPECT_TRUE(rbaStateManager->VolumeCreated(&volumeEventBase, &volumeMountPerf, &volumeArrayInfo));
    //When: try to acquire ownership
    //Then: returns success
    EXPECT_TRUE(rbaStateManager->BulkAcquireOwnership(VOLUME_ID, 0, RBA_AMOUNT));

    //When: receive volume mount event
    EXPECT_TRUE(rbaStateManager->VolumeMounted(&volumeEventBase, &volumeMountPerf, &volumeArrayInfo));
    //When: try to acquire ownership again
    //Then: returns failure since no change has been made
    EXPECT_FALSE(rbaStateManager->BulkAcquireOwnership(VOLUME_ID, 0, RBA_AMOUNT));

    //When: receive volume unmount event
    EXPECT_TRUE(rbaStateManager->VolumeUnmounted(&volumeEventBase, &volumeArrayInfo));
    //When: try to release ownership
    //Then: returns failure since the previous ownership is preserved even after volume became unmount
    EXPECT_FALSE(rbaStateManager->BulkAcquireOwnership(VOLUME_ID, 0, RBA_AMOUNT));

    //When: receive volume deletion event
    EXPECT_TRUE(rbaStateManager->VolumeDeleted(&volumeEventBase, &volumeArrayInfo));

    //When: receive volume loaded event
    EXPECT_TRUE(rbaStateManager->VolumeLoaded(&volumeEventBase, &volumeMountPerf, &volumeArrayInfo));
    //When: try to acquire ownership
    //Then: returns success
    EXPECT_TRUE(rbaStateManager->BulkAcquireOwnership(VOLUME_ID, 0, RBA_AMOUNT));

    //When: receive volume updated event
    EXPECT_TRUE(rbaStateManager->VolumeUpdated(&volumeEventBase, &volumeMountPerf, &volumeArrayInfo));
    //When: try to acquire ownership again
    //Then: returns failure since no change has been made
    EXPECT_FALSE(rbaStateManager->BulkAcquireOwnership(VOLUME_ID, 0, RBA_AMOUNT));

    //When: receive volume detached event
    rbaStateManager->VolumeDetached(vector<int>{0, 1}, &volumeArrayInfo);
    //When: try to release ownership
    //Then: returns failure since the previous ownership is preserved even after volume is detached
    EXPECT_FALSE(rbaStateManager->BulkAcquireOwnership(VOLUME_ID, 0, RBA_AMOUNT));
}

} // namespace pos