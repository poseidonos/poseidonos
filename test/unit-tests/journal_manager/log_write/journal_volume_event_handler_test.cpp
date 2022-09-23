#include "src/journal_manager/log_write/journal_volume_event_handler.h"

#include <gtest/gtest.h>

#include <future>
#include <thread>

#include "test/unit-tests/allocator/i_context_manager_mock.h"
#include "test/unit-tests/event_scheduler/event_scheduler_mock.h"
#include "test/unit-tests/journal_manager/checkpoint/checkpoint_manager_mock.h"
#include "test/unit-tests/journal_manager/checkpoint/dirty_map_manager_mock.h"
#include "test/unit-tests/journal_manager/config/journal_configuration_mock.h"
#include "test/unit-tests/journal_manager/log_buffer/log_write_context_factory_mock.h"
#include "test/unit-tests/journal_manager/log_write/log_write_handler_mock.h"

using ::testing::_;
using ::testing::InSequence;
using ::testing::Invoke;
using ::testing::NiceMock;
using ::testing::Return;

namespace pos
{
TEST(JournalVolumeEventHandler, Init_testIfExecutedSuccessfully)
{
    // Given
    JournalVolumeEventHandler handler;

    // When
    handler.Init(nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr);

    // Then
}

TEST(JournalVolumeEventHandler, WriteVolumeDeletedLog_testIfLogNotWrittenWhenJournalDisabled)
{
    // Given
    NiceMock<MockJournalConfiguration> config;
    ON_CALL(config, IsEnabled).WillByDefault(Return(false));

    JournalVolumeEventHandler handler;
    handler.Init(nullptr, nullptr, nullptr, nullptr, &config, nullptr, nullptr);

    // When
    int volumeId = 0;
    int ret = handler.WriteVolumeDeletedLog(volumeId);

    // Then
    EXPECT_EQ(ret, 0);
}

TEST(JournalVolumeEventHandler, WriteVolumeDeletedLog_testIfLogWritten)
{
    // Given
    NiceMock<MockLogWriteContextFactory> factory;
    NiceMock<MockCheckpointManager> checkpointManager;
    NiceMock<MockDirtyMapManager> dirtyMapManager;
    NiceMock<MockLogWriteHandler> logWriteHandler;
    NiceMock<MockIContextManager> contextManager;
    NiceMock<MockEventScheduler> eventScheduler;
    NiceMock<MockJournalConfiguration> config;
    ON_CALL(config, IsEnabled).WillByDefault(Return(true));

    JournalVolumeEventHandler handler;
    handler.Init(&factory, &checkpointManager, &dirtyMapManager, &logWriteHandler, &config, &contextManager, &eventScheduler);

    int volumeId = 4;
    uint64_t segVersion = 10;

    // Then
    EXPECT_CALL(contextManager, GetStoredContextVersion(SEGMENT_CTX)).WillOnce(Return(segVersion));
    EXPECT_CALL(factory, CreateVolumeDeletedLogWriteContext(volumeId, segVersion, _));
    EXPECT_CALL(dirtyMapManager, DeleteDirtyList(volumeId));

    EXPECT_CALL(checkpointManager, BlockCheckpointAndWaitToBeIdle).Times(1);

    // When
    std::thread writeLogSync(&JournalVolumeEventHandler::WriteVolumeDeletedLog, &handler, volumeId);

    std::this_thread::sleep_for(1s);
    handler.VolumeDeletedLogWriteDone(volumeId);
    writeLogSync.join();
}

TEST(JournalVolumeEventHandler, TriggerMetadataFlush_testIfNotFlushedWhenJournalDisabled)
{
    // Given
    NiceMock<MockJournalConfiguration> config;
    ON_CALL(config, IsEnabled).WillByDefault(Return(false));

    JournalVolumeEventHandler handler;
    handler.Init(nullptr, nullptr, nullptr, nullptr, &config, nullptr, nullptr);

    // When
    int ret = handler.TriggerMetadataFlush();

    // Then
    EXPECT_EQ(ret, 0);
}

TEST(JournalVolumeEventHandler, TriggerMetadataFlush_testIfMetaFlushed)
{
    // Given
    NiceMock<MockCheckpointManager> checkpointManager;
    NiceMock<MockJournalConfiguration> config;
    ON_CALL(config, IsEnabled).WillByDefault(Return(true));

    JournalVolumeEventHandler handler;
    handler.Init(nullptr, &checkpointManager, nullptr, nullptr, &config, nullptr, nullptr);

    // Then
    {
        InSequence s;
        EXPECT_CALL(checkpointManager, StartCheckpoint).Times(1).WillOnce(Return(0));
        EXPECT_CALL(checkpointManager, UnblockCheckpoint).Times(1);
    }

    // When
    std::future<int> metaFlushSync = std::async(&JournalVolumeEventHandler::TriggerMetadataFlush, &handler);

    std::this_thread::sleep_for(1s);
    handler.MetaFlushed();

    EXPECT_TRUE(metaFlushSync.get() == 0);
}

TEST(JournalVolumeEventHandler, TriggerMetadataFlush_testIfFailsWhenMetaFlushFails)
{
    // Given
    NiceMock<MockCheckpointManager> checkpointManager;
    NiceMock<MockJournalConfiguration> config;
    ON_CALL(config, IsEnabled).WillByDefault(Return(true));

    JournalVolumeEventHandler handler;
    handler.Init(nullptr, &checkpointManager, nullptr, nullptr, &config, nullptr, nullptr);

    // Then
    EXPECT_CALL(checkpointManager, StartCheckpoint).Times(1).WillOnce(Return(-1));
    EXPECT_CALL(checkpointManager, UnblockCheckpoint).Times(1);

    // When
    int ret = handler.TriggerMetadataFlush();
    EXPECT_TRUE(ret < 0);
}

} // namespace pos
