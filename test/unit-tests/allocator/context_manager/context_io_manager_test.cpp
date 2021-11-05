#include "src/allocator/context_manager/context_io_manager.h"

#include <gtest/gtest.h>

#include "test/unit-tests/allocator/address/allocator_address_info_mock.h"
#include "test/unit-tests/allocator/context_manager/allocator_file_io_mock.h"
#include "test/unit-tests/event_scheduler/event_scheduler_mock.h"
#include "test/unit-tests/journal_manager/checkpoint/checkpoint_meta_flush_completed_mock.h"
#include "test/unit-tests/telemetry/telemetry_client/telemetry_publisher_mock.h"

using ::testing::NiceMock;
using ::testing::Return;

namespace pos
{
TEST(ContextIoManager, ContextIoManager_testConstructor)
{
    NiceMock<MockAllocatorAddressInfo> info;
    NiceMock<MockTelemetryPublisher> tp;
    NiceMock<MockEventScheduler> scheduler;

    {
        ContextIoManager ioManager(&info, &tp);
    }

    {
        ContextIoManager* ioManager = new ContextIoManager(&info, &tp);
        delete ioManager;
    }

    {
        ContextIoManager ioManager(&info, &tp, &scheduler);
    }

    {
        ContextIoManager* ioManager = new ContextIoManager(&info, &tp, &scheduler);
        delete ioManager;
    }
}

TEST(ContextIoManager, SetAllocatorFileIo_testAdd)
{
    NiceMock<MockAllocatorAddressInfo> info;
    NiceMock<MockTelemetryPublisher> tp;
    ContextIoManager ioManager(&info, &tp);

    NiceMock<MockAllocatorFileIo>* segmentCtxIo = new NiceMock<MockAllocatorFileIo>;
    NiceMock<MockAllocatorFileIo>* allocatorCtxIo = new NiceMock<MockAllocatorFileIo>;
    NiceMock<MockAllocatorFileIo>* rebuildCtxIo = new NiceMock<MockAllocatorFileIo>;

    ioManager.SetAllocatorFileIo(SEGMENT_CTX, segmentCtxIo);
    ioManager.SetAllocatorFileIo(ALLOCATOR_CTX, allocatorCtxIo);
    ioManager.SetAllocatorFileIo(REBUILD_CTX, rebuildCtxIo);
}

TEST(ContextIoManager, Init_testFileCreate)
{
    // given
    NiceMock<MockAllocatorAddressInfo> info;
    NiceMock<MockTelemetryPublisher> tp;
    ContextIoManager ioManager(&info, &tp);

    NiceMock<MockAllocatorFileIo>* segmentCtxIo = new NiceMock<MockAllocatorFileIo>;
    NiceMock<MockAllocatorFileIo>* allocatorCtxIo = new NiceMock<MockAllocatorFileIo>;
    NiceMock<MockAllocatorFileIo>* rebuildCtxIo = new NiceMock<MockAllocatorFileIo>;

    ioManager.SetAllocatorFileIo(SEGMENT_CTX, segmentCtxIo);
    ioManager.SetAllocatorFileIo(ALLOCATOR_CTX, allocatorCtxIo);
    ioManager.SetAllocatorFileIo(REBUILD_CTX, rebuildCtxIo);

    EXPECT_CALL(*segmentCtxIo, Init);
    EXPECT_CALL(*segmentCtxIo, LoadContext).WillOnce(Return(0));
    EXPECT_CALL(*segmentCtxIo, Flush).WillOnce(Return(0));

    EXPECT_CALL(*allocatorCtxIo, Init);
    EXPECT_CALL(*allocatorCtxIo, LoadContext).WillOnce(Return(0));
    EXPECT_CALL(*allocatorCtxIo, Flush).WillOnce(Return(0));

    EXPECT_CALL(*rebuildCtxIo, Init);
    EXPECT_CALL(*rebuildCtxIo, LoadContext).WillOnce(Return(0));
    EXPECT_CALL(*rebuildCtxIo, Flush).WillOnce(Return(0));

    EXPECT_CALL(info, IsUT).WillRepeatedly(Return(true));

    ioManager.Init();
}

TEST(ContextIoManager, Init_testFileLoad)
{
    // given
    NiceMock<MockAllocatorAddressInfo> info;
    NiceMock<MockTelemetryPublisher> tp;
    ContextIoManager ioManager(&info, &tp);

    NiceMock<MockAllocatorFileIo>* segmentCtxIo = new NiceMock<MockAllocatorFileIo>;
    NiceMock<MockAllocatorFileIo>* allocatorCtxIo = new NiceMock<MockAllocatorFileIo>;
    NiceMock<MockAllocatorFileIo>* rebuildCtxIo = new NiceMock<MockAllocatorFileIo>;

    ioManager.SetAllocatorFileIo(SEGMENT_CTX, segmentCtxIo);
    ioManager.SetAllocatorFileIo(ALLOCATOR_CTX, allocatorCtxIo);
    ioManager.SetAllocatorFileIo(REBUILD_CTX, rebuildCtxIo);

    EXPECT_CALL(*segmentCtxIo, Init);
    EXPECT_CALL(*segmentCtxIo, LoadContext).WillOnce(Return(1));

    EXPECT_CALL(*allocatorCtxIo, Init);
    EXPECT_CALL(*allocatorCtxIo, LoadContext).WillOnce(Return(1));

    EXPECT_CALL(*rebuildCtxIo, Init);
    EXPECT_CALL(*rebuildCtxIo, LoadContext).WillOnce(Return(1));

    EXPECT_CALL(info, IsUT).WillOnce(Return(false)).WillOnce(Return(true)).WillOnce(Return(false)).WillOnce(Return(true)).WillOnce(Return(false)).WillOnce(Return(true));
    ioManager.Init();
}

TEST(ContextIoManager, Init_testFileFlushFail)
{
    // given
    NiceMock<MockAllocatorAddressInfo> info;
    NiceMock<MockTelemetryPublisher> tp;
    ContextIoManager ioManager(&info, &tp);

    NiceMock<MockAllocatorFileIo>* segmentCtxIo = new NiceMock<MockAllocatorFileIo>;
    NiceMock<MockAllocatorFileIo>* allocatorCtxIo = new NiceMock<MockAllocatorFileIo>;
    NiceMock<MockAllocatorFileIo>* rebuildCtxIo = new NiceMock<MockAllocatorFileIo>;

    ioManager.SetAllocatorFileIo(SEGMENT_CTX, segmentCtxIo);
    ioManager.SetAllocatorFileIo(ALLOCATOR_CTX, allocatorCtxIo);
    ioManager.SetAllocatorFileIo(REBUILD_CTX, rebuildCtxIo);

    EXPECT_CALL(*segmentCtxIo, Init);
    EXPECT_CALL(*segmentCtxIo, LoadContext).WillOnce(Return(0));
    EXPECT_CALL(*segmentCtxIo, Flush).WillOnce(Return(-1));

    EXPECT_CALL(info, IsUT).WillOnce(Return(false)).WillOnce(Return(true));
    ioManager.Init();
}

TEST(ContextIoManager, Init_testFileLoadFail)
{
    // given
    NiceMock<MockAllocatorAddressInfo> info;
    NiceMock<MockTelemetryPublisher> tp;
    ContextIoManager ioManager(&info, &tp);

    NiceMock<MockAllocatorFileIo>* segmentCtxIo = new NiceMock<MockAllocatorFileIo>;
    NiceMock<MockAllocatorFileIo>* allocatorCtxIo = new NiceMock<MockAllocatorFileIo>;
    NiceMock<MockAllocatorFileIo>* rebuildCtxIo = new NiceMock<MockAllocatorFileIo>;

    ioManager.SetAllocatorFileIo(SEGMENT_CTX, segmentCtxIo);
    ioManager.SetAllocatorFileIo(ALLOCATOR_CTX, allocatorCtxIo);
    ioManager.SetAllocatorFileIo(REBUILD_CTX, rebuildCtxIo);

    EXPECT_CALL(*segmentCtxIo, Init);
    EXPECT_CALL(*segmentCtxIo, LoadContext).WillOnce(Return(-1));

    EXPECT_CALL(info, IsUT).WillOnce(Return(false)).WillOnce(Return(true));
    ioManager.Init();
}

TEST(ContextIoManager, Dispose_testIfAllFileIsDisposed)
{
    NiceMock<MockAllocatorAddressInfo> info;
    NiceMock<MockTelemetryPublisher> tp;
    ContextIoManager ioManager(&info, &tp);

    NiceMock<MockAllocatorFileIo>* segmentCtxIo = new NiceMock<MockAllocatorFileIo>;
    NiceMock<MockAllocatorFileIo>* allocatorCtxIo = new NiceMock<MockAllocatorFileIo>;
    NiceMock<MockAllocatorFileIo>* rebuildCtxIo = new NiceMock<MockAllocatorFileIo>;

    ioManager.SetAllocatorFileIo(SEGMENT_CTX, segmentCtxIo);
    ioManager.SetAllocatorFileIo(ALLOCATOR_CTX, allocatorCtxIo);
    ioManager.SetAllocatorFileIo(REBUILD_CTX, rebuildCtxIo);

    EXPECT_CALL(*segmentCtxIo, Dispose);
    EXPECT_CALL(*allocatorCtxIo, Dispose);
    EXPECT_CALL(*rebuildCtxIo, Dispose);

    ioManager.Dispose();
}

TEST(ContextIoManager, FlushContexts_IfSyncSuccessAllFile)
{
    // given
    AllocatorAddressInfo addrInfo;
    addrInfo.SetUT(true);
    NiceMock<MockAllocatorAddressInfo> info;
    NiceMock<MockTelemetryPublisher> tp;
    ContextIoManager ioManager(&info, &tp);

    NiceMock<MockAllocatorFileIo>* segmentCtxIo = new NiceMock<MockAllocatorFileIo>;
    NiceMock<MockAllocatorFileIo>* allocatorCtxIo = new NiceMock<MockAllocatorFileIo>;
    NiceMock<MockAllocatorFileIo>* rebuildCtxIo = new NiceMock<MockAllocatorFileIo>;
    ioManager.SetAllocatorFileIo(SEGMENT_CTX, segmentCtxIo);
    ioManager.SetAllocatorFileIo(ALLOCATOR_CTX, allocatorCtxIo);
    ioManager.SetAllocatorFileIo(REBUILD_CTX, rebuildCtxIo);

    EXPECT_CALL(*segmentCtxIo, Flush).WillOnce(Return(0));
    EXPECT_CALL(*allocatorCtxIo, Flush).WillOnce(Return(0));

    EXPECT_CALL(info, IsUT).WillRepeatedly(Return(true));

    // when
    EventSmartPtr flushCallback(new MockCheckpointMetaFlushCompleted((CheckpointHandler*)this, 0));
    int ret = ioManager.FlushContexts(flushCallback, true);

    // then
    EXPECT_EQ(0, ret);
}

TEST(ContextIoManager, FlushContexts_IfSyncFailFirstFile)
{
    // given
    AllocatorAddressInfo addrInfo;
    addrInfo.SetUT(true);
    NiceMock<MockAllocatorAddressInfo> info;
    NiceMock<MockTelemetryPublisher> tp;
    ContextIoManager ioManager(&info, &tp);

    NiceMock<MockAllocatorFileIo>* segmentCtxIo = new NiceMock<MockAllocatorFileIo>;
    NiceMock<MockAllocatorFileIo>* allocatorCtxIo = new NiceMock<MockAllocatorFileIo>;
    NiceMock<MockAllocatorFileIo>* rebuildCtxIo = new NiceMock<MockAllocatorFileIo>;
    ioManager.SetAllocatorFileIo(SEGMENT_CTX, segmentCtxIo);
    ioManager.SetAllocatorFileIo(ALLOCATOR_CTX, allocatorCtxIo);
    ioManager.SetAllocatorFileIo(REBUILD_CTX, rebuildCtxIo);

    EXPECT_CALL(*segmentCtxIo, Flush).WillOnce(Return(-1));

    EXPECT_CALL(info, IsUT).WillRepeatedly(Return(true));

    // when
    EventSmartPtr flushCallback(new MockCheckpointMetaFlushCompleted((CheckpointHandler*)this, 0));
    int ret = ioManager.FlushContexts(flushCallback, true);

    // then
    EXPECT_LE(-1, ret);
}

TEST(ContextIoManager, FlushContexts_IfSyncFailSecondFile)
{
    // given
    AllocatorAddressInfo addrInfo;
    addrInfo.SetUT(true);
    NiceMock<MockTelemetryPublisher> tp;
    ContextIoManager ioManager(&addrInfo, &tp);

    NiceMock<MockAllocatorFileIo>* segmentCtxIo = new NiceMock<MockAllocatorFileIo>;
    NiceMock<MockAllocatorFileIo>* allocatorCtxIo = new NiceMock<MockAllocatorFileIo>;
    NiceMock<MockAllocatorFileIo>* rebuildCtxIo = new NiceMock<MockAllocatorFileIo>;
    ioManager.SetAllocatorFileIo(SEGMENT_CTX, segmentCtxIo);
    ioManager.SetAllocatorFileIo(ALLOCATOR_CTX, allocatorCtxIo);
    ioManager.SetAllocatorFileIo(REBUILD_CTX, rebuildCtxIo);

    EXPECT_CALL(*segmentCtxIo, Flush).WillOnce(Return(0));
    EXPECT_CALL(*allocatorCtxIo, Flush).WillOnce(Return(-1));

    // when
    EventSmartPtr flushCallback(new MockCheckpointMetaFlushCompleted((CheckpointHandler*)this, 0));
    int ret = ioManager.FlushContexts(flushCallback, true);

    // then
    EXPECT_LE(-1, ret);
}

TEST(ContextIoManager, FlushContexts_IfAsyncAlreadyFlushing)
{
    // given
    NiceMock<MockAllocatorAddressInfo> info;
    NiceMock<MockTelemetryPublisher> tp;
    ContextIoManager ioManager(&info, &tp);

    NiceMock<MockAllocatorFileIo>* segmentCtxIo = new NiceMock<MockAllocatorFileIo>;
    NiceMock<MockAllocatorFileIo>* allocatorCtxIo = new NiceMock<MockAllocatorFileIo>;
    NiceMock<MockAllocatorFileIo>* rebuildCtxIo = new NiceMock<MockAllocatorFileIo>;
    ioManager.SetAllocatorFileIo(SEGMENT_CTX, segmentCtxIo);
    ioManager.SetAllocatorFileIo(ALLOCATOR_CTX, allocatorCtxIo);
    ioManager.SetAllocatorFileIo(REBUILD_CTX, rebuildCtxIo);

    EXPECT_CALL(*segmentCtxIo, Flush).WillOnce(Return(0));
    EXPECT_CALL(*allocatorCtxIo, Flush).WillOnce(Return(0));

    EXPECT_CALL(info, IsUT).WillRepeatedly(Return(true));

    // when 1
    EventSmartPtr flushCallback(new MockCheckpointMetaFlushCompleted((CheckpointHandler*)this, 0));
    int ret = ioManager.FlushContexts(flushCallback, false);

    // then
    EXPECT_EQ(0, ret);

    // when 2
    ret = ioManager.FlushContexts(nullptr, false);
    // then
    EXPECT_EQ((int)POS_EVENT_ID::ALLOCATOR_META_ARCHIVE_FLUSH_IN_PROGRESS, ret);
}

TEST(ContextIoManager, FlushContexts_IfAsyncSuccessAllFile)
{
    // given
    NiceMock<MockAllocatorAddressInfo> info;
    NiceMock<MockTelemetryPublisher> tp;
    ContextIoManager ioManager(&info, &tp);

    NiceMock<MockAllocatorFileIo>* segmentCtxIo = new NiceMock<MockAllocatorFileIo>;
    NiceMock<MockAllocatorFileIo>* allocatorCtxIo = new NiceMock<MockAllocatorFileIo>;
    NiceMock<MockAllocatorFileIo>* rebuildCtxIo = new NiceMock<MockAllocatorFileIo>;
    ioManager.SetAllocatorFileIo(SEGMENT_CTX, segmentCtxIo);
    ioManager.SetAllocatorFileIo(ALLOCATOR_CTX, allocatorCtxIo);
    ioManager.SetAllocatorFileIo(REBUILD_CTX, rebuildCtxIo);

    EXPECT_CALL(*segmentCtxIo, Flush).WillOnce(Return(0));
    EXPECT_CALL(*allocatorCtxIo, Flush).WillOnce(Return(0));

    EXPECT_CALL(info, IsUT).WillRepeatedly(Return(true));

    // when
    int ret = ioManager.FlushContexts(nullptr, false);

    // then
    EXPECT_EQ(0, ret);
}

TEST(ContextIoManager, FlushContexts_IfAsyncFailFirstFile)
{
    // given
    NiceMock<MockAllocatorAddressInfo> info;
    NiceMock<MockTelemetryPublisher> tp;
    ContextIoManager ioManager(&info, &tp);

    ON_CALL(info, IsUT).WillByDefault(Return(true));

    NiceMock<MockAllocatorFileIo>* segmentCtxIo = new NiceMock<MockAllocatorFileIo>;
    NiceMock<MockAllocatorFileIo>* allocatorCtxIo = new NiceMock<MockAllocatorFileIo>;
    NiceMock<MockAllocatorFileIo>* rebuildCtxIo = new NiceMock<MockAllocatorFileIo>;
    ioManager.SetAllocatorFileIo(SEGMENT_CTX, segmentCtxIo);
    ioManager.SetAllocatorFileIo(ALLOCATOR_CTX, allocatorCtxIo);
    ioManager.SetAllocatorFileIo(REBUILD_CTX, rebuildCtxIo);

    EXPECT_CALL(*segmentCtxIo, Flush).WillOnce(Return(-1));

    EXPECT_CALL(info, IsUT).WillRepeatedly(Return(true));

    // when
    int ret = ioManager.FlushContexts(nullptr, false);

    // then
    EXPECT_LE(-1, ret);
}

TEST(ContextIoManager, FlushContexts_IfAsyncFailSecondFile)
{
    // given
    NiceMock<MockAllocatorAddressInfo> info;
    NiceMock<MockTelemetryPublisher> tp;
    ContextIoManager ioManager(&info, &tp);

    ON_CALL(info, IsUT).WillByDefault(Return(true));

    NiceMock<MockAllocatorFileIo>* segmentCtxIo = new NiceMock<MockAllocatorFileIo>;
    NiceMock<MockAllocatorFileIo>* allocatorCtxIo = new NiceMock<MockAllocatorFileIo>;
    NiceMock<MockAllocatorFileIo>* rebuildCtxIo = new NiceMock<MockAllocatorFileIo>;
    ioManager.SetAllocatorFileIo(SEGMENT_CTX, segmentCtxIo);
    ioManager.SetAllocatorFileIo(ALLOCATOR_CTX, allocatorCtxIo);
    ioManager.SetAllocatorFileIo(REBUILD_CTX, rebuildCtxIo);

    EXPECT_CALL(*segmentCtxIo, Flush).WillOnce(Return(0));
    EXPECT_CALL(*allocatorCtxIo, Flush).WillOnce(Return(-1));

    EXPECT_CALL(info, IsUT).WillRepeatedly(Return(true));

    // when
    int ret = ioManager.FlushContexts(nullptr, false);

    // then
    EXPECT_LE(-1, ret);
}

TEST(ContextIoManager, FlushRebuildContext_testSyncFlush)
{
    NiceMock<MockAllocatorAddressInfo> info;
    NiceMock<MockTelemetryPublisher> tp;
    ContextIoManager ioManager(&info, &tp);

    NiceMock<MockAllocatorFileIo>* rebuildCtxIo = new NiceMock<MockAllocatorFileIo>;
    ioManager.SetAllocatorFileIo(REBUILD_CTX, rebuildCtxIo);

    EXPECT_CALL(info, IsUT).WillRepeatedly(Return(true));

    EXPECT_CALL(*rebuildCtxIo, Flush).WillOnce(Return(0));
    int ret = ioManager.FlushRebuildContext(nullptr, true);
    EXPECT_EQ(ret, 0);
}

TEST(ContextIoManager, FlushRebuildContext_testAsyncFlush)
{
    NiceMock<MockAllocatorAddressInfo> info;
    NiceMock<MockTelemetryPublisher> tp;
    ContextIoManager ioManager(&info, &tp);

    NiceMock<MockAllocatorFileIo>* rebuildCtxIo = new NiceMock<MockAllocatorFileIo>;
    ioManager.SetAllocatorFileIo(REBUILD_CTX, rebuildCtxIo);

    EXPECT_CALL(*rebuildCtxIo, Flush).WillOnce(Return(0));
    int ret = ioManager.FlushRebuildContext(nullptr, false);
    EXPECT_EQ(ret, 0);
}

TEST(ContextIoManager, GetStoredContextVersion_TestGetter)
{
    NiceMock<MockAllocatorAddressInfo> info;
    NiceMock<MockTelemetryPublisher> tp;
    ContextIoManager ioManager(&info, &tp);

    NiceMock<MockAllocatorFileIo>* rebuildCtxIo = new NiceMock<MockAllocatorFileIo>;
    ioManager.SetAllocatorFileIo(REBUILD_CTX, rebuildCtxIo);

    EXPECT_CALL(*rebuildCtxIo, GetStoredVersion).WillOnce(Return(10));
    EXPECT_EQ(ioManager.GetStoredContextVersion(REBUILD_CTX), 10);
}

TEST(ContextIoManager, GetContextSectionAddr_TestGetter)
{
    NiceMock<MockAllocatorAddressInfo> info;
    NiceMock<MockTelemetryPublisher> tp;
    ContextIoManager ioManager(&info, &tp);

    NiceMock<MockAllocatorFileIo>* allocatorCtxIo = new NiceMock<MockAllocatorFileIo>;
    ioManager.SetAllocatorFileIo(ALLOCATOR_CTX, allocatorCtxIo);

    char buffer[100];
    EXPECT_CALL(*allocatorCtxIo, GetSectionAddr(AC_CURRENT_SSD_LSID)).WillOnce(Return(buffer));
    EXPECT_EQ(ioManager.GetContextSectionAddr(ALLOCATOR_CTX, AC_CURRENT_SSD_LSID), buffer);
}

TEST(ContextIoManager, GetContextSectionSize_TestGetter)
{
    NiceMock<MockAllocatorAddressInfo> info;
    NiceMock<MockTelemetryPublisher> tp;
    ContextIoManager ioManager(&info, &tp);

    NiceMock<MockAllocatorFileIo>* segmentCtxIo = new NiceMock<MockAllocatorFileIo>;
    ioManager.SetAllocatorFileIo(SEGMENT_CTX, segmentCtxIo);

    EXPECT_CALL(*segmentCtxIo, GetSectionSize(SC_SEGMENT_INFO)).WillOnce(Return(1024*1024));
    EXPECT_EQ(ioManager.GetContextSectionSize(SEGMENT_CTX, SC_SEGMENT_INFO), 1024*1024);
}

TEST(ContextIoManager, TestCallbackFunc_TestFlushCallback)
{
    // given
    NiceMock<MockAllocatorAddressInfo> info;
    NiceMock<MockTelemetryPublisher> tp;
    NiceMock<MockEventScheduler> scheduler;
    ContextIoManager ioManager(&info, &tp, &scheduler);

    NiceMock<MockAllocatorFileIo>* segmentCtxIo = new NiceMock<MockAllocatorFileIo>;
    NiceMock<MockAllocatorFileIo>* allocatorCtxIo = new NiceMock<MockAllocatorFileIo>;
    NiceMock<MockAllocatorFileIo>* rebuildCtxIo = new NiceMock<MockAllocatorFileIo>;

    ioManager.SetAllocatorFileIo(SEGMENT_CTX, segmentCtxIo);
    ioManager.SetAllocatorFileIo(ALLOCATOR_CTX, allocatorCtxIo);
    ioManager.SetAllocatorFileIo(REBUILD_CTX, rebuildCtxIo);

    EventSmartPtr flushCallback(new MockCheckpointMetaFlushCompleted((CheckpointHandler*)this, 0));
    ioManager.SetCallbackFunc(flushCallback);

    // given 1.
    char* buf = new char[100];
    AsyncMetaFileIoCtx* ctx = new AsyncMetaFileIoCtx();
    ctx->buffer = buf;

    // given 1.
    reinterpret_cast<CtxHeader*>(buf)->sig = SegmentCtx::SIG_SEGMENT_CTX;
    EXPECT_CALL(*segmentCtxIo, AfterFlush);
    // when 1.
    ioManager.TestCallbackFunc(ctx, ContextIoManager::IOTYPE_FLUSH, 2);

    // given 2.
    buf = new char[100];
    ctx = new AsyncMetaFileIoCtx();
    ctx->buffer = buf;
    reinterpret_cast<CtxHeader*>(buf)->sig = AllocatorCtx::SIG_ALLOCATOR_CTX;
    EXPECT_CALL(*allocatorCtxIo, AfterFlush);
    EXPECT_CALL(scheduler, EnqueueEvent(flushCallback));

    // when 2.
    ioManager.TestCallbackFunc(ctx, ContextIoManager::IOTYPE_FLUSH, 1);

    // given 3.
    buf = new char[100];
    ctx = new AsyncMetaFileIoCtx();
    ctx->buffer = buf;
    reinterpret_cast<CtxHeader*>(buf)->sig = RebuildCtx::SIG_REBUILD_CTX;
    EXPECT_CALL(*rebuildCtxIo, AfterFlush);
    // when 3.
    ioManager.TestCallbackFunc(ctx, ContextIoManager::IOTYPE_REBUILDFLUSH, 1);
}

TEST(ContextIoManager, TestCallbackFunc_TestLoadCallback)
{
    // given
    NiceMock<MockAllocatorAddressInfo> info;
    NiceMock<MockTelemetryPublisher> tp;
    ContextIoManager ioManager(&info, &tp);

    NiceMock<MockAllocatorFileIo>* segmentCtxIo = new NiceMock<MockAllocatorFileIo>;
    NiceMock<MockAllocatorFileIo>* allocatorCtxIo = new NiceMock<MockAllocatorFileIo>;
    NiceMock<MockAllocatorFileIo>* rebuildCtxIo = new NiceMock<MockAllocatorFileIo>;

    ioManager.SetAllocatorFileIo(SEGMENT_CTX, segmentCtxIo);
    ioManager.SetAllocatorFileIo(ALLOCATOR_CTX, allocatorCtxIo);
    ioManager.SetAllocatorFileIo(REBUILD_CTX, rebuildCtxIo);

    // given 1.
    char* buf = new char[100];
    AsyncMetaFileIoCtx* ctx = new AsyncMetaFileIoCtx();
    ctx->buffer = buf;

    // given 1.
    reinterpret_cast<CtxHeader*>(buf)->sig = SegmentCtx::SIG_SEGMENT_CTX;
    EXPECT_CALL(*segmentCtxIo, AfterLoad);
    // when 1.
    ioManager.TestCallbackFunc(ctx, ContextIoManager::IOTYPE_READ, 1);

    // given 2.
    buf = new char[100];
    ctx = new AsyncMetaFileIoCtx();
    ctx->buffer = buf;
    reinterpret_cast<CtxHeader*>(buf)->sig = AllocatorCtx::SIG_ALLOCATOR_CTX;
    EXPECT_CALL(*allocatorCtxIo, AfterLoad);
    // when 2.
    ioManager.TestCallbackFunc(ctx, ContextIoManager::IOTYPE_READ, 1);

    // given 3.
    buf = new char[100];
    ctx = new AsyncMetaFileIoCtx();
    ctx->buffer = buf;
    reinterpret_cast<CtxHeader*>(buf)->sig = RebuildCtx::SIG_REBUILD_CTX;
    EXPECT_CALL(*rebuildCtxIo, AfterLoad);
    // when 3.
    ioManager.TestCallbackFunc(ctx, ContextIoManager::IOTYPE_READ, 1);

    // given 4.
    buf = new char[100];
    ctx = new AsyncMetaFileIoCtx();
    ctx->buffer = buf;
    reinterpret_cast<CtxHeader*>(buf)->sig = 0;
    EXPECT_CALL(info, IsUT).WillOnce(Return(false)).WillOnce(Return(true));
    // when 4.
    ioManager.TestCallbackFunc(ctx, ContextIoManager::IOTYPE_READ, 1);
}

TEST(ContextIoManager, TestCallbackFunc_TestWaitPendingAll)
{
    // given
    NiceMock<MockAllocatorAddressInfo> info;
    NiceMock<MockTelemetryPublisher> tp;
    ContextIoManager ioManager(&info, &tp);

    NiceMock<MockAllocatorFileIo>* segmentCtxIo = new NiceMock<MockAllocatorFileIo>;
    NiceMock<MockAllocatorFileIo>* allocatorCtxIo = new NiceMock<MockAllocatorFileIo>;
    NiceMock<MockAllocatorFileIo>* rebuildCtxIo = new NiceMock<MockAllocatorFileIo>;

    ioManager.SetAllocatorFileIo(SEGMENT_CTX, segmentCtxIo);
    ioManager.SetAllocatorFileIo(ALLOCATOR_CTX, allocatorCtxIo);
    ioManager.SetAllocatorFileIo(REBUILD_CTX, rebuildCtxIo);

    EXPECT_CALL(info, IsUT).WillOnce(Return(false)).WillOnce(Return(true));

    // when
    ioManager.TestCallbackFunc(nullptr, ContextIoManager::IOTYPE_ALL, 1);
}

} // namespace pos
