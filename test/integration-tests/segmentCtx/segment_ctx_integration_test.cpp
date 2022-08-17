#include "segment_ctx_integration_test.h"

#include <string>

#include "test/integration-tests/segmentCtx/segment_ctx_tester.h"
#include "src/logger/logger.h"
#include "src/allocator/context_manager/context_io_manager.h"
#include "src/allocator/context_manager/context_manager.h"
#include "src/allocator/context_manager/rebuild_ctx/rebuild_ctx.h"
#include "src/include/address_type.h"
#include "src/meta_file_intf/mock_file_intf.h"
#include "src/allocator/address/allocator_address_info.h"

#include "test/integration-tests/allocator/allocator_it_common.h"
#include "test/unit-tests/allocator/context_manager/allocator_ctx/allocator_ctx_mock.h"
#include "test/unit-tests/allocator/context_manager/allocator_file_io_mock.h"
#include "test/unit-tests/allocator/context_manager/block_allocation_status_mock.h"
#include "test/unit-tests/allocator/context_manager/context_io_manager_mock.h"
#include "test/unit-tests/allocator/context_manager/context_replayer_mock.h"

#include "test/unit-tests/allocator/context_manager/segment_ctx/segment_info_mock.h"
#include "test/unit-tests/allocator/context_manager/segment_ctx/segment_list_mock.h"
#include "test/unit-tests/event_scheduler/event_mock.h"
#include "test/unit-tests/event_scheduler/event_scheduler_mock.h"
#include "test/unit-tests/lib/bitmap_mock.h"
#include "test/unit-tests/meta_file_intf/async_context_mock.h"
#include "test/unit-tests/meta_file_intf/meta_file_intf_mock.h"

using ::testing::NiceMock;

namespace pos
{
class SegmentCtxIntegrationTest : public ::testing::Test
{
protected:
    virtual void SetUp(void)
    {
        segmentCtx = new NiceMock<SegmentCtxTester>;
    };

    virtual void TearDown(void)
    {
        delete segmentCtx;
    };

    NiceMock<SegmentCtxTester>* segmentCtx;
};

TEST_F(SegmentCtxIntegrationTest, MockOverride_Example)
{
    NiceMock<SegmentCtxTesterExample>* seg = new NiceMock<SegmentCtxTesterExample>;

    // Test with re-defined mock functions.
    uint64_t ret = seg->GetNumOfFreeSegment();
    EXPECT_EQ(ret, 100);
    delete seg;
}

TEST_F(SegmentCtxIntegrationTest, LifeCycleTest)
{
    // test init and dispose.
    segmentCtx->Init();
    segmentCtx->Dispose();

    // re-init
    segmentCtx->Init();

    // allocate free segments.
    uint64_t numOfFreeSegments = segmentCtx->GetNumOfSegment();
    uint64_t remainFreeSegCount = segmentCtx->GetNumOfFreeSegment();
    EXPECT_EQ(remainFreeSegCount, numOfFreeSegments);

    uint64_t numOfAllocatedSegments = segmentCtx->GetAllocatedSegmentCount();
    EXPECT_EQ(numOfAllocatedSegments, 0);

    for (int segId = 0; segId < numOfFreeSegments; ++segId)
    {
        SegmentId getId = segmentCtx->AllocateFreeSegment();
        EXPECT_EQ(getId, segId);
    }

    // All free segments exhausted.
    SegmentId getInvalidId = segmentCtx->AllocateFreeSegment();
    EXPECT_EQ(getInvalidId, UNMAP_SEGMENT);

    remainFreeSegCount = segmentCtx->GetNumOfFreeSegment();
    EXPECT_EQ(remainFreeSegCount, 0);

    numOfAllocatedSegments = segmentCtx->GetAllocatedSegmentCount();
    EXPECT_EQ(numOfAllocatedSegments, numOfFreeSegments);

    // Start log write - group id 0
    int logGroupId = 0;
    uint32_t segId = 0;

    VirtualBlkAddr vsa;
    VirtualBlks blks;

    uint32_t numOfBlksPerStripe = segmentCtx->GetNumOfBlksPerStripe();

    vsa.stripeId = 0;
    for (uint32_t offset = 0; offset < numOfBlksPerStripe; offset++)
    {
        vsa.offset = offset;
        blks.startVsa = vsa;
        blks.numBlks = 1;

        segmentCtx->ValidateBlocksWithGroupId(blks, logGroupId);
    }

    uint32_t validBlkCount = segmentCtx->GetValidBlockCount(segId);
    EXPECT_EQ(validBlkCount, segmentCtx->GetNumOfBlksPerStripe());

    // Increase occupied stripe count to make segment 0 in ssd state. (valid block count is still not 0)
    for (uint32_t stripeId = 0; stripeId < segmentCtx->GetStripesPerSegment(); stripeId++)
    {
        segmentCtx->UpdateOccupiedStripeCount(stripeId);
    }

    uint32_t occupiedStripeCount = segmentCtx->GetOccupiedStripeCount(segId);
    EXPECT_EQ(occupiedStripeCount, segmentCtx->GetStripesPerSegment());

    SegmentInfo* retSegmentInfo = segmentCtx->GetSegmentInfos();
    EXPECT_EQ(retSegmentInfo[segId].GetState(), SegmentState::SSD);
}

TEST_F(SegmentCtxIntegrationTest, UpdateSegmentList_IfTargetSegmentInvalidatedByUserWrite)
{
    NiceMock<MockAllocatorAddressInfo> addrInfo;
    NiceMock<MockEventScheduler> eventScheduler;
    NiceMock<MockTelemetryPublisher> tp;

    NiceMock<MockContextIoManager>* ioManager = new NiceMock<MockContextIoManager>;
    NiceMock<MockAllocatorCtx>* allocatorCtx = new NiceMock<MockAllocatorCtx>;
    NiceMock<MockRebuildCtx>* rebuildCtx = new NiceMock<MockRebuildCtx>;
    NiceMock<MockGcCtx>* gcCtx = new NiceMock<MockGcCtx>;
    NiceMock<MockBlockAllocationStatus>* blockAllocStatus = new NiceMock<MockBlockAllocationStatus>();
    NiceMock<MockContextReplayer>* contextReplayer = new NiceMock<MockContextReplayer>;

    SegmentList freeSegmentList;
    SegmentList ssdSegmentList;
    SegmentList rebuildSegmentList;
    SegmentList victimSegmentList;
    SegmentList nvramSegmentList;

    // Given
    const uint32_t numOfSegment = 10;
    const uint32_t maxValidBlockCount = 128;
    const uint32_t maxOccupiedStripeCount = 128;
    const uint32_t numOfStripesPerSegment = 10;
    const uint32_t arrayId = 0;
    uint32_t expectedVictimSegId = 0;

    SegmentInfo* segInfos = new SegmentInfo[numOfSegment](maxValidBlockCount, maxOccupiedStripeCount, SegmentState::SSD);
    SegmentCtx* segmentCtx = new SegmentCtx(&tp, rebuildCtx, &addrInfo, gcCtx, arrayId, segInfos);

    ON_CALL(addrInfo, IsUT).WillByDefault(Return(true));
    ON_CALL(addrInfo, GetblksPerSegment).WillByDefault(Return(maxValidBlockCount));
    ON_CALL(addrInfo, GetnumUserAreaSegments).WillByDefault(Return(numOfSegment));
    ON_CALL(addrInfo, GetstripesPerSegment).WillByDefault(Return(numOfStripesPerSegment));

    segmentCtx->SetSegmentList(SegmentState::SSD, &ssdSegmentList);
    segmentCtx->SetSegmentList(SegmentState::VICTIM, &victimSegmentList);
    segmentCtx->SetSegmentList(SegmentState::NVRAM, &nvramSegmentList);
    segmentCtx->SetSegmentList(SegmentState::FREE, &freeSegmentList);
    segmentCtx->SetRebuildList(&rebuildSegmentList);

    // Add all segment into ssd segment list.
    for (uint32_t segId = 0; segId < numOfSegment; ++segId)
    {
        ssdSegmentList.AddToList(segId);
    }

    // decrese valid block count by user write to make segId 0 as victim segment.
    VirtualBlks blks;
    blks.numBlks = 64;
    blks.startVsa.stripeId = expectedVictimSegId * numOfSegment;
    blks.startVsa.offset = 0;
    segmentCtx->InvalidateBlks(blks, false);

    // get victim segment id
    uint32_t victimSegId = segmentCtx->AllocateGCVictimSegment();
    EXPECT_EQ(expectedVictimSegId, victimSegId);

    bool ret = victimSegmentList.Contains(victimSegId);
    EXPECT_EQ(true, ret);

    ret = freeSegmentList.Contains(victimSegId);
    EXPECT_EQ(false, ret);

    ret = nvramSegmentList.Contains(victimSegId);
    EXPECT_EQ(false, ret);

    ret = rebuildSegmentList.Contains(victimSegId);
    EXPECT_EQ(false, ret);

    // Invalidated victim block by user write before doing garbage collection.
    blks.startVsa.offset = 64;
    segmentCtx->InvalidateBlks(blks, false);

    uint32_t remainValidBlkCount = segInfos[victimSegId].GetValidBlockCount();
    EXPECT_EQ(0, remainValidBlkCount);

    // all segment valid blocks invalidated but its state is SegmentState::VICTIM,
    // hence it just wait for being freed by garbage collector.
    ret = victimSegmentList.Contains(victimSegId);
    EXPECT_EQ(true, ret);

    ret = freeSegmentList.Contains(victimSegId);
    EXPECT_EQ(false, ret);

    // return back victim segment into free list.
    blks.numBlks = 0;
    segmentCtx->InvalidateBlks(blks, true);

    ret = victimSegmentList.Contains(victimSegId);
    EXPECT_EQ(false, ret);

    ret = freeSegmentList.Contains(victimSegId);
    EXPECT_EQ(true, ret);

    // add target segment into rebuild list.
    const uint32_t rebuildTargetSegId = 2;
    rebuildSegmentList.AddToList(rebuildTargetSegId);

    // Invalidated by user write
    blks.numBlks = 64;
    blks.startVsa.stripeId = rebuildTargetSegId * numOfSegment;
    blks.startVsa.offset = 0;
    segmentCtx->InvalidateBlks(blks, false);

    victimSegId = segmentCtx->AllocateGCVictimSegment();
    EXPECT_EQ(rebuildTargetSegId, victimSegId);

    // Invalidated victim block by user write before doing garbage collection.
    blks.numBlks = 64;
    blks.startVsa.stripeId = victimSegId * numOfSegment;
    blks.startVsa.offset = 0;
    segmentCtx->InvalidateBlks(blks, false);

    remainValidBlkCount = segInfos[victimSegId].GetValidBlockCount();
    EXPECT_EQ(0, remainValidBlkCount);

    ret = victimSegmentList.Contains(victimSegId);
    EXPECT_EQ(false, ret);

    ret = freeSegmentList.Contains(victimSegId);
    EXPECT_EQ(false, ret);

    ret = rebuildSegmentList.Contains(victimSegId);
    EXPECT_EQ(true, ret);

    delete [] segInfos;
    delete segmentCtx;
    delete ioManager;
    delete allocatorCtx;
    delete rebuildCtx;
    delete gcCtx;
    delete blockAllocStatus;
    delete contextReplayer;
}
}
