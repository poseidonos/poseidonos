#include "src/allocator/context_manager/context_manager.h"

#include <gtest/gtest.h>

#include <mutex>

#include "test/unit-tests/allocator/context_manager/allocator_ctx/allocator_ctx_mock.h"
#include "test/unit-tests/allocator/context_manager/block_allocation_status_mock.h"
#include "test/unit-tests/allocator/context_manager/context_io_manager_mock.h"
#include "test/unit-tests/allocator/context_manager/context_manager_mock.h"
#include "test/unit-tests/allocator/context_manager/context_replayer_mock.h"
#include "test/unit-tests/allocator/context_manager/i_allocator_file_io_client_mock.h"
#include "test/unit-tests/allocator/context_manager/segment_ctx/segment_ctx_mock.h"
#include "test/unit-tests/journal_manager/checkpoint/checkpoint_meta_flush_completed_mock.h"
#include "test/unit-tests/journal_manager/config/journal_configuration_mock.h"
#include "test/unit-tests/journal_manager/log_buffer/versioned_segment_info_mock.h"
#include "src/journal_manager/journal_manager.h"
#include "src/journal_manager/log_buffer/i_versioned_segment_context.h"
#include "test/unit-tests/journal_manager/status/journal_status_provider_mock.h"
#include "test/unit-tests/journal_manager/log_buffer/versioned_segment_ctx_mock.h"

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
    ContextManager ctxManager(&tc, allocCtx, segCtx, reCtx, nullptr, gcCtx, blockAllocStatus,
        ioManager, nullptr, &addrInfo, 0);

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
    ContextManager ctxManager(&tc, allocCtx, segCtx, reCtx, nullptr, gcCtx, blockAllocStatus, ioManager, nullptr, &addrInfo, 0);

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
    NiceMock<MockVersionedSegmentCtx>* vscCtx = new NiceMock<MockVersionedSegmentCtx>();
    ContextManager ctxManager(&tc, allocCtx, segCtx, reCtx, vscCtx, gcCtx, blockAllocStatus, ioManager, nullptr, nullptr, 0);

    EXPECT_CALL(*ioManager, FlushContexts).WillOnce(Return(0));

    // when
    int ret = ctxManager.FlushContexts(nullptr, false);
    // then
    EXPECT_LE(0, ret);
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
    ContextManager ctxManager(&tc, allocCtx, segCtx, reCtx, nullptr, gcCtx, blockAllocStatus, ioManager, nullptr, nullptr, 0);

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
    ContextManager ctxManager(&tc, allocCtx, segCtx, reCtx, nullptr, gcCtx, blockAllocStatus, ioManager, nullptr, nullptr, 0);

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
    ContextManager ctxManager(&tc, allocCtx, segCtx, reCtx, nullptr, gcCtx, blockAllocStatus, ioManager, nullptr, nullptr, 0);

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
    ContextManager ctxManager(&tc, allocCtx, segCtx, reCtx, nullptr, gcCtx, blockAllocStatus, ioManager, nullptr, nullptr, 0);

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
    ContextManager ctxManager(&tc, allocCtx, segCtx, reCtx, nullptr, gcCtx, blockAllocStatus, ioManager, nullptr, nullptr, 0);

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
    ContextManager ctxManager(&tc, allocCtx, segCtx, reCtx, nullptr, gcCtx, blockAllocStatus, ioManager, nullptr, nullptr, 0);

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
    ContextManager ctxManager(&tc, allocCtx, segCtx, reCtx, nullptr, gcCtx, blockAllocStatus, ioManager, nullptr, nullptr, 0);

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
    ContextManager ctxManager(&tc, allocCtx, segCtx, reCtx, nullptr, gcCtx, blockAllocStatus, ioManager, nullptr, nullptr, 0);

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
    ContextManager ctxManager(&tc, allocCtx, segCtx, reCtx, nullptr, gcCtx, blockAllocStatus, ioManager, nullptr, nullptr, 0);

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
    ContextManager ctxManager(&tc, allocCtx, segCtx, reCtx, nullptr, gcCtx, blockAllocStatus, ioManager, nullptr, nullptr, 0);

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
    ContextManager ctxManager(&tc, allocCtx, segCtx, reCtx, nullptr, gcCtx, blockAllocStatus, ioManager, nullptr, nullptr, 0);
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
    ContextManager ctxManager(&tc, allocCtx, segCtx, reCtx, nullptr, gcCtx, blockAllocStatus, ioManager, nullptr, nullptr, 0);
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
    ContextManager ctxManager(&tc, allocCtx, segCtx, reCtx, nullptr, gcCtx, blockAllocStatus, ioManager, ctxReplayer, nullptr, 0);
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
    ContextManager ctxManager(&tc, allocCtx, segCtx, reCtx, nullptr, gcCtx, blockAllocStatus, ioManager, nullptr, nullptr, 0);

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
    EXPECT_EQ((int)ERRID(ALLOCATOR_NO_FREE_SEGMENT), ret);
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
    ContextManager ctxManager(&tc, allocCtx, segCtx, reCtx, nullptr, gcCtx, blockAllocStatus, ioManager, nullptr, nullptr, 0);
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
    ContextManager ctxManager(&tc, allocCtx, segCtx, reCtx, nullptr, gcCtx, blockAllocStatus, ioManager, nullptr, nullptr, 0);
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
    ContextManager ctxManager(&tc, allocCtx, segCtx, reCtx, nullptr, gcCtx, blockAllocStatus, ioManager, nullptr, nullptr, 0);
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
    ContextManager ctxManager(&tc, allocCtx, segCtx, reCtx, nullptr, gcCtx, blockAllocStatus, ioManager, nullptr, nullptr, 0);
    // when
    ctxManager.NeedRebuildAgain();
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
    ContextManager ctxManager(&tc, allocCtx, segCtx, reCtx, nullptr, gcCtx, blockAllocStatus, ioManager, nullptr, nullptr, 0);

    // given 1.
    EXPECT_CALL(*segCtx, MakeRebuildTarget).WillOnce(Return(-1));
    // when 1.
    int ret = ctxManager.MakeRebuildTargetSegmentList();
    // then 1.
    EXPECT_EQ(-1, ret);

    // given 2.
    EXPECT_CALL(*segCtx, MakeRebuildTarget).WillOnce(Return(0));
    // when 1.
    ret = ctxManager.MakeRebuildTargetSegmentList();
    // then 1.
    EXPECT_EQ(0, ret);
}

TEST(ContextManager, GetNvramSegmentList_testIfListIsReturnedAsSegmentContextReturned)
{
    // given
    NiceMock<MockAllocatorCtx>* allocCtx = new NiceMock<MockAllocatorCtx>();
    NiceMock<MockSegmentCtx>* segCtx = new NiceMock<MockSegmentCtx>();
    NiceMock<MockRebuildCtx>* reCtx = new NiceMock<MockRebuildCtx>();
    NiceMock<MockGcCtx>* gcCtx = new NiceMock<MockGcCtx>();
    NiceMock<MockBlockAllocationStatus>* blockAllocStatus = new NiceMock<MockBlockAllocationStatus>();
    NiceMock<MockContextIoManager>* ioManager = new NiceMock<MockContextIoManager>;
    NiceMock<MockTelemetryPublisher> tc;
    ContextManager ctxManager(&tc, allocCtx, segCtx, reCtx, nullptr, gcCtx, blockAllocStatus, ioManager, nullptr, nullptr, 0);

    // given
    EXPECT_CALL(*segCtx, GetNvramSegmentList)
        .WillOnce([]()
        {
            std::set<SegmentId> sets = {0, 1, 2};
            return sets;
        });
    // when
    auto actual = ctxManager.GetNvramSegmentList();
    std::set<SegmentId> expected = {0, 1, 2};
    // then
    EXPECT_TRUE(actual == expected);
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
    ContextManager ctxManager(&tc, allocCtx, segCtx, reCtx, nullptr, gcCtx, blockAllocStatus, ioManager, nullptr, nullptr, 0);
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
    ContextManager ctxManager(&tc, allocCtx, segCtx, reCtx, nullptr, gcCtx, blockAllocStatus, ioManager, nullptr, nullptr, 0);

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
