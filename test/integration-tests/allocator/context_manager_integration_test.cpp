#include <gtest/gtest.h>

#include "src/allocator/context_manager/context_manager.h"
#include "src/allocator/context_manager/rebuild_ctx/rebuild_ctx.h"
#include "src/meta_file_intf/mock_file_intf.h"

#include "test/integration-tests/allocator/allocator_it_common.h"

#include "test/unit-tests/allocator/address/allocator_address_info_mock.h"
#include "test/unit-tests/allocator/context_manager/block_allocation_status_mock.h"
#include "test/unit-tests/allocator/context_manager/context_replayer_mock.h"
#include "test/unit-tests/allocator/context_manager/file_io_manager_mock.h"
#include "test/unit-tests/allocator/context_manager/allocator_ctx/allocator_ctx_mock.h"
#include "test/unit-tests/allocator/context_manager/rebuild_ctx/rebuild_ctx_mock.h"
#include "test/unit-tests/allocator/context_manager/segment_ctx/segment_lock_mock.h"
#include "test/unit-tests/allocator/context_manager/segment_ctx/segment_states_mock.h"
#include "test/unit-tests/allocator/context_manager/segment_ctx/segment_ctx_mock.h"
#include "test/unit-tests/allocator/context_manager/wbstripe_ctx/wbstripe_ctx_mock.h"
#include "test/unit-tests/allocator/context_manager/gc_ctx/gc_ctx_mock.h"
#include "test/unit-tests/lib/bitmap_mock.h"
#include "test/unit-tests/meta_file_intf/meta_file_intf_mock.h"
#include "test/unit-tests/telemetry/telemetry_client/telemetry_publisher_mock.h"
#include "test/unit-tests/event_scheduler/event_mock.h"
#include "test/unit-tests/meta_file_intf/async_context_mock.h"
#include "test/unit-tests/event_scheduler/event_scheduler_mock.h"

#include <thread>
#include <condition_variable>
#include <mutex>
#include <atomic>

using namespace ::testing;

namespace pos
{

TEST(ContextManagerIntegrationTest, DISABLED_GetRebuildTargetSegment_FreeUserDataSegment)
{
    // Constant
    const int TEST_SEG_CNT = 1;
    const int TEST_TRIAL = 100;

    // AllocatorAddressInfo (Mock)
    NiceMock<MockAllocatorAddressInfo>* allocatorAddressInfo = new NiceMock<MockAllocatorAddressInfo>();
    EXPECT_CALL(*allocatorAddressInfo, GetblksPerStripe).WillRepeatedly(Return(BLK_PER_STRIPE));
    EXPECT_CALL(*allocatorAddressInfo, GetchunksPerStripe).WillRepeatedly(Return(CHUNK_PER_STRIPE));
    EXPECT_CALL(*allocatorAddressInfo, GetnumWbStripes).WillRepeatedly(Return(WB_STRIPE));
    EXPECT_CALL(*allocatorAddressInfo, GetnumUserAreaStripes).WillRepeatedly(Return(USER_STRIPE));
    EXPECT_CALL(*allocatorAddressInfo, GetblksPerSegment).WillRepeatedly(Return(USER_BLOCKS));
    EXPECT_CALL(*allocatorAddressInfo, GetstripesPerSegment).WillRepeatedly(Return(STRIPE_PER_SEGMENT));
    EXPECT_CALL(*allocatorAddressInfo, GetnumUserAreaSegments).WillRepeatedly(Return(TEST_SEG_CNT));
    EXPECT_CALL(*allocatorAddressInfo, IsUT).WillRepeatedly(Return(true));

    // SegmentCtx (Mock)
    NiceMock<MockSegmentCtx>* segmentCtx = new NiceMock<MockSegmentCtx>();
    std::mutex segCtxLock;
    std::mutex segStateLock;
    EXPECT_CALL(*segmentCtx, GetOccupiedStripeCount).WillRepeatedly(Return(STRIPE_PER_SEGMENT));
    EXPECT_CALL(*segmentCtx, GetSegmentCtxLock).WillRepeatedly(ReturnRef(segStateLock));

    // WbStripeCtx (Mock)
    NiceMock<MockWbStripeCtx>* wbStripeCtx = new NiceMock<MockWbStripeCtx>();
    std::mutex allocWbLsidbitMapLock;
    EXPECT_CALL(*wbStripeCtx, GetAllocWbLsidBitmapLock).WillRepeatedly(ReturnRef(allocWbLsidbitMapLock));

    // GcCtx (Mock)
    NiceMock<MockGcCtx>* gcCtx = new NiceMock<MockGcCtx>();

    // ContextReplayer (Mock)
    NiceMock<MockContextReplayer>* contextReplayer = new NiceMock<MockContextReplayer>();

    // BlockAllocationStatus (Mock)
    NiceMock<MockBlockAllocationStatus>* blockAllocStatus = new NiceMock<MockBlockAllocationStatus>();

    // TelemetryPublisher (Mock)
    NiceMock<MockTelemetryPublisher>* telemetryPublisher = new NiceMock<MockTelemetryPublisher>();

    // EventScheduler (Mock)
    NiceMock<MockEventScheduler> eventScheduler;

    // AllocatorFileIoManager (Real)
    MockFileIntf* file = new MockFileIntf("RebuildContext", ARRAY_ID);    // "RebuildContext" file
    MetaFileIntf* fileArr[NUM_FILES];
    for (int i = 0; i < NUM_FILES; ++i)
    {
        fileArr[i] = new NiceMock<MockMetaFileIntf>("fileName", ARRAY_ID);
        if (i == REBUILD_CTX)
        {
            fileArr[REBUILD_CTX] = file;
        }
    }    
    AllocatorFileIoManager* allocatorFileIoManager = new AllocatorFileIoManager(fileArr, allocatorAddressInfo);

    // AllocatorCtx (Real)
    NiceMock<MockBitMapMutex>* allocSegBitmap = new NiceMock<MockBitMapMutex>();
    AllocatorCtx* allocatorCtx = new AllocatorCtx(nullptr, allocSegBitmap, allocatorAddressInfo);

    // RebuildCtx (Real)
    RebuildCtx* rebuildCtx = new RebuildCtx(allocatorCtx, segmentCtx, allocatorAddressInfo);

    // ContextManager (Real)
    ContextManager contextManager(telemetryPublisher, &eventScheduler, allocatorCtx, segmentCtx, rebuildCtx,
                                  wbStripeCtx, gcCtx, blockAllocStatus, allocatorFileIoManager,
                                  contextReplayer, false, allocatorAddressInfo, ARRAY_ID);

    // Prepare Test
    contextManager.Init();
    rebuildCtx->EmplaceRebuildTargetSegment(0);
    rebuildCtx->SetTargetSegmentCnt(TEST_SEG_CNT);

    // Start Test
    for (int i = 0; i < TEST_TRIAL; ++i)
    {
        int nanoSec = std::rand() % 100;
        std::thread th1(&RebuildCtx::GetRebuildTargetSegment, rebuildCtx);
        std::this_thread::sleep_for(std::chrono::nanoseconds(nanoSec));
        std::thread th2(&RebuildCtx::FreeSegmentInRebuildTarget, rebuildCtx, 0);
        th1.join();
        th2.join();

        std::thread th3(&RebuildCtx::FreeSegmentInRebuildTarget, rebuildCtx, 0);
        std::this_thread::sleep_for(std::chrono::nanoseconds(nanoSec));
        std::thread th4(&RebuildCtx::GetRebuildTargetSegment, rebuildCtx);
        th3.join();
        th4.join();
    }

    // Clean up
    delete allocatorAddressInfo;
}

TEST(ContextManagerIntegrationTest, FlushContexts_FlushRebuildContext)
{
    NiceMock<MockTelemetryPublisher> telemetryPublisher;
    NiceMock<MockEventScheduler> eventScheduler;
    NiceMock<MockAllocatorCtx>* allocatorCtx = new NiceMock<MockAllocatorCtx>;
    NiceMock<MockSegmentCtx>* segmentCtx = new NiceMock<MockSegmentCtx>;
    NiceMock<MockRebuildCtx>* rebuildCtx = new NiceMock<MockRebuildCtx>;
    NiceMock<MockWbStripeCtx>* wbStripeCtx = new NiceMock<MockWbStripeCtx>;
    NiceMock<MockGcCtx>* gcCtx = new NiceMock<MockGcCtx>;
    NiceMock<MockBlockAllocationStatus>* blockAllocStatus = new NiceMock<MockBlockAllocationStatus>();
    NiceMock<MockAllocatorFileIoManager>* allocatorFileIoManager = new NiceMock<MockAllocatorFileIoManager>;
    NiceMock<MockContextReplayer>* contextReplayer = new NiceMock<MockContextReplayer>;
    NiceMock<MockAllocatorAddressInfo> allocatorAddressInfo;

    ContextManager contextManager(&telemetryPublisher, &eventScheduler, allocatorCtx, segmentCtx, rebuildCtx,
                                  wbStripeCtx, gcCtx, blockAllocStatus, allocatorFileIoManager,
                                  contextReplayer, false, &allocatorAddressInfo, ARRAY_ID);

    std::mutex allocatorLock, segmentLock, wbLsidBitmapLock;
    EXPECT_CALL(*allocatorCtx, GetAllocatorCtxLock).WillRepeatedly(ReturnRef(allocatorLock));
    EXPECT_CALL(*segmentCtx, GetSegmentCtxLock).WillRepeatedly(ReturnRef(segmentLock));
    EXPECT_CALL(*wbStripeCtx, GetAllocWbLsidBitmapLock).WillRepeatedly(ReturnRef(wbLsidBitmapLock));

    ON_CALL(eventScheduler, EnqueueEvent).WillByDefault([&](EventSmartPtr event)
    {
        event->Execute();
    });

    // Conditional variable to control the test timing
    std::mutex mut;
    std::condition_variable cv;
    bool rebuildFlushCompleted = false;
    std::atomic<int> numFlushedContexts(0);

    // Test set-up for testing FlushContexts
    EventSmartPtr checkpointCallback(new MockEvent());
    EXPECT_CALL(*(MockEvent*)(checkpointCallback.get()), Execute).Times(1);

    // Expects flushing allocator contexts to wait for rebuild context flush done
    AsyncMetaFileIoCtx* allocCtxFlush = new AsyncMetaFileIoCtx();
    allocCtxFlush->buffer = (char*)(new CtxHeader());
    ((CtxHeader*)(allocCtxFlush->buffer))->sig = AllocatorCtx::SIG_ALLOCATOR_CTX;
    EXPECT_CALL(*allocatorFileIoManager, Store(ALLOCATOR_CTX, _, _))
        .WillOnce([&](int owner, char* buf, MetaIoCbPtr callback)
        {
            std::thread allocCtxFlushCallback([&, callback]
            {
                {
                    std::unique_lock<std::mutex> lock(mut);
                    cv.wait(lock, [&]{ return (rebuildFlushCompleted == true); });
                }
                callback(allocCtxFlush);

                numFlushedContexts++;
                cv.notify_all();
            });
            allocCtxFlushCallback.detach();
            return 0;
        });
    AsyncMetaFileIoCtx* segCtxFlush = new AsyncMetaFileIoCtx();
    segCtxFlush->buffer = (char*)(new CtxHeader());
    ((CtxHeader*)(segCtxFlush->buffer))->sig = SegmentCtx::SIG_SEGMENT_CTX;
    EXPECT_CALL(*allocatorFileIoManager, Store(SEGMENT_CTX, _, _))
        .WillOnce([&](int owner, char* buf, MetaIoCbPtr callback)
        {
            std::thread segCtxFlushCallback([&, callback]
            {
                {
                    std::unique_lock<std::mutex> lock(mut);
                    cv.wait(lock, [&]{ return (rebuildFlushCompleted == true); });
                }
                callback(segCtxFlush);

                numFlushedContexts++;
                cv.notify_all();
            });
            segCtxFlushCallback.detach();
            return 0;
        });

    // When 1. Flushing contexts started by CheckpointHandler
    contextManager.FlushContexts(checkpointCallback, false);

    // Test set-up for testing FlushRebuildContext
    // Expects flushing segment context to be completed right away
    AsyncMetaFileIoCtx* rebuildCtxFlush = new AsyncMetaFileIoCtx();
    rebuildCtxFlush->buffer = (char*)(new CtxHeader());
    ((CtxHeader*)(rebuildCtxFlush->buffer))->sig = RebuildCtx::SIG_REBUILD_CTX;
    EXPECT_CALL(*allocatorFileIoManager, Store(REBUILD_CTX, _, _))
        .WillOnce([&](int owner, char* buf, MetaIoCbPtr callback)
        {
            callback(rebuildCtxFlush);

            rebuildFlushCompleted = true;
            cv.notify_all();
            return 0;
        });

    // When 2. Flushing rebuild contexts started
    contextManager.FlushRebuildContext(nullptr, false);

    // Wait for all flush completed
    {
        std::unique_lock<std::mutex> lock(mut);
        cv.wait(lock, [&] { return (numFlushedContexts == 2); });
    }

    // Then, checkpointCallback should be called
}
}   // namespace pos
