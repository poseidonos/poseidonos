#include "src/journal_manager/checkpoint/log_group_releaser.h"

#include <gtest/gtest.h>

#include "test/unit-tests/allocator/i_context_manager_mock.h"
#include "test/unit-tests/event_scheduler/event_scheduler_mock.h"
#include "test/unit-tests/journal_manager/checkpoint/checkpoint_manager_mock.h"
#include "test/unit-tests/journal_manager/checkpoint/log_group_releaser_spy.h"
#include "test/unit-tests/journal_manager/config/journal_configuration_mock.h"
#include "test/unit-tests/journal_manager/log_buffer/buffer_write_done_notifier_mock.h"
#include "test/unit-tests/journal_manager/log_buffer/journal_log_buffer_mock.h"

using ::testing::_;
using ::testing::InSequence;
using ::testing::NiceMock;
using ::testing::Return;

namespace pos
{
TEST(LogGroupReleaser, Init_testIfExecutedSuccessfully)
{
    // Given
    LogGroupReleaser releaser;

    // When
    releaser.Init(nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr);

    // Then
}

TEST(LogGroupReleaser, Reset_testIfResetSuccessfully)
{
    // Given
    LogGroupReleaser releaser;

    // When
    releaser.Reset();

    // Then: Full log group should be cleared
    EXPECT_EQ(releaser.GetFullLogGroups().size(), 0);
    EXPECT_EQ(releaser.GetFlushingLogGroupId(), -1);
}

TEST(LogGroupReleaser, AddToFullLogGroup_)
{
}

TEST(LogGroupReleaser, GetFlushingLogGroupId_testIfExecutedSuccessfully)
{
    // Given
    LogGroupReleaserSpy releaser;
    int tartgetLogGroupId = 0;
    releaser.SetFlushingLogGroupId(tartgetLogGroupId);

    // When
    int actual = releaser.GetFlushingLogGroupId();

    // Then
    EXPECT_EQ(actual, tartgetLogGroupId);
}

TEST(LogGroupReleaser, GetFullLogGroups_testIfExecutedSuccessfully)
{
    // Given
    LogGroupReleaserSpy releaser;
    std::list<LogGroupInfo> logGroups;
    logGroups.push_back({0, 2});
    releaser.SetFullLogGroups(logGroups);

    // When
    auto actual = releaser.GetFullLogGroups();
    std::list<int> expect;
    for (auto it: logGroups)
    {
        expect.push_back(it.logGroupId);
    }

    // Then
    EXPECT_EQ(actual, expect);
}

TEST(LogGroupReleaser, GetStatus_testIfExecutedSuccessfully)
{
    // Given
    NiceMock<MockCheckpointManager> checkpointManager;
    LogGroupReleaser releaser;
    releaser.Init(nullptr, nullptr, nullptr, &checkpointManager, nullptr, nullptr, nullptr);

    // When
    CheckpointStatus status = INIT;
    EXPECT_CALL(checkpointManager, GetStatus).WillOnce(Return(status));

    CheckpointStatus actual = releaser.GetStatus();

    // Then
    EXPECT_EQ(actual, status);
}

TEST(LogGroupReleaser, _FlushNextLogGroup_testIfCheckpointStartedWhenThereIsNoFlushingLogGroup)
{
    // Given
    NiceMock<MockJournalConfiguration> config;
    NiceMock<MockIContextManager> contextManager;
    NiceMock<MockCheckpointManager> checkpointManager;
    NiceMock<MockEventScheduler> eventScheduler;
    LogGroupReleaserSpy releaser;

    releaser.Init(&config, nullptr, nullptr, &checkpointManager,
        nullptr, &contextManager, &eventScheduler);

    // When: There is only one full log group in the full list,
    // no flushing log group exist, and checkpoint is not in progress
    int targetLogGroup = 0;
    std::list<LogGroupInfo> logGroups;
    logGroups.push_back({0, 0});
    logGroups.push_back({1, 1});
    releaser.SetFullLogGroups(logGroups);

    releaser.SetFlushingLogGroupId(-1);
    releaser.SetCheckpointTriggerInProgress(false);

    // Then: Releaser should push event to start checkpoint
    EXPECT_CALL(eventScheduler, EnqueueEvent).Times(1);

    // When
    releaser.FlushNextLogGroup();

    // Then: Full log group list should not include log group 0,
    // and flushing log group id should be updated
    EXPECT_NE(releaser.GetFullLogGroups().front(), targetLogGroup);
    EXPECT_EQ(releaser.GetFlushingLogGroupId(), targetLogGroup);
}

TEST(LogGroupReleaser, _FlushNextLogGroup_testIfCheckpointNotStartedCheckpointIsInProgress)
{
    // Given
    NiceMock<MockJournalConfiguration> config;
    NiceMock<MockIContextManager> contextManager;
    NiceMock<MockCheckpointManager> checkpointManager;
    NiceMock<MockEventScheduler> eventScheduler;
    LogGroupReleaserSpy releaser;

    releaser.Init(&config, nullptr, nullptr, &checkpointManager,
        nullptr, &contextManager, &eventScheduler);

    // When: There is only one full log group in the full list,
    // no flushing log group exist, and checkpoint is in progress
    int fullLogGroup = 0;

    std::list<LogGroupInfo> logGroups;
    logGroups.push_back({fullLogGroup, 0});
    releaser.SetFullLogGroups(logGroups);

    releaser.SetFlushingLogGroupId(-1);
    releaser.SetCheckpointTriggerInProgress(true);

    // Then: Checkpoint event should not be inserted
    EXPECT_CALL(eventScheduler, EnqueueEvent).Times(0);

    // When
    releaser.FlushNextLogGroup();
}

TEST(LogGroupReleaser, _FlushNextLogGroup_testIfCheckpointNotStartedWhenThereIsFlushingLogGroup)
{
    // Given
    NiceMock<MockCheckpointManager> checkpointManager;
    NiceMock<MockEventScheduler> eventScheduler;
    LogGroupReleaserSpy releaser;

    releaser.Init(nullptr, nullptr, nullptr, &checkpointManager,
        nullptr, nullptr, nullptr);

    // When: There is a full log group in the full list, and is flushing log group
    int flushingLogGroup = 0;
    releaser.SetFlushingLogGroupId(flushingLogGroup);

    int fullLogGroup = 1;
    std::list<LogGroupInfo> logGroups;
    logGroups.push_back({fullLogGroup, (uint32_t)fullLogGroup});
    
    releaser.SetFullLogGroups(logGroups);

    // Then: Checkpoint event should not be inserted
    EXPECT_CALL(eventScheduler, EnqueueEvent).Times(0);

    // When
    releaser.FlushNextLogGroup();
}

TEST(LogGroupReleaser, _FlushNextLogGroup_testIfCheckpointNotStartedWhenThereIsNoFullLogGroups)
{
    // Given
    NiceMock<MockCheckpointManager> checkpointManager;
    NiceMock<MockEventScheduler> eventScheduler;
    LogGroupReleaserSpy releaser;

    releaser.Init(nullptr, nullptr, nullptr, &checkpointManager,
        nullptr, nullptr, &eventScheduler);

    // When: There is no full log group in the full list, and flushing log group
    releaser.SetFlushingLogGroupId(-1);

    std::list<LogGroupInfo> logGroups;
    releaser.SetFullLogGroups(logGroups);

    // Then: Checkpoint event should not be inserted
    EXPECT_CALL(eventScheduler, EnqueueEvent).Times(0);

    // When
    releaser.FlushNextLogGroup();
}

TEST(LogGroupReleaser, LogGroupResetCompleted_testIfNextCheckpointStarted)
{
    // Given
    NiceMock<MockJournalConfiguration> config;
    NiceMock<MockIContextManager> contextManager;
    NiceMock<MockCheckpointManager> checkpointManager;
    NiceMock<MockLogBufferWriteDoneNotifier> notifier;
    NiceMock<MockEventScheduler> eventScheduler;

    LogGroupReleaserSpy releaser;
    releaser.Init(&config, &notifier, nullptr, &checkpointManager,
        nullptr, &contextManager, &eventScheduler);

    // When: Log group 0 is flushed and log group 1 is in the full log group list
    int flushedlogGroupId = 0;
    std::list<LogGroupInfo> fullLogGroup;
    fullLogGroup.push_back({1, 1});
    releaser.SetFullLogGroups(fullLogGroup);
    releaser.SetCheckpointTriggerInProgress(false);

    // Then: Notifier should be notified and checkpoint should be started
    EXPECT_CALL(notifier, NotifyLogBufferReseted(flushedlogGroupId)).Times(1);
    EXPECT_CALL(eventScheduler, EnqueueEvent).Times(1);

    // When
    releaser.LogGroupResetCompleted(flushedlogGroupId);
}

TEST(LogGroupReleaser, LogGroupResetCompleted_testIfNextCheckpointIsNotStartedWhenThereIsNoFullLogGroup)
{
    // Given
    NiceMock<MockCheckpointManager> checkpointManager;
    NiceMock<MockLogBufferWriteDoneNotifier> notifier;
    NiceMock<MockEventScheduler> eventScheduler;

    LogGroupReleaserSpy releaser;
    releaser.Init(nullptr, &notifier, nullptr, &checkpointManager, nullptr, nullptr, nullptr);

    // When: Log group 0 is flushed and full log group list is empty
    int flushedlogGroupId = 0;
    std::list<LogGroupInfo> fullLogGroup;
    releaser.SetFullLogGroups(fullLogGroup);
    releaser.SetCheckpointTriggerInProgress(false);

    // Then: Notifier should be notified and checkpoint should not be started
    EXPECT_CALL(notifier, NotifyLogBufferReseted(flushedlogGroupId)).Times(1);
    EXPECT_CALL(eventScheduler, EnqueueEvent).Times(0);

    // When
    releaser.LogGroupResetCompleted(flushedlogGroupId);
}
} // namespace pos
