#include "src/allocator/context_manager/context_manager.h"

#include <gtest/gtest.h>

#include <mutex>

#include "test/unit-tests/allocator/address/allocator_address_info_mock.h"
#include "test/unit-tests/allocator/context_manager/allocator_ctx/allocator_ctx_mock.h"
#include "test/unit-tests/allocator/context_manager/block_allocation_status_mock.h"
#include "test/unit-tests/allocator/context_manager/context_io_manager_mock.h"
#include "test/unit-tests/allocator/context_manager/context_manager_mock.h"
#include "test/unit-tests/allocator/context_manager/context_replayer_mock.h"
#include "test/unit-tests/allocator/context_manager/gc_ctx/gc_ctx_mock.h"
#include "test/unit-tests/allocator/context_manager/i_allocator_file_io_client_mock.h"
#include "test/unit-tests/allocator/context_manager/rebuild_ctx/rebuild_ctx_mock.h"
#include "test/unit-tests/allocator/context_manager/segment_ctx/segment_ctx_mock.h"
#include "test/unit-tests/journal_manager/checkpoint/checkpoint_meta_flush_completed_mock.h"
#include "test/unit-tests/telemetry/telemetry_client/telemetry_publisher_mock.h"

using ::testing::_;
using ::testing::AtLeast;
using testing::NiceMock;
using ::testing::Return;
using ::testing::ReturnRef;
namespace pos
{
TEST(ContextManager, ContextManager_)
{
}

TEST(ContextManager, Init_TestCaseFormatFormat)
{
    // given
    NiceMock<MockAllocatorAddressInfo> addrInfo;
    NiceMock<MockAllocatorCtx>* allocCtx = new NiceMock<MockAllocatorCtx>;
    NiceMock<MockSegmentCtx>* segCtx = new NiceMock<MockSegmentCtx>;
    NiceMock<MockRebuildCtx>* reCtx = new NiceMock<MockRebuildCtx>;
    NiceMock<MockGcCtx>* gcCtx = new NiceMock<MockGcCtx>;
    NiceMock<MockBlockAllocationStatus>* blockAllocStatus = new NiceMock<MockBlockAllocationStatus>;
    NiceMock<MockContextIoManager>* ioManager = new NiceMock<MockContextIoManager>;
    NiceMock<MockTelemetryPublisher> tc;
    ContextManager ctxManager(&tc, allocCtx, segCtx, reCtx, gcCtx, blockAllocStatus, ioManager, nullptr, &addrInfo, 0);

    EXPECT_CALL(*allocCtx, Init);
    EXPECT_CALL(*segCtx, Init);
    EXPECT_CALL(*reCtx, Init);
    EXPECT_CALL(*ioManager, Init);

    // when 1.
    ctxManager.Init();
}

TEST(ContextManager, Close_TestAllClosed)
{
    // given
    NiceMock<MockAllocatorAddressInfo> addrInfo;
    NiceMock<MockAllocatorCtx>* allocCtx = new NiceMock<MockAllocatorCtx>();
    NiceMock<MockSegmentCtx>* segCtx = new NiceMock<MockSegmentCtx>();
    NiceMock<MockRebuildCtx>* reCtx = new NiceMock<MockRebuildCtx>();
    NiceMock<MockGcCtx>* gcCtx = new NiceMock<MockGcCtx>();
    NiceMock<MockBlockAllocationStatus>* blockAllocStatus = new NiceMock<MockBlockAllocationStatus>();
    NiceMock<MockContextIoManager>* ioManager = new NiceMock<MockContextIoManager>;
    NiceMock<MockTelemetryPublisher> tc;
    ContextManager ctxManager(&tc, allocCtx, segCtx, reCtx, gcCtx, blockAllocStatus, ioManager, nullptr, &addrInfo, 0);

    EXPECT_CALL(*allocCtx, Dispose);
    EXPECT_CALL(*segCtx, Dispose);
    EXPECT_CALL(*ioManager, Dispose);
    EXPECT_CALL(*reCtx, Dispose);

    // when
    ctxManager.Dispose();
}

TEST(ContextManager, FlushContexts_testFlushStarted)
{
    // given
    NiceMock<MockAllocatorCtx>* allocCtx = new NiceMock<MockAllocatorCtx>();
    NiceMock<MockSegmentCtx>* segCtx = new NiceMock<MockSegmentCtx>();
    NiceMock<MockRebuildCtx>* reCtx = new NiceMock<MockRebuildCtx>();
    NiceMock<MockGcCtx>* gcCtx = new NiceMock<MockGcCtx>();
    NiceMock<MockBlockAllocationStatus>* blockAllocStatus = new NiceMock<MockBlockAllocationStatus>();
    NiceMock<MockContextIoManager>* ioManager = new NiceMock<MockContextIoManager>;
    NiceMock<MockTelemetryPublisher> tc;
    ContextManager ctxManager(&tc, allocCtx, segCtx, reCtx, gcCtx, blockAllocStatus, ioManager, nullptr, nullptr, 0);

    EXPECT_CALL(*ioManager, FlushContexts).WillOnce(Return(0));

    // when
    int ret = ctxManager.FlushContexts(nullptr, false);
    // then
    EXPECT_LE(0, ret);
}

TEST(ContextManager, UpdateOccupiedStripeCount_testWhenSegmentIsNotFreed)
{
    // given
    NiceMock<MockAllocatorAddressInfo> addrInfo;
    NiceMock<MockAllocatorCtx>* allocCtx = new NiceMock<MockAllocatorCtx>();
    NiceMock<MockSegmentCtx>* segCtx = new NiceMock<MockSegmentCtx>();
    NiceMock<MockRebuildCtx>* reCtx = new NiceMock<MockRebuildCtx>();
    NiceMock<MockGcCtx>* gcCtx = new NiceMock<MockGcCtx>();
    NiceMock<MockBlockAllocationStatus>* blockAllocStatus = new NiceMock<MockBlockAllocationStatus>();
    NiceMock<MockContextIoManager>* ioManager = new NiceMock<MockContextIoManager>;
    NiceMock<MockTelemetryPublisher> tc;
    ContextManager ctxManager(&tc, allocCtx, segCtx, reCtx, gcCtx, blockAllocStatus, ioManager, nullptr, &addrInfo, 0);

    // expect
    EXPECT_CALL(addrInfo, GetstripesPerSegment).WillOnce(Return(1024));
    EXPECT_CALL(*segCtx, IncreaseOccupiedStripeCount).WillOnce(Return(false));

    // when
    ctxManager.UpdateOccupiedStripeCount(5);
}

TEST(ContextManager, UpdateOccupiedStripeCount_testWhenSegmentFreed)
{
    // given
    NiceMock<MockAllocatorAddressInfo> addrInfo;
    NiceMock<MockAllocatorCtx>* allocCtx = new NiceMock<MockAllocatorCtx>();
    NiceMock<MockSegmentCtx>* segCtx = new NiceMock<MockSegmentCtx>();
    NiceMock<MockRebuildCtx>* reCtx = new NiceMock<MockRebuildCtx>();
    NiceMock<MockGcCtx>* gcCtx = new NiceMock<MockGcCtx>();
    NiceMock<MockBlockAllocationStatus>* blockAllocStatus = new NiceMock<MockBlockAllocationStatus>();
    NiceMock<MockContextIoManager>* ioManager = new NiceMock<MockContextIoManager>;
    NiceMock<MockTelemetryPublisher> tc;
    ContextManager ctxManager(&tc, allocCtx, segCtx, reCtx, gcCtx, blockAllocStatus, ioManager, nullptr, &addrInfo, 0);

    // expect
    EXPECT_CALL(addrInfo, GetstripesPerSegment).WillOnce(Return(1024));
    EXPECT_CALL(*segCtx, IncreaseOccupiedStripeCount).WillOnce(Return(true));

    // when
    ctxManager.UpdateOccupiedStripeCount(5);
}

TEST(ContextManager, AllocateFreeSegment_TestFreeSegmentAllocationByState)
{
    // given
    NiceMock<MockAllocatorCtx>* allocCtx = new NiceMock<MockAllocatorCtx>();
    NiceMock<MockSegmentCtx>* segCtx = new NiceMock<MockSegmentCtx>();
    NiceMock<MockRebuildCtx>* reCtx = new NiceMock<MockRebuildCtx>();
    NiceMock<MockGcCtx>* gcCtx = new NiceMock<MockGcCtx>();
    NiceMock<MockBlockAllocationStatus>* blockAllocStatus = new NiceMock<MockBlockAllocationStatus>();
    NiceMock<MockContextIoManager>* ioManager = new NiceMock<MockContextIoManager>;
    NiceMock<MockTelemetryPublisher> tc;
    ContextManager ctxManager(&tc, allocCtx, segCtx, reCtx, gcCtx, blockAllocStatus, ioManager, nullptr, nullptr, 0);

    // given 1.
    EXPECT_CALL(*segCtx, AllocateFreeSegment).WillOnce(Return(UNMAP_SEGMENT));
    // when 1.
    SegmentId ret = ctxManager.AllocateFreeSegment();
    EXPECT_EQ(ret, UNMAP_SEGMENT);

    // given 2. first failed, second success
    EXPECT_CALL(*segCtx, AllocateFreeSegment).WillOnce(Return(5));
    // when 2.
    ret = ctxManager.AllocateFreeSegment();
    EXPECT_EQ(ret, 5);
}

TEST(ContextManager, AllocateGCVictimSegment_TestIfVictimIsUpdated)
{
    // given
    AllocatorAddressInfo addrInfo;
    addrInfo.SetstripesPerSegment(5);
    addrInfo.SetblksPerSegment(20);
    addrInfo.SetnumUserAreaSegments(5);

    std::mutex segStateLock;
    NiceMock<MockAllocatorCtx>* allocCtx = new NiceMock<MockAllocatorCtx>();
    NiceMock<MockSegmentCtx>* segCtx = new NiceMock<MockSegmentCtx>();
    NiceMock<MockRebuildCtx>* reCtx = new NiceMock<MockRebuildCtx>();
    NiceMock<MockGcCtx>* gcCtx = new NiceMock<MockGcCtx>();
    NiceMock<MockBlockAllocationStatus>* blockAllocStatus = new NiceMock<MockBlockAllocationStatus>();
    NiceMock<MockContextIoManager>* ioManager = new NiceMock<MockContextIoManager>;
    NiceMock<MockTelemetryPublisher> tc;
    ContextManager ctxManager(&tc, allocCtx, segCtx, reCtx, gcCtx, blockAllocStatus, ioManager, nullptr, nullptr, 0);

    EXPECT_CALL(*segCtx, AllocateGCVictimSegment).WillOnce(Return(15));

    // when 1
    int ret = ctxManager.AllocateGCVictimSegment();
    // then
    EXPECT_EQ(15, ret);

    EXPECT_CALL(*segCtx, AllocateGCVictimSegment).WillOnce(Return(UNMAP_SEGMENT));

    // when 2.
    ret = ctxManager.AllocateGCVictimSegment();
    // then 2.
    EXPECT_EQ(UNMAP_SEGMENT, ret);
}

TEST(ContextManager, GetNumOfFreeSegment_TestSimpleGetter)
{
    // given
    NiceMock<MockAllocatorCtx>* allocCtx = new NiceMock<MockAllocatorCtx>();
    NiceMock<MockSegmentCtx>* segCtx = new NiceMock<MockSegmentCtx>();
    NiceMock<MockRebuildCtx>* reCtx = new NiceMock<MockRebuildCtx>();
    NiceMock<MockGcCtx>* gcCtx = new NiceMock<MockGcCtx>();
    NiceMock<MockBlockAllocationStatus>* blockAllocStatus = new NiceMock<MockBlockAllocationStatus>();
    NiceMock<MockContextIoManager>* ioManager = new NiceMock<MockContextIoManager>;
    NiceMock<MockTelemetryPublisher> tc;
    ContextManager ctxManager(&tc, allocCtx, segCtx, reCtx, gcCtx, blockAllocStatus, ioManager, nullptr, nullptr, 0);

    EXPECT_CALL(*segCtx, GetNumOfFreeSegment).WillOnce(Return(50));
    // when 1.
    int ret = segCtx->GetNumOfFreeSegment();
    // then 1.
    EXPECT_EQ(50, ret);
    // given 2.
    EXPECT_CALL(*segCtx, GetNumOfFreeSegmentWoLock).WillOnce(Return(50));
    // when 2.
    ret = segCtx->GetNumOfFreeSegmentWoLock();
    // then 2.
    EXPECT_EQ(50, ret);
}

TEST(ContextManager, GetGcThreshold_TestSimpleGetterByMode)
{
    // given
    NiceMock<MockAllocatorCtx>* allocCtx = new NiceMock<MockAllocatorCtx>();
    NiceMock<MockSegmentCtx>* segCtx = new NiceMock<MockSegmentCtx>();
    NiceMock<MockRebuildCtx>* reCtx = new NiceMock<MockRebuildCtx>();
    GcCtx* gcCtx = new GcCtx();
    NiceMock<MockBlockAllocationStatus>* blockAllocStatus = new NiceMock<MockBlockAllocationStatus>();
    NiceMock<MockContextIoManager>* ioManager = new NiceMock<MockContextIoManager>;
    NiceMock<MockTelemetryPublisher> tc;
    ContextManager ctxManager(&tc, allocCtx, segCtx, reCtx, gcCtx, blockAllocStatus, ioManager, nullptr, nullptr, 0);

    gcCtx->SetNormalGcThreshold(10);
    gcCtx->SetUrgentThreshold(5);

    // when 1.
    int ret = ctxManager.GetGcThreshold(MODE_NORMAL_GC);
    // then 1.
    EXPECT_EQ(10, ret);

    // when 2.
    ret = ctxManager.GetGcThreshold(MODE_URGENT_GC);
    // then 2.
    EXPECT_EQ(5, ret);
}

TEST(ContextManager, GetStoredContextVersion_TestSimpleGetter)
{
    // given
    NiceMock<MockAllocatorCtx>* allocCtx = new NiceMock<MockAllocatorCtx>();
    NiceMock<MockSegmentCtx>* segCtx = new NiceMock<MockSegmentCtx>();
    NiceMock<MockRebuildCtx>* reCtx = new NiceMock<MockRebuildCtx>();
    NiceMock<MockGcCtx>* gcCtx = new NiceMock<MockGcCtx>();
    NiceMock<MockBlockAllocationStatus>* blockAllocStatus = new NiceMock<MockBlockAllocationStatus>();
    NiceMock<MockContextIoManager>* ioManager = new NiceMock<MockContextIoManager>;
    NiceMock<MockTelemetryPublisher> tc;
    ContextManager ctxManager(&tc, allocCtx, segCtx, reCtx, gcCtx, blockAllocStatus, ioManager, nullptr, nullptr, 0);

    // when 1.
    EXPECT_CALL(*ioManager, GetStoredContextVersion(SEGMENT_CTX));
    ctxManager.GetStoredContextVersion(SEGMENT_CTX);

    // when 2.
    EXPECT_CALL(*ioManager, GetStoredContextVersion(ALLOCATOR_CTX));
    ctxManager.GetStoredContextVersion(ALLOCATOR_CTX);
}

TEST(ContextManager, AllocateRebuildTargetSegment_TestSimpleByPassFunc)
{
    // given
    NiceMock<MockAllocatorCtx>* allocCtx = new NiceMock<MockAllocatorCtx>();
    NiceMock<MockSegmentCtx>* segCtx = new NiceMock<MockSegmentCtx>();
    NiceMock<MockRebuildCtx>* reCtx = new NiceMock<MockRebuildCtx>();
    NiceMock<MockGcCtx>* gcCtx = new NiceMock<MockGcCtx>();
    NiceMock<MockBlockAllocationStatus>* blockAllocStatus = new NiceMock<MockBlockAllocationStatus>();
    NiceMock<MockContextIoManager>* ioManager = new NiceMock<MockContextIoManager>;
    NiceMock<MockTelemetryPublisher> tc;
    ContextManager ctxManager(&tc, allocCtx, segCtx, reCtx, gcCtx, blockAllocStatus, ioManager, nullptr, nullptr, 0);

    EXPECT_CALL(*segCtx, GetRebuildTargetSegment).WillOnce(Return(5));

    // when
    int ret = ctxManager.AllocateRebuildTargetSegment();

    // then
    EXPECT_EQ(5, ret);
}

TEST(ContextManager, ReleaseRebuildSegment__TestSimpleByPassFunc)
{
    // given
    NiceMock<MockAllocatorCtx>* allocCtx = new NiceMock<MockAllocatorCtx>();
    NiceMock<MockSegmentCtx>* segCtx = new NiceMock<MockSegmentCtx>();
    NiceMock<MockRebuildCtx>* reCtx = new NiceMock<MockRebuildCtx>();
    NiceMock<MockGcCtx>* gcCtx = new NiceMock<MockGcCtx>();
    NiceMock<MockBlockAllocationStatus>* blockAllocStatus = new NiceMock<MockBlockAllocationStatus>();
    NiceMock<MockContextIoManager>* ioManager = new NiceMock<MockContextIoManager>;
    NiceMock<MockTelemetryPublisher> tc;
    ContextManager ctxManager(&tc, allocCtx, segCtx, reCtx, gcCtx, blockAllocStatus, ioManager, nullptr, nullptr, 0);

    // given 1.
    EXPECT_CALL(*segCtx, SetRebuildCompleted(5)).WillOnce(Return(0));
    // when 1.
    int ret = ctxManager.ReleaseRebuildSegment(5);
    // then 1.
    EXPECT_EQ(0, ret);

    // given 2.
    EXPECT_CALL(*segCtx, SetRebuildCompleted(5)).WillOnce(Return(-1));
    // when 2.
    ret = ctxManager.ReleaseRebuildSegment(5);
    // then 2.
    EXPECT_EQ(-1, ret);

    // given 3.
    EXPECT_CALL(*segCtx, SetRebuildCompleted(5)).WillOnce(Return(0));
    // when 3.
    ret = ctxManager.ReleaseRebuildSegment(5);
    // then 3.
    EXPECT_EQ(0, ret);
}

TEST(ContextManager, NeedRebuildAgain_TestSimpleByPassFunc)
{
    // given
    NiceMock<MockAllocatorCtx>* allocCtx = new NiceMock<MockAllocatorCtx>();
    NiceMock<MockSegmentCtx>* segCtx = new NiceMock<MockSegmentCtx>();
    NiceMock<MockRebuildCtx>* reCtx = new NiceMock<MockRebuildCtx>();
    NiceMock<MockGcCtx>* gcCtx = new NiceMock<MockGcCtx>();
    NiceMock<MockBlockAllocationStatus>* blockAllocStatus = new NiceMock<MockBlockAllocationStatus>();
    NiceMock<MockContextIoManager>* ioManager = new NiceMock<MockContextIoManager>;
    NiceMock<MockTelemetryPublisher> tc;
    ContextManager ctxManager(&tc, allocCtx, segCtx, reCtx, gcCtx, blockAllocStatus, ioManager, nullptr, nullptr, 0);

    EXPECT_CALL(*segCtx, LoadRebuildList).WillOnce(Return(true));

    // when
    bool ret = ctxManager.NeedRebuildAgain();

    // then
    EXPECT_EQ(true, ret);
}

TEST(ContextManager, GetContextSectionAddr_TestSimpleGetter)
{
    // given
    NiceMock<MockAllocatorCtx>* allocCtx = new NiceMock<MockAllocatorCtx>();
    NiceMock<MockSegmentCtx>* segCtx = new NiceMock<MockSegmentCtx>();
    NiceMock<MockRebuildCtx>* reCtx = new NiceMock<MockRebuildCtx>();
    NiceMock<MockGcCtx>* gcCtx = new NiceMock<MockGcCtx>();
    NiceMock<MockBlockAllocationStatus>* blockAllocStatus = new NiceMock<MockBlockAllocationStatus>();
    NiceMock<MockContextIoManager>* ioManager = new NiceMock<MockContextIoManager>;
    NiceMock<MockTelemetryPublisher> tc;
    ContextManager ctxManager(&tc, allocCtx, segCtx, reCtx, gcCtx, blockAllocStatus, ioManager, nullptr, nullptr, 0);

    EXPECT_CALL(*ioManager, GetContextSectionAddr).WillOnce(Return((char*)100));

    // when
    char* ret = ctxManager.GetContextSectionAddr(0, 0);

    // then
    EXPECT_EQ((char*)100, ret);
}

TEST(ContextManager, GetContextSectionSize_TestSimpleGetter)
{
    // given
    NiceMock<MockAllocatorCtx>* allocCtx = new NiceMock<MockAllocatorCtx>();
    NiceMock<MockSegmentCtx>* segCtx = new NiceMock<MockSegmentCtx>();
    NiceMock<MockRebuildCtx>* reCtx = new NiceMock<MockRebuildCtx>();
    NiceMock<MockGcCtx>* gcCtx = new NiceMock<MockGcCtx>();
    NiceMock<MockBlockAllocationStatus>* blockAllocStatus = new NiceMock<MockBlockAllocationStatus>();
    NiceMock<MockContextIoManager>* ioManager = new NiceMock<MockContextIoManager>;
    NiceMock<MockTelemetryPublisher> tc;
    ContextManager ctxManager(&tc, allocCtx, segCtx, reCtx, gcCtx, blockAllocStatus, ioManager, nullptr, nullptr, 0);

    EXPECT_CALL(*ioManager, GetContextSectionSize).WillOnce(Return(1000));

    // when
    int ret = ctxManager.GetContextSectionSize(0, 0);

    // then
    EXPECT_EQ(1000, ret);
}

TEST(ContextManager, GetRebuildCtx_TestSimpleGetter)
{
    // given
    NiceMock<MockAllocatorCtx>* allocCtx = new NiceMock<MockAllocatorCtx>();
    NiceMock<MockSegmentCtx>* segCtx = new NiceMock<MockSegmentCtx>();
    NiceMock<MockRebuildCtx>* reCtx = new NiceMock<MockRebuildCtx>();
    NiceMock<MockGcCtx>* gcCtx = new NiceMock<MockGcCtx>();
    NiceMock<MockBlockAllocationStatus>* blockAllocStatus = new NiceMock<MockBlockAllocationStatus>();
    NiceMock<MockContextIoManager>* ioManager = new NiceMock<MockContextIoManager>;
    NiceMock<MockTelemetryPublisher> tc;
    ContextManager ctxManager(&tc, allocCtx, segCtx, reCtx, gcCtx, blockAllocStatus, ioManager, nullptr, nullptr, 0);
    // when
    MockRebuildCtx* ret = reinterpret_cast<MockRebuildCtx*>(ctxManager.GetRebuildCtx());
    // then
    EXPECT_EQ(reCtx, ret);
}

TEST(ContextManager, GetGcCtx_TestSimpleGetter)
{
    // given
    NiceMock<MockAllocatorCtx>* allocCtx = new NiceMock<MockAllocatorCtx>();
    NiceMock<MockSegmentCtx>* segCtx = new NiceMock<MockSegmentCtx>();
    NiceMock<MockRebuildCtx>* reCtx = new NiceMock<MockRebuildCtx>();
    NiceMock<MockGcCtx>* gcCtx = new NiceMock<MockGcCtx>();
    NiceMock<MockBlockAllocationStatus>* blockAllocStatus = new NiceMock<MockBlockAllocationStatus>();
    NiceMock<MockContextIoManager>* ioManager = new NiceMock<MockContextIoManager>;
    NiceMock<MockTelemetryPublisher> tc;
    ContextManager ctxManager(&tc, allocCtx, segCtx, reCtx, gcCtx, blockAllocStatus, ioManager, nullptr, nullptr, 0);
    // when
    MockGcCtx* ret = reinterpret_cast<MockGcCtx*>(ctxManager.GetGcCtx());
    // then
    EXPECT_EQ(gcCtx, ret);
}

TEST(ContextManager, GetContextReplayer_TestSimpleGetter)
{
    // given
    NiceMock<MockAllocatorCtx>* allocCtx = new NiceMock<MockAllocatorCtx>();
    NiceMock<MockSegmentCtx>* segCtx = new NiceMock<MockSegmentCtx>();
    NiceMock<MockRebuildCtx>* reCtx = new NiceMock<MockRebuildCtx>();
    NiceMock<MockGcCtx>* gcCtx = new NiceMock<MockGcCtx>();
    NiceMock<MockBlockAllocationStatus>* blockAllocStatus = new NiceMock<MockBlockAllocationStatus>();
    NiceMock<MockContextIoManager>* ioManager = new NiceMock<MockContextIoManager>;
    NiceMock<MockContextReplayer>* ctxReplayer = new NiceMock<MockContextReplayer>();
    NiceMock<MockTelemetryPublisher> tc;
    ContextManager ctxManager(&tc, allocCtx, segCtx, reCtx, gcCtx, blockAllocStatus, ioManager, ctxReplayer, nullptr, 0);
    // when
    ContextReplayer* ret = ctxManager.GetContextReplayer();
    // then
    EXPECT_EQ(ctxReplayer, ret);
}

TEST(ContextManager, SetNextSsdLsid_TestCheckReturnedSegmentId)
{
    // given
    NiceMock<MockAllocatorCtx>* allocCtx = new NiceMock<MockAllocatorCtx>();
    NiceMock<MockSegmentCtx>* segCtx = new NiceMock<MockSegmentCtx>();
    NiceMock<MockRebuildCtx>* reCtx = new NiceMock<MockRebuildCtx>();
    NiceMock<MockGcCtx>* gcCtx = new NiceMock<MockGcCtx>();
    NiceMock<MockBlockAllocationStatus>* blockAllocStatus = new NiceMock<MockBlockAllocationStatus>();
    NiceMock<MockContextIoManager>* ioManager = new NiceMock<MockContextIoManager>;
    NiceMock<MockTelemetryPublisher> tc;
    ContextManager ctxManager(&tc, allocCtx, segCtx, reCtx, gcCtx, blockAllocStatus, ioManager, nullptr, nullptr, 0);

    // given 1.
    EXPECT_CALL(*segCtx, AllocateFreeSegment).WillOnce(Return(5));
    EXPECT_CALL(*allocCtx, SetNextSsdLsid(5));

    std::mutex allocCtxLock;
    EXPECT_CALL(*allocCtx, GetCtxLock).WillOnce(ReturnRef(allocCtxLock));

    // when 1.
    int ret = ctxManager.SetNextSsdLsid();
    // then 1.
    EXPECT_EQ(0, ret);

    // given 2.
    EXPECT_CALL(*segCtx, AllocateFreeSegment).WillOnce(Return(UNMAP_SEGMENT));
    // when 2.
    ret = ctxManager.SetNextSsdLsid();
    // then 2.
    EXPECT_EQ((int)-EID(ALLOCATOR_NO_FREE_SEGMENT), ret);
}

TEST(ContextManager, GetSegmentCtx_TestSimpleGetter)
{
    // given
    NiceMock<MockAllocatorCtx>* allocCtx = new NiceMock<MockAllocatorCtx>();
    NiceMock<MockSegmentCtx>* segCtx = new NiceMock<MockSegmentCtx>();
    NiceMock<MockRebuildCtx>* reCtx = new NiceMock<MockRebuildCtx>();
    NiceMock<MockGcCtx>* gcCtx = new NiceMock<MockGcCtx>();
    NiceMock<MockBlockAllocationStatus>* blockAllocStatus = new NiceMock<MockBlockAllocationStatus>();
    NiceMock<MockContextIoManager>* ioManager = new NiceMock<MockContextIoManager>;
    NiceMock<MockTelemetryPublisher> tc;
    ContextManager ctxManager(&tc, allocCtx, segCtx, reCtx, gcCtx, blockAllocStatus, ioManager, nullptr, nullptr, 0);
    // when
    SegmentCtx* ret = ctxManager.GetSegmentCtx();
    // then
    EXPECT_EQ(segCtx, ret);
}

TEST(ContextManager, GetAllocatorCtx_TestSimpleGetter)
{
    // given
    NiceMock<MockAllocatorCtx>* allocCtx = new NiceMock<MockAllocatorCtx>();
    NiceMock<MockSegmentCtx>* segCtx = new NiceMock<MockSegmentCtx>();
    NiceMock<MockRebuildCtx>* reCtx = new NiceMock<MockRebuildCtx>();
    NiceMock<MockGcCtx>* gcCtx = new NiceMock<MockGcCtx>();
    NiceMock<MockBlockAllocationStatus>* blockAllocStatus = new NiceMock<MockBlockAllocationStatus>();
    NiceMock<MockContextIoManager>* ioManager = new NiceMock<MockContextIoManager>;
    NiceMock<MockTelemetryPublisher> tc;
    ContextManager ctxManager(&tc, allocCtx, segCtx, reCtx, gcCtx, blockAllocStatus, ioManager, nullptr, nullptr, 0);
    // when
    AllocatorCtx* ret = ctxManager.GetAllocatorCtx();
    // then
    EXPECT_EQ(allocCtx, ret);
}

TEST(ContextManager, GetCtxLock_TestSimpleGetter)
{
    // given
    NiceMock<MockAllocatorCtx>* allocCtx = new NiceMock<MockAllocatorCtx>();
    NiceMock<MockSegmentCtx>* segCtx = new NiceMock<MockSegmentCtx>();
    NiceMock<MockRebuildCtx>* reCtx = new NiceMock<MockRebuildCtx>();
    NiceMock<MockGcCtx>* gcCtx = new NiceMock<MockGcCtx>();
    NiceMock<MockBlockAllocationStatus>* blockAllocStatus = new NiceMock<MockBlockAllocationStatus>();
    NiceMock<MockContextIoManager>* ioManager = new NiceMock<MockContextIoManager>;
    NiceMock<MockTelemetryPublisher> tc;
    ContextManager ctxManager(&tc, allocCtx, segCtx, reCtx, gcCtx, blockAllocStatus, ioManager, nullptr, nullptr, 0);
    // when
    ctxManager.GetCtxLock();
}

TEST(ContextManager, NeedRebuildAgain_TestSimpleGetter)
{
    // given
    NiceMock<MockAllocatorCtx>* allocCtx = new NiceMock<MockAllocatorCtx>();
    NiceMock<MockSegmentCtx>* segCtx = new NiceMock<MockSegmentCtx>();
    NiceMock<MockRebuildCtx>* reCtx = new NiceMock<MockRebuildCtx>();
    NiceMock<MockGcCtx>* gcCtx = new NiceMock<MockGcCtx>();
    NiceMock<MockBlockAllocationStatus>* blockAllocStatus = new NiceMock<MockBlockAllocationStatus>();
    NiceMock<MockContextIoManager>* ioManager = new NiceMock<MockContextIoManager>;
    NiceMock<MockTelemetryPublisher> tc;
    ContextManager ctxManager(&tc, allocCtx, segCtx, reCtx, gcCtx, blockAllocStatus, ioManager, nullptr, nullptr, 0);
    // when
    ctxManager.NeedRebuildAgain();
}

TEST(ContextManager, ValidateBlks_TestSimple)
{
    // given
    NiceMock<MockAllocatorAddressInfo> addrInfo;
    NiceMock<MockAllocatorCtx>* allocCtx = new NiceMock<MockAllocatorCtx>();
    NiceMock<MockSegmentCtx>* segCtx = new NiceMock<MockSegmentCtx>();
    NiceMock<MockRebuildCtx>* reCtx = new NiceMock<MockRebuildCtx>();
    NiceMock<MockGcCtx>* gcCtx = new NiceMock<MockGcCtx>();
    NiceMock<MockBlockAllocationStatus>* blockAllocStatus = new NiceMock<MockBlockAllocationStatus>();
    NiceMock<MockContextIoManager>* ioManager = new NiceMock<MockContextIoManager>;
    NiceMock<MockTelemetryPublisher> tc;
    ContextManager sut(&tc, allocCtx, segCtx, reCtx, gcCtx, blockAllocStatus, ioManager, nullptr, &addrInfo, 0);

    // given
    VirtualBlkAddr vsa = {
        .stripeId = 120,
        .offset = 0};
    VirtualBlks blks = {
        .startVsa = vsa,
        .numBlks = 1};

    EXPECT_CALL(addrInfo, GetstripesPerSegment).WillRepeatedly(Return(100));
    EXPECT_CALL(*segCtx, IncreaseValidBlockCount(1, 1)).Times(1);
    // when
    sut.ValidateBlks(blks);
}

TEST(ContextManager, InvalidateBlks_TestSimpleSetter)
{
    // given
    NiceMock<MockAllocatorAddressInfo> addrInfo;
    NiceMock<MockAllocatorCtx>* allocCtx = new NiceMock<MockAllocatorCtx>();
    NiceMock<MockSegmentCtx>* segCtx = new NiceMock<MockSegmentCtx>();
    NiceMock<MockRebuildCtx>* reCtx = new NiceMock<MockRebuildCtx>();
    NiceMock<MockGcCtx>* gcCtx = new NiceMock<MockGcCtx>();
    NiceMock<MockBlockAllocationStatus>* blockAllocStatus = new NiceMock<MockBlockAllocationStatus>();
    NiceMock<MockContextIoManager>* ioManager = new NiceMock<MockContextIoManager>;
    NiceMock<MockTelemetryPublisher> tc;
    ContextManager sut(&tc, allocCtx, segCtx, reCtx, gcCtx, blockAllocStatus, ioManager, nullptr, &addrInfo, 0);

    EXPECT_CALL(addrInfo, GetstripesPerSegment).WillRepeatedly(Return(10));

    VirtualBlkAddr vsa = {
        .stripeId = 0,
        .offset = 0};
    VirtualBlks blks = {
        .startVsa = vsa,
        .numBlks = 1};
    // given 1.
    EXPECT_CALL(*segCtx, DecreaseValidBlockCount).WillOnce(Return(false));
    // when 1.
    sut.InvalidateBlks(blks);
    // given 2.
    EXPECT_CALL(*segCtx, DecreaseValidBlockCount).WillOnce(Return(true));
    // when 2.
    sut.InvalidateBlks(blks);
}

TEST(ContextManager, MakeRebuildTargetSegmentList_TestwithFlushOrwithoutFlush)
{
    // given
    NiceMock<MockAllocatorCtx>* allocCtx = new NiceMock<MockAllocatorCtx>();
    NiceMock<MockSegmentCtx>* segCtx = new NiceMock<MockSegmentCtx>();
    NiceMock<MockRebuildCtx>* reCtx = new NiceMock<MockRebuildCtx>();
    NiceMock<MockGcCtx>* gcCtx = new NiceMock<MockGcCtx>();
    NiceMock<MockBlockAllocationStatus>* blockAllocStatus = new NiceMock<MockBlockAllocationStatus>();
    NiceMock<MockContextIoManager>* ioManager = new NiceMock<MockContextIoManager>;
    NiceMock<MockTelemetryPublisher> tc;
    ContextManager ctxManager(&tc, allocCtx, segCtx, reCtx, gcCtx, blockAllocStatus, ioManager, nullptr, nullptr, 0);

    std::set<SegmentId> segmentList;

    // given 1.
    EXPECT_CALL(*segCtx, MakeRebuildTarget).WillOnce(Return(-1));
    // when 1.
    int ret = ctxManager.MakeRebuildTargetSegmentList(segmentList);
    // then 1.
    EXPECT_EQ(-1, ret);

    // given 2.
    segmentList.clear();
    EXPECT_CALL(*segCtx, MakeRebuildTarget)
        .WillOnce([&](std::set<SegmentId>& segmentList)
        {
            segmentList.emplace(0);
            segmentList.emplace(1);
            return 0;
        });
    // when 1.
    ret = ctxManager.MakeRebuildTargetSegmentList(segmentList);
    // then 1.
    EXPECT_EQ(0, ret);

    std::set<SegmentId> expected = {0, 1};
    EXPECT_EQ(segmentList, expected);
}

TEST(ContextManager, GetRebuildTargetSegmentCount_TestwithFlushOrwithoutFlush)
{
    // given
    NiceMock<MockAllocatorCtx>* allocCtx = new NiceMock<MockAllocatorCtx>();
    NiceMock<MockSegmentCtx>* segCtx = new NiceMock<MockSegmentCtx>();
    NiceMock<MockRebuildCtx>* reCtx = new NiceMock<MockRebuildCtx>();
    NiceMock<MockGcCtx>* gcCtx = new NiceMock<MockGcCtx>();
    NiceMock<MockBlockAllocationStatus>* blockAllocStatus = new NiceMock<MockBlockAllocationStatus>();
    NiceMock<MockContextIoManager>* ioManager = new NiceMock<MockContextIoManager>;
    NiceMock<MockTelemetryPublisher> tc;
    ContextManager ctxManager(&tc, allocCtx, segCtx, reCtx, gcCtx, blockAllocStatus, ioManager, nullptr, nullptr, 0);
    EXPECT_CALL(*segCtx, GetRebuildTargetSegmentCount).WillOnce(Return(7));
    // when
    int ret = ctxManager.GetRebuildTargetSegmentCount();
    // then
    EXPECT_EQ(7, ret);
}

TEST(ContextManager, StopRebuilding_TestwithFlushOrwithoutFlush)
{
    // given
    NiceMock<MockAllocatorCtx>* allocCtx = new NiceMock<MockAllocatorCtx>();
    NiceMock<MockSegmentCtx>* segCtx = new NiceMock<MockSegmentCtx>();
    NiceMock<MockRebuildCtx>* reCtx = new NiceMock<MockRebuildCtx>();
    NiceMock<MockGcCtx>* gcCtx = new NiceMock<MockGcCtx>();
    NiceMock<MockBlockAllocationStatus>* blockAllocStatus = new NiceMock<MockBlockAllocationStatus>();
    NiceMock<MockContextIoManager>* ioManager = new NiceMock<MockContextIoManager>;
    NiceMock<MockTelemetryPublisher> tc;
    ContextManager ctxManager(&tc, allocCtx, segCtx, reCtx, gcCtx, blockAllocStatus, ioManager, nullptr, nullptr, 0);

    // given 1.
    EXPECT_CALL(*segCtx, StopRebuilding).WillOnce(Return(-1));
    // when 1.
    int ret = ctxManager.StopRebuilding();
    // then 1.
    EXPECT_EQ(-1, ret);

    // given 2.
    EXPECT_CALL(*segCtx, StopRebuilding).WillOnce(Return(0));

    // when 1.
    ret = ctxManager.StopRebuilding();
    // then 1.
    EXPECT_EQ(0, ret);
}

} // namespace pos
