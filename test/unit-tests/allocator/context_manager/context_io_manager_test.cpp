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
        NiceMock<MockAllocatorFileIo>* segmentCtxIo = new NiceMock<MockAllocatorFileIo>;
        NiceMock<MockAllocatorFileIo>* allocatorCtxIo = new NiceMock<MockAllocatorFileIo>;
        NiceMock<MockAllocatorFileIo>* rebuildCtxIo = new NiceMock<MockAllocatorFileIo>;
        ContextIoManager ioManager(&info, &tp, segmentCtxIo, allocatorCtxIo, rebuildCtxIo);
    }

    {
        NiceMock<MockAllocatorFileIo>* segmentCtxIo = new NiceMock<MockAllocatorFileIo>;
        NiceMock<MockAllocatorFileIo>* allocatorCtxIo = new NiceMock<MockAllocatorFileIo>;
        NiceMock<MockAllocatorFileIo>* rebuildCtxIo = new NiceMock<MockAllocatorFileIo>;
        ContextIoManager* ioManager = new ContextIoManager(&info, &tp, segmentCtxIo, allocatorCtxIo, rebuildCtxIo);
        delete ioManager;
    }

    {
        NiceMock<MockAllocatorFileIo>* segmentCtxIo = new NiceMock<MockAllocatorFileIo>;
        NiceMock<MockAllocatorFileIo>* allocatorCtxIo = new NiceMock<MockAllocatorFileIo>;
        NiceMock<MockAllocatorFileIo>* rebuildCtxIo = new NiceMock<MockAllocatorFileIo>;
        ContextIoManager ioManager(&info, &tp, &scheduler, segmentCtxIo, allocatorCtxIo, rebuildCtxIo);
    }

    {
        NiceMock<MockAllocatorFileIo>* segmentCtxIo = new NiceMock<MockAllocatorFileIo>;
        NiceMock<MockAllocatorFileIo>* allocatorCtxIo = new NiceMock<MockAllocatorFileIo>;
        NiceMock<MockAllocatorFileIo>* rebuildCtxIo = new NiceMock<MockAllocatorFileIo>;
        ContextIoManager* ioManager = new ContextIoManager(&info, &tp, &scheduler, segmentCtxIo, allocatorCtxIo, rebuildCtxIo);
        delete ioManager;
    }
}

TEST(ContextIoManager, Init_testFileCreate)
{
    // given
    NiceMock<MockAllocatorAddressInfo> info;
    NiceMock<MockTelemetryPublisher> tp;

    NiceMock<MockAllocatorFileIo>* segmentCtxIo = new NiceMock<MockAllocatorFileIo>;
    NiceMock<MockAllocatorFileIo>* allocatorCtxIo = new NiceMock<MockAllocatorFileIo>;
    NiceMock<MockAllocatorFileIo>* rebuildCtxIo = new NiceMock<MockAllocatorFileIo>;

    ContextIoManager ioManager(&info, &tp, segmentCtxIo, allocatorCtxIo, rebuildCtxIo);

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

    NiceMock<MockAllocatorFileIo>* segmentCtxIo = new NiceMock<MockAllocatorFileIo>;
    NiceMock<MockAllocatorFileIo>* allocatorCtxIo = new NiceMock<MockAllocatorFileIo>;
    NiceMock<MockAllocatorFileIo>* rebuildCtxIo = new NiceMock<MockAllocatorFileIo>;

    ContextIoManager ioManager(&info, &tp, segmentCtxIo, allocatorCtxIo, rebuildCtxIo);

    EXPECT_CALL(*segmentCtxIo, Init);
    EXPECT_CALL(*segmentCtxIo, LoadContext).WillOnce(Return(1));

    EXPECT_CALL(*allocatorCtxIo, Init);
    EXPECT_CALL(*allocatorCtxIo, LoadContext).WillOnce(Return(1));

    EXPECT_CALL(*rebuildCtxIo, Init);
    EXPECT_CALL(*rebuildCtxIo, LoadContext).WillOnce(Return(1));

    ioManager.Init();
}

TEST(ContextIoManager, Init_testFileFlushFail)
{
    // given
    NiceMock<MockAllocatorAddressInfo> info;
    NiceMock<MockTelemetryPublisher> tp;

    NiceMock<MockAllocatorFileIo>* segmentCtxIo = new NiceMock<MockAllocatorFileIo>;
    NiceMock<MockAllocatorFileIo>* allocatorCtxIo = new NiceMock<MockAllocatorFileIo>;
    NiceMock<MockAllocatorFileIo>* rebuildCtxIo = new NiceMock<MockAllocatorFileIo>;

    ContextIoManager ioManager(&info, &tp, segmentCtxIo, allocatorCtxIo, rebuildCtxIo);

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

    NiceMock<MockAllocatorFileIo>* segmentCtxIo = new NiceMock<MockAllocatorFileIo>;
    NiceMock<MockAllocatorFileIo>* allocatorCtxIo = new NiceMock<MockAllocatorFileIo>;
    NiceMock<MockAllocatorFileIo>* rebuildCtxIo = new NiceMock<MockAllocatorFileIo>;

    ContextIoManager ioManager(&info, &tp, segmentCtxIo, allocatorCtxIo, rebuildCtxIo);

    EXPECT_CALL(*segmentCtxIo, Init);
    EXPECT_CALL(*segmentCtxIo, LoadContext).WillOnce(Return(-1));

    EXPECT_CALL(info, IsUT).WillOnce(Return(false)).WillOnce(Return(true));
    ioManager.Init();
}

TEST(ContextIoManager, Dispose_testIfAllFileIsDisposed)
{
    NiceMock<MockAllocatorAddressInfo> info;
    NiceMock<MockTelemetryPublisher> tp;

    NiceMock<MockAllocatorFileIo>* segmentCtxIo = new NiceMock<MockAllocatorFileIo>;
    NiceMock<MockAllocatorFileIo>* allocatorCtxIo = new NiceMock<MockAllocatorFileIo>;
    NiceMock<MockAllocatorFileIo>* rebuildCtxIo = new NiceMock<MockAllocatorFileIo>;

    ContextIoManager ioManager(&info, &tp, segmentCtxIo, allocatorCtxIo, rebuildCtxIo);

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
    NiceMock<MockEventScheduler> scheduler;

    NiceMock<MockAllocatorFileIo>* segmentCtxIo = new NiceMock<MockAllocatorFileIo>;
    NiceMock<MockAllocatorFileIo>* allocatorCtxIo = new NiceMock<MockAllocatorFileIo>;

    ContextIoManager ioManager(&info, &tp, &scheduler, segmentCtxIo, allocatorCtxIo, nullptr);

    EXPECT_CALL(*segmentCtxIo, Flush)
        .WillOnce([&](AllocatorCtxIoCompletion completion, int dstSectionId, char* externalBuf)
        {
            completion();
            return 0;
        });
    EXPECT_CALL(*allocatorCtxIo, Flush)
        .WillOnce([&](AllocatorCtxIoCompletion completion, int dstSectionId, char* externalBuf)
        {
            completion();
            return 0;
        });

    EXPECT_CALL(*segmentCtxIo, GetNumFilesFlushing).WillRepeatedly(Return(0));
    EXPECT_CALL(*allocatorCtxIo, GetNumFilesFlushing).WillRepeatedly(Return(0));

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

    NiceMock<MockAllocatorFileIo>* segmentCtxIo = new NiceMock<MockAllocatorFileIo>;
    NiceMock<MockAllocatorFileIo>* allocatorCtxIo = new NiceMock<MockAllocatorFileIo>;
    NiceMock<MockAllocatorFileIo>* rebuildCtxIo = new NiceMock<MockAllocatorFileIo>;

    ContextIoManager ioManager(&info, &tp, nullptr, segmentCtxIo, allocatorCtxIo, rebuildCtxIo);

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

    NiceMock<MockAllocatorFileIo>* segmentCtxIo = new NiceMock<MockAllocatorFileIo>;
    NiceMock<MockAllocatorFileIo>* allocatorCtxIo = new NiceMock<MockAllocatorFileIo>;
    NiceMock<MockAllocatorFileIo>* rebuildCtxIo = new NiceMock<MockAllocatorFileIo>;

    ContextIoManager ioManager(&addrInfo, &tp, nullptr, segmentCtxIo, allocatorCtxIo, rebuildCtxIo);

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

    NiceMock<MockAllocatorFileIo>* segmentCtxIo = new NiceMock<MockAllocatorFileIo>;
    NiceMock<MockAllocatorFileIo>* allocatorCtxIo = new NiceMock<MockAllocatorFileIo>;
    NiceMock<MockAllocatorFileIo>* rebuildCtxIo = new NiceMock<MockAllocatorFileIo>;

    ContextIoManager ioManager(&info, &tp, nullptr, segmentCtxIo, allocatorCtxIo, rebuildCtxIo);

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
    EXPECT_EQ(EID(ALLOCATOR_META_ARCHIVE_FLUSH_IN_PROGRESS), ret);
}

TEST(ContextIoManager, FlushContexts_IfAsyncSuccessAllFile)
{
    // given
    NiceMock<MockAllocatorAddressInfo> info;
    NiceMock<MockTelemetryPublisher> tp;

    NiceMock<MockAllocatorFileIo>* segmentCtxIo = new NiceMock<MockAllocatorFileIo>;
    NiceMock<MockAllocatorFileIo>* allocatorCtxIo = new NiceMock<MockAllocatorFileIo>;
    NiceMock<MockAllocatorFileIo>* rebuildCtxIo = new NiceMock<MockAllocatorFileIo>;

    ContextIoManager ioManager(&info, &tp, nullptr, segmentCtxIo, allocatorCtxIo, rebuildCtxIo);

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
    ON_CALL(info, IsUT).WillByDefault(Return(true));

    NiceMock<MockAllocatorFileIo>* segmentCtxIo = new NiceMock<MockAllocatorFileIo>;
    NiceMock<MockAllocatorFileIo>* allocatorCtxIo = new NiceMock<MockAllocatorFileIo>;
    NiceMock<MockAllocatorFileIo>* rebuildCtxIo = new NiceMock<MockAllocatorFileIo>;

    ContextIoManager ioManager(&info, &tp, nullptr, segmentCtxIo, allocatorCtxIo, rebuildCtxIo);

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

    ON_CALL(info, IsUT).WillByDefault(Return(true));

    NiceMock<MockAllocatorFileIo>* segmentCtxIo = new NiceMock<MockAllocatorFileIo>;
    NiceMock<MockAllocatorFileIo>* allocatorCtxIo = new NiceMock<MockAllocatorFileIo>;
    NiceMock<MockAllocatorFileIo>* rebuildCtxIo = new NiceMock<MockAllocatorFileIo>;

    ContextIoManager ioManager(&info, &tp, nullptr, segmentCtxIo, allocatorCtxIo, rebuildCtxIo);

    EXPECT_CALL(*segmentCtxIo, Flush).WillOnce(Return(0));
    EXPECT_CALL(*allocatorCtxIo, Flush).WillOnce(Return(-1));

    EXPECT_CALL(info, IsUT).WillRepeatedly(Return(true));

    // when
    int ret = ioManager.FlushContexts(nullptr, false);

    // then
    EXPECT_LE(-1, ret);
}

TEST(ContextIoManager, GetStoredContextVersion_TestGetter)
{
    NiceMock<MockAllocatorAddressInfo> info;
    NiceMock<MockTelemetryPublisher> tp;

    NiceMock<MockAllocatorFileIo>* rebuildCtxIo = new NiceMock<MockAllocatorFileIo>;
    ContextIoManager ioManager(&info, &tp, nullptr, nullptr, nullptr, rebuildCtxIo);

    EXPECT_CALL(*rebuildCtxIo, GetStoredVersion).WillOnce(Return(10));
    EXPECT_EQ(ioManager.GetStoredContextVersion(REBUILD_CTX), 10);
}

TEST(ContextIoManager, GetContextSectionAddr_TestGetter)
{
    NiceMock<MockAllocatorAddressInfo> info;
    NiceMock<MockTelemetryPublisher> tp;

    NiceMock<MockAllocatorFileIo>* allocatorCtxIo = new NiceMock<MockAllocatorFileIo>;
    ContextIoManager ioManager(&info, &tp, nullptr, nullptr, allocatorCtxIo, nullptr);

    char buffer[100];
    EXPECT_CALL(*allocatorCtxIo, GetSectionAddr(AC_CURRENT_SSD_LSID)).WillOnce(Return(buffer));
    EXPECT_EQ(ioManager.GetContextSectionAddr(ALLOCATOR_CTX, AC_CURRENT_SSD_LSID), buffer);
}

TEST(ContextIoManager, GetContextSectionSize_TestGetter)
{
    NiceMock<MockAllocatorAddressInfo> info;
    NiceMock<MockTelemetryPublisher> tp;

    NiceMock<MockAllocatorFileIo>* segmentCtxIo = new NiceMock<MockAllocatorFileIo>;
    ContextIoManager ioManager(&info, &tp, nullptr, segmentCtxIo, nullptr, nullptr);

    EXPECT_CALL(*segmentCtxIo, GetSectionSize(SC_SEGMENT_INFO)).WillOnce(Return(1024*1024));
    EXPECT_EQ(ioManager.GetContextSectionSize(SEGMENT_CTX, SC_SEGMENT_INFO), 1024*1024);
}
} // namespace pos
