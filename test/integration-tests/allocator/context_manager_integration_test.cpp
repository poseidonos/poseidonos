#include <atomic>
#include <condition_variable>
#include <mutex>
#include <thread>

#include "src/allocator/context_manager/context_io_manager.h"
#include "src/allocator/context_manager/context_manager.h"
#include "src/allocator/context_manager/rebuild_ctx/rebuild_ctx.h"
#include "src/include/address_type.h"
#include "src/meta_file_intf/mock_file_intf.h"
#include "src/allocator/context_manager/segment_ctx/segment_ctx.h"

#include "test/integration-tests/allocator/allocator_it_common.h"
#include "test/integration-tests/allocator/address/allocator_address_info_tester.h"
#include "test/unit-tests/allocator/context_manager/allocator_ctx/allocator_ctx_mock.h"
#include "test/unit-tests/allocator/context_manager/allocator_file_io_mock.h"
#include "test/unit-tests/allocator/context_manager/block_allocation_status_mock.h"
#include "test/unit-tests/allocator/context_manager/context_io_manager_mock.h"
#include "test/unit-tests/allocator/context_manager/context_replayer_mock.h"
#include "test/unit-tests/allocator/context_manager/segment_ctx/segment_ctx_mock.h"
#include "test/unit-tests/event_scheduler/event_mock.h"
#include "test/unit-tests/event_scheduler/event_scheduler_mock.h"
#include "test/unit-tests/lib/bitmap_mock.h"
#include "test/unit-tests/meta_file_intf/async_context_mock.h"
#include "test/unit-tests/meta_file_intf/meta_file_intf_mock.h"
#include "src/journal_manager/log_buffer/i_versioned_segment_context.h"
#include "test/unit-tests/journal_manager/status/journal_status_provider_mock.h"

#include "test/unit-tests/array_models/interface/i_array_info_mock.h"
#include "test/unit-tests/bio/volume_io_mock.h"
#include "test/unit-tests/journal_manager/checkpoint/checkpoint_manager_mock.h"
#include "test/unit-tests/journal_manager/checkpoint/dirty_map_manager_mock.h"
#include "test/unit-tests/journal_manager/checkpoint/log_group_releaser_mock.h"

#include "test/unit-tests/journal_manager/journal_writer_mock.h"
#include "test/unit-tests/journal_manager/log_buffer/buffer_write_done_notifier_mock.h"
#include "test/unit-tests/journal_manager/log_buffer/versioned_segment_ctx_mock.h"
#include "test/unit-tests/journal_manager/log_buffer/callback_sequence_controller_mock.h"
#include "test/unit-tests/journal_manager/log_buffer/journal_log_buffer_mock.h"
#include "test/unit-tests/journal_manager/log_buffer/i_journal_log_buffer_mock.h"
#include "test/unit-tests/journal_manager/log_buffer/log_write_context_factory_mock.h"
#include "test/unit-tests/journal_manager/log_write/buffer_offset_allocator_mock.h"
#include "test/unit-tests/journal_manager/log_write/journal_event_factory_mock.h"
#include "test/unit-tests/journal_manager/log_write/journal_volume_event_handler_mock.h"
#include "test/unit-tests/journal_manager/log_write/log_write_handler_mock.h"
#include "test/unit-tests/journal_manager/replay/replay_handler_mock.h"

#include "test/unit-tests/telemetry/telemetry_client/telemetry_client_mock.h"
#include "test/unit-tests/journal_manager/log_buffer/versioned_segment_info_mock.h"
#include "src/journal_manager/journal_manager.h"
#include "test/unit-tests/journal_manager/config/journal_configuration_mock.h"
#include "test/unit-tests/allocator/context_manager/context_manager_mock.h"
#include "test/unit-tests/journal_manager/checkpoint/checkpoint_meta_flush_completed_mock.h"

using namespace ::testing;
using testing::NiceMock;

namespace pos
{
class ContextManagerIntegrationTest : public testing::Test
{
public:
    ContextManagerIntegrationTest(void) {};
    virtual ~ContextManagerIntegrationTest(void) {};

    virtual void SetUp(void);
    virtual void TearDown(void);

protected:
    uint32_t IncreaseVscValidBlockCount(int logGroupId, int segId, int cnt);
    uint32_t DecreaseVscValidBlockCount(int logGroupId, int segId, int cnt);
    uint32_t IncreaseVscOccupiedStripeCount(int logGroupId, int segId, int cnt);

    const uint32_t numOfSegment = 10;
    const uint32_t validBlockCount = 0;
    const uint32_t maxOccupiedStripeCount = 128;
    const uint32_t numOfStripesPerSegment = 10;
    const uint32_t arrayId = 0;
    const uint32_t numLogGroups = 2;

    PartitionLogicalSize partitionLogicalSize;

    JournalManager* journal;
    SegmentInfo* segInfosForSegCtx;
    SegmentCtx* segCtx;
    IVersionedSegmentContext* versionedSegCtx;
    SegmentInfo* loadedSegInfos;
    ContextManager* ctxManager;
    CheckpointHandler* cpHandler;

    NiceMock<MockIArrayInfo>* arrayInfo;

    NiceMock<MockJournalConfiguration>* config;
    NiceMock<MockJournalStatusProvider>* statusProvider;
    NiceMock<MockLogWriteHandler>* logWriteHandler;
    NiceMock<MockJournalWriter>* journalWriter;
    NiceMock<MockLogWriteContextFactory>* logWriteContextFactory;
    NiceMock<MockJournalEventFactory>* journalEventFactory;
    NiceMock<MockJournalVolumeEventHandler>* volumeEventHandler;
    NiceMock<MockIJournalLogBuffer>* logBuffer;
    NiceMock<MockBufferOffsetAllocator>* bufferAllocator;
    NiceMock<MockLogGroupReleaser>* logGroupReleaser;
    NiceMock<MockCheckpointManager>* checkpointManager;

    NiceMock<MockDirtyMapManager>* dirtyMapManager;
    NiceMock<MockLogBufferWriteDoneNotifier>* logFilledNotifier;
    NiceMock<MockReplayHandler>* replayHandler;

    NiceMock<MockTelemetryPublisher>* tp;
    NiceMock<MockTelemetryClient>* tc;
    NiceMock<MockContextManager>* contextManager;

    NiceMock<MockAllocatorCtx>* allocCtx;
    NiceMock<MockRebuildCtx>* reCtx;
    NiceMock<MockGcCtx>* gcCtx;
    NiceMock<MockBlockAllocationStatus>* blockAllocStatus;
    NiceMock<MockContextIoManager>* ioManager;

    NiceMock<MockAllocatorAddressInfo>* addrInfo;
    NiceMock<MockCheckpointMetaFlushCompleted>* checkpointFlushCompleted;

    const int ALLOCATOR_META_ID = 1000;

private:
    void _InitializePartitionSize(void);
};

void
ContextManagerIntegrationTest::SetUp(void)
{
    _InitializePartitionSize();

    arrayInfo = new NiceMock<MockIArrayInfo>;
    config = new NiceMock<MockJournalConfiguration>;

    EXPECT_CALL(*arrayInfo, GetSizeInfo(_)).WillRepeatedly(Return(&partitionLogicalSize));
    ON_CALL(*config, GetNumLogGroups).WillByDefault(Return(numLogGroups));
    EXPECT_CALL(*config, IsEnabled()).WillRepeatedly(Return(true));

    statusProvider = new NiceMock<MockJournalStatusProvider>;
    logWriteHandler = new NiceMock<MockLogWriteHandler>;
    journalWriter = new NiceMock<MockJournalWriter>;
    logWriteContextFactory = new NiceMock<MockLogWriteContextFactory>;
    journalEventFactory = new NiceMock<MockJournalEventFactory>;
    volumeEventHandler = new NiceMock<MockJournalVolumeEventHandler>;
    logBuffer = new NiceMock<MockIJournalLogBuffer>;
    bufferAllocator = new NiceMock<MockBufferOffsetAllocator>;
    logGroupReleaser = new NiceMock<MockLogGroupReleaser>;
    checkpointManager = new NiceMock<MockCheckpointManager>;

    dirtyMapManager = new NiceMock<MockDirtyMapManager>;
    logFilledNotifier = new NiceMock<MockLogBufferWriteDoneNotifier>;
    replayHandler = new NiceMock<MockReplayHandler>;

    tp = new NiceMock<MockTelemetryPublisher>;
    tc = new NiceMock<MockTelemetryClient>;
    contextManager = new NiceMock<MockContextManager>;

    allocCtx = new NiceMock<MockAllocatorCtx>();
    reCtx = new NiceMock<MockRebuildCtx>();
    gcCtx = new NiceMock<MockGcCtx>();
    blockAllocStatus = new NiceMock<MockBlockAllocationStatus>();
    ioManager = new NiceMock<MockContextIoManager>;
    addrInfo = new NiceMock<MockAllocatorAddressInfo>;

    segInfosForSegCtx = new SegmentInfo[numOfSegment](validBlockCount,
        maxOccupiedStripeCount, SegmentState::SSD);
    segCtx = new SegmentCtx(tp, reCtx, addrInfo, gcCtx, arrayId, segInfosForSegCtx);

    journal = new JournalManager(config, statusProvider,
        logWriteContextFactory, journalEventFactory, logWriteHandler,
        volumeEventHandler, journalWriter,
        logBuffer, bufferAllocator, logGroupReleaser, checkpointManager,
        nullptr, dirtyMapManager, logFilledNotifier, replayHandler, arrayInfo, tp);

    versionedSegCtx = journal->GetVersionedSegmentContext();
    loadedSegInfos = new SegmentInfo[numOfSegment]();

    std::vector<std::shared_ptr<VersionedSegmentInfo>> versionedSegmentInfo;
    for (int index = 0; index < numLogGroups; index++)
    {
        std::shared_ptr<VersionedSegmentInfo> input(new VersionedSegmentInfo());
        versionedSegmentInfo.push_back(input);
    }
    versionedSegCtx->Init(config, loadedSegInfos, numOfSegment, versionedSegmentInfo);

    ctxManager = new ContextManager(tp, allocCtx, segCtx, reCtx,
        versionedSegCtx, gcCtx, blockAllocStatus, ioManager, nullptr, nullptr, 0);

    cpHandler = new CheckpointHandler(ALLOCATOR_META_ID);;
    cpHandler->Init(nullptr, ctxManager, nullptr);

    EXPECT_CALL(*contextManager, GetSegmentCtx()).WillRepeatedly(Return(segCtx));

    checkpointFlushCompleted =
        new NiceMock<MockCheckpointMetaFlushCompleted>(cpHandler, ALLOCATOR_META_ID);
}

void
ContextManagerIntegrationTest::TearDown(void)
{
    delete [] segInfosForSegCtx;
    delete [] loadedSegInfos;

    if (nullptr != arrayInfo)
    {
        delete arrayInfo;
    }

    if (nullptr != tc)
    {
        delete tc;
    }

    if (nullptr != journal)
    {
        delete journal;
    }

    if (nullptr != contextManager)
    {
        delete contextManager;
    }

    if (nullptr != ioManager)
    {
        delete ioManager;
    }

    delete cpHandler;
}

void
ContextManagerIntegrationTest::_InitializePartitionSize(void)
{
    partitionLogicalSize.minWriteBlkCnt = 0;
    partitionLogicalSize.blksPerChunk = 4;
    partitionLogicalSize.blksPerStripe = 16;
    partitionLogicalSize.chunksPerStripe = 4;
    partitionLogicalSize.stripesPerSegment = 2;
    partitionLogicalSize.totalStripes = 300;
    partitionLogicalSize.totalSegments = 300;
}

uint32_t
ContextManagerIntegrationTest::IncreaseVscValidBlockCount(int logGroupId, int segId, int cnt)
{
    versionedSegCtx->IncreaseValidBlockCount(logGroupId, segId, cnt);
    return cnt;
}

uint32_t
ContextManagerIntegrationTest::DecreaseVscValidBlockCount(int logGroupId, int segId, int cnt)
{
    versionedSegCtx->DecreaseValidBlockCount(logGroupId, segId, cnt);
    return cnt;
}

uint32_t
ContextManagerIntegrationTest::IncreaseVscOccupiedStripeCount(int logGroupId, int segId, int cnt)
{
    for (int i = 0; i < cnt; i++)
    {
        versionedSegCtx->IncreaseOccupiedStripeCount(logGroupId, segId);
    }
    return cnt;
}

TEST_F(ContextManagerIntegrationTest, DISABLED_GetRebuildTargetSegment_FreeUserDataSegment)
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

    // WbStripeCtx (Mock)
    NiceMock<MockAllocatorCtx>* allocatorCtx = new NiceMock<MockAllocatorCtx>();

    // GcCtx (Mock)
    NiceMock<MockGcCtx>* gcCtx = new NiceMock<MockGcCtx>();

    // ContextReplayer (Mock)
    NiceMock<MockContextReplayer>* contextReplayer = new NiceMock<MockContextReplayer>();

    // BlockAllocationStatus (Mock)
    NiceMock<MockBlockAllocationStatus>* blockAllocStatus = new NiceMock<MockBlockAllocationStatus>();

    // TelemetryPublisher (Mock)
    NiceMock<MockTelemetryPublisher>* telemetryPublisher = new NiceMock<MockTelemetryPublisher>();

    // RebuildCtx (Mock)
    NiceMock<MockRebuildCtx>* rebuildCtx = new NiceMock<MockRebuildCtx>();

    // SegmentCtx (Real)
    SegmentCtx* segmentCtx = new SegmentCtx(nullptr, rebuildCtx, allocatorAddressInfo, gcCtx, 0);

    // Start Test
    for (int i = 0; i < TEST_TRIAL; ++i)
    {
        int nanoSec = std::rand() % 100;
        std::thread th1(&SegmentCtx::GetRebuildTargetSegment, segmentCtx);
        std::this_thread::sleep_for(std::chrono::nanoseconds(nanoSec));
        VirtualBlks blksToInvalidate = {
            .startVsa = {
                .stripeId = 0,
                .offset = 0},
            .numBlks = 1};
        std::thread th2(&SegmentCtx::InvalidateBlks, segmentCtx, blksToInvalidate, false);
        th1.join();
        th2.join();

        VirtualBlks blksToInvalidate2 = {
            .startVsa = {
                .stripeId = 3,
                .offset = 0},
            .numBlks = 1};
        std::thread th3(&SegmentCtx::InvalidateBlks, segmentCtx, blksToInvalidate2, false);
        std::this_thread::sleep_for(std::chrono::nanoseconds(nanoSec));
        std::thread th4(&SegmentCtx::GetRebuildTargetSegment, segmentCtx);
        th3.join();
        th4.join();
    }

    // Clean up
    delete allocatorAddressInfo;
}

TEST_F(ContextManagerIntegrationTest, DISABLED_FlushContexts_FlushRebuildContext)
{
    NiceMock<MockAllocatorAddressInfo> allocatorAddressInfo;
    NiceMock<MockTelemetryPublisher> telemetryPublisher;
    NiceMock<MockEventScheduler> eventScheduler;

    NiceMock<MockAllocatorFileIo>* segmentCtxIo = new NiceMock<MockAllocatorFileIo>;
    NiceMock<MockAllocatorFileIo>* allocatorCtxIo = new NiceMock<MockAllocatorFileIo>;
    NiceMock<MockAllocatorFileIo>* rebuildCtxIo = new NiceMock<MockAllocatorFileIo>;

    ContextIoManager ioManager(&allocatorAddressInfo, &telemetryPublisher, &eventScheduler, segmentCtxIo, allocatorCtxIo, rebuildCtxIo);

    ON_CALL(eventScheduler, EnqueueEvent).WillByDefault([&](EventSmartPtr event) {
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
    EXPECT_CALL(*allocatorCtxIo, Flush)
        .WillOnce([&](AllocatorCtxIoCompletion callback, int dstSectionId, char* externalBuf)
        {
            std::thread allocCtxFlushCallback([&]
            {
                {
                    std::unique_lock<std::mutex> lock(mut);
                    cv.wait(lock, [&] { return (rebuildFlushCompleted == true); });
                }

                numFlushedContexts++;
                cv.notify_all();
            });
            allocCtxFlushCallback.detach();
            return 0;
        });
    AsyncMetaFileIoCtx* segCtxFlush = new AsyncMetaFileIoCtx();
    segCtxFlush->buffer = (char*)(new CtxHeader());
    ((CtxHeader*)(segCtxFlush->buffer))->sig = SegmentCtx::SIG_SEGMENT_CTX;
    EXPECT_CALL(*segmentCtxIo, Flush)
        .WillOnce([&](AllocatorCtxIoCompletion callback, int dstSectionId, char* externalBuf)
        {
            std::thread segCtxFlushCallback([&]
            {
                {
                    std::unique_lock<std::mutex> lock(mut);
                    cv.wait(lock, [&] { return (rebuildFlushCompleted == true); });
                }

                numFlushedContexts++;
                cv.notify_all();
            });
            segCtxFlushCallback.detach();
            return 0;
        });

    // When 1. Flushing contexts started by CheckpointHandler
    ioManager.FlushContexts(checkpointCallback, false);

    // Test set-up for testing FlushRebuildContext
    // Expects flushing segment context to be completed right away
    AsyncMetaFileIoCtx* rebuildCtxFlush = new AsyncMetaFileIoCtx();
    rebuildCtxFlush->buffer = (char*)(new CtxHeader());
    ((CtxHeader*)(rebuildCtxFlush->buffer))->sig = RebuildCtx::SIG_REBUILD_CTX;
    EXPECT_CALL(*rebuildCtxIo, Flush)
        .WillOnce([&](AllocatorCtxIoCompletion callback, int dstSectionId, char* externalBuf)
        {
            callback();

            rebuildFlushCompleted = true;
            cv.notify_all();
            return 0;
        });

    // When 2. Flushing rebuild contexts started
    // ioManager.FlushRebuildContext(nullptr, false);

    // Wait for all flush completed
    {
        std::unique_lock<std::mutex> lock(mut);
        cv.wait(lock, [&] { return (numFlushedContexts == 2); });
    }

    // Then, checkpointCallback should be called
}

TEST_F(ContextManagerIntegrationTest, UpdateSegmentContext_testIfSegmentOverwritten)
{
    // Given
    NiceMock<MockAllocatorAddressInfo> addrInfo;
    uint32_t numSegments = 10;
    ON_CALL(addrInfo, GetnumUserAreaSegments).WillByDefault(Return(numSegments));

    NiceMock<MockTelemetryPublisher> telemetryPublisher;
    NiceMock<MockEventScheduler> eventScheduler;

    NiceMock<MockContextIoManager>* ioManager = new NiceMock<MockContextIoManager>;
    NiceMock<MockAllocatorCtx>* allocatorCtx = new NiceMock<MockAllocatorCtx>;
    NiceMock<MockRebuildCtx>* rebuildCtx = new NiceMock<MockRebuildCtx>;
    NiceMock<MockGcCtx>* gcCtx = new NiceMock<MockGcCtx>;
    NiceMock<MockBlockAllocationStatus>* blockAllocStatus = new NiceMock<MockBlockAllocationStatus>();
    NiceMock<MockContextReplayer>* contextReplayer = new NiceMock<MockContextReplayer>;
    NiceMock<MockTelemetryPublisher> tp;
    SegmentCtx* segmentCtx = new SegmentCtx(&tp, rebuildCtx, &addrInfo, gcCtx, 0);
    ON_CALL(addrInfo, IsUT).WillByDefault(Return(true));

    std::mutex allocatorLock;
    ON_CALL(*allocatorCtx, GetCtxLock).WillByDefault(ReturnRef(allocatorLock));

    ContextManager contextManager(&telemetryPublisher, allocatorCtx, segmentCtx, rebuildCtx, nullptr,
        gcCtx, blockAllocStatus, ioManager, contextReplayer, &addrInfo, ARRAY_ID);
    contextManager.Init();

    // Set valid block count of each segments to 32 for test
    uint32_t maxValidBlkCount = 32;
    ON_CALL(addrInfo, GetblksPerSegment).WillByDefault(Return(maxValidBlkCount));
    ON_CALL(addrInfo, GetstripesPerSegment).WillByDefault(Return(STRIPE_PER_SEGMENT));

    for (SegmentId segId = 0; segId < numSegments; segId++)
    {
        VirtualBlks blks = {
            .startVsa = {
                .stripeId = segId * STRIPE_PER_SEGMENT,
                .offset = 0},
            .numBlks = maxValidBlkCount};
        segmentCtx->ValidateBlks(blks);
    }

    // When : All stripes in each segment is occupied
    ON_CALL(addrInfo, GetstripesPerSegment).WillByDefault(Return(STRIPE_PER_SEGMENT));
    for (StripeId lsid = 0; lsid < STRIPE_PER_SEGMENT * numSegments; lsid++)
    {
        segmentCtx->UpdateOccupiedStripeCount(lsid);
    }

    // Then: State of occupied segments must be SSD
    SegmentState expectedState = SegmentState::SSD;
    for (SegmentId segId = 0; segId < numSegments; segId++)
    {
        SegmentState actualState = segmentCtx->GetSegmentState(segId);
        EXPECT_EQ(expectedState, actualState);
    }

    // When: All of segments is overwritten
    for (SegmentId segId = 0; segId < numSegments; segId++)
    {
        VirtualBlkAddr startVsa = {
            .stripeId = segId * STRIPE_PER_SEGMENT,
            .offset = 0,
        };
        VirtualBlks blks = {
            .startVsa = startVsa,
            .numBlks = maxValidBlkCount,
        };

        segmentCtx->InvalidateBlks(blks, false);
    }

    // Then: State of overwritten segments must be FREE and occupied stripe count is zero
    expectedState = SegmentState::FREE;
    int expectedOccupiedCount = 0;
    for (SegmentId segId = 0; segId < numSegments; segId++)
    {
        SegmentState actualState = segmentCtx->GetSegmentState(segId);
        EXPECT_EQ(expectedState, actualState);

        int actualOccupiedCount = segmentCtx->GetOccupiedStripeCount(segId);
        EXPECT_EQ(expectedOccupiedCount, actualOccupiedCount);
    }
}
} // namespace pos
