#include "src/journal_manager/checkpoint/log_group_releaser.h"

#include <gtest/gtest.h>

#include "test/unit-tests/allocator/i_context_manager_mock.h"
#include "test/unit-tests/event_scheduler/event_scheduler_mock.h"
#include "test/unit-tests/journal_manager/checkpoint/checkpoint_manager_mock.h"
#include "test/unit-tests/journal_manager/config/journal_configuration_mock.h"
#include "test/unit-tests/journal_manager/log_buffer/buffer_write_done_notifier_mock.h"
#include "test/unit-tests/journal_manager/log_buffer/journal_log_buffer_mock.h"

using ::testing::_;
using ::testing::InSequence;
using ::testing::NiceMock;
using ::testing::Return;

namespace pos
{
class LogGroupReleaserSpy : public LogGroupReleaser
{
public:
    using LogGroupReleaser::LogGroupReleaser;

    void SetLogGroupReadyToCheckpoint(int logGroupId, uint32_t seqNum)
    {
        logGroups[logGroupId].Reset();
        logGroups[logGroupId].SetWaiting(seqNum);
        logGroups[logGroupId].SetReleasing();

        nextLogGroupId = logGroupId;
    }
    LogGroupFooter _CreateLogGroupFooter(int logGroupId)
    {
        return LogGroupReleaser::_CreateLogGroupFooter(logGroupId);
    }
    LogGroupFooter _CreateLogGroupFooterForReset(int logGroupId, LogGroupFooter& prevFooter)
    {
        return LogGroupReleaser::_CreateLogGroupFooterForReset(logGroupId, prevFooter);
    }
};

TEST(LogGroupReleaser, Init_testIfExecutedSuccessfully)
{
    // Given
    NiceMock<MockJournalConfiguration> config;
    NiceMock<MockIContextManager> contextManager;
    NiceMock<MockCheckpointManager> checkpointManager;
    NiceMock<MockEventScheduler> eventScheduler;
    LogGroupReleaser releaser;

    // When
    releaser.Init(&config, nullptr, nullptr, &checkpointManager,
        nullptr, &contextManager, &eventScheduler);

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
    EXPECT_EQ(releaser.GetFlushingLogGroupId(), 0);
}

TEST(LogGroupReleaser, MarkLogGroupFull_testIfFirstLogGroupCheckpointIsStarted)
{
    // Given
    NiceMock<MockJournalConfiguration> config;
    NiceMock<MockIContextManager> contextManager;
    NiceMock<MockCheckpointManager> checkpointManager;
    NiceMock<MockEventScheduler> eventScheduler;
    LogGroupReleaser releaser;

    ON_CALL(config, GetNumLogGroups).WillByDefault(Return(2));

    releaser.Init(&config, nullptr, nullptr, &checkpointManager,
        nullptr, &contextManager, &eventScheduler);

    // Then: Releaser should push event to start checkpoint
    EXPECT_CALL(eventScheduler, EnqueueEvent).Times(1);

    // When: log group 0 is full
    releaser.MarkLogGroupFull(0, 0); // Add log group 0, seq num 0
    EXPECT_EQ(releaser.GetFlushingLogGroupId(), 0);
    std::list<int> expectedFullLogGroups = {0};
    EXPECT_EQ(releaser.GetFullLogGroups(), expectedFullLogGroups);

    // When: log group 1 is full, but releasing log group 0 is not yet completed
    releaser.MarkLogGroupFull(1, 1); // Add log group 1, seq num 1
    EXPECT_EQ(releaser.GetFlushingLogGroupId(), 0);
    std::list<int> expectedFullLogGroups2 = {0, 1};
    EXPECT_EQ(releaser.GetFullLogGroups(), expectedFullLogGroups2);
}

TEST(LogGroupReleaser, MarkLogGroupFull_testIfCheckpointIsNotStartedWhenOnlySecondLogGroupIsFull)
{
    // Given
    NiceMock<MockJournalConfiguration> config;
    NiceMock<MockIContextManager> contextManager;
    NiceMock<MockCheckpointManager> checkpointManager;
    NiceMock<MockEventScheduler> eventScheduler;
    LogGroupReleaser releaser;

    ON_CALL(config, GetNumLogGroups).WillByDefault(Return(2));

    releaser.Init(&config, nullptr, nullptr, &checkpointManager,
        nullptr, &contextManager, &eventScheduler);

    // Then: Releaser should push event to start checkpoint
    EXPECT_CALL(eventScheduler, EnqueueEvent).Times(0);

    // When: log group 1 is full, before log group 0 is full
    releaser.MarkLogGroupFull(1, 1); // Add log group 1, seq num 1

    // Then: Releaser is waiting for log group 0 to be completed,
    // and log group 1 is just in the full log group list
    EXPECT_EQ(releaser.GetFlushingLogGroupId(), 0);
    std::list<int> expected = {1};
    EXPECT_EQ(releaser.GetFullLogGroups(), expected);
}

TEST(LogGroupReleaser, GetStatus_testIfExecutedSuccessfully)
{
    // Given
    NiceMock<MockJournalConfiguration> config;
    NiceMock<MockIContextManager> contextManager;
    NiceMock<MockCheckpointManager> checkpointManager;
    NiceMock<MockEventScheduler> eventScheduler;
    LogGroupReleaser releaser;

    ON_CALL(config, GetNumLogGroups).WillByDefault(Return(2));

    releaser.Init(&config, nullptr, nullptr, &checkpointManager,
        nullptr, &contextManager, &eventScheduler);

    // When
    CheckpointStatus status = INIT;
    EXPECT_CALL(checkpointManager, GetStatus).WillOnce(Return(status));

    CheckpointStatus actual = releaser.GetStatus();

    // Then
    EXPECT_EQ(actual, status);
}

TEST(LogGroupReleaser, LogGroupResetCompleted_testIfNextCheckpointStarted)
{
    // Given
    NiceMock<MockJournalConfiguration> config;
    NiceMock<MockIContextManager> contextManager;
    NiceMock<MockCheckpointManager> checkpointManager;
    NiceMock<MockLogBufferWriteDoneNotifier> notifier;
    NiceMock<MockEventScheduler> eventScheduler;

    ON_CALL(config, GetNumLogGroups).WillByDefault(Return(2));

    LogGroupReleaser releaser;
    releaser.Init(&config, &notifier, nullptr, &checkpointManager,
        nullptr, &contextManager, &eventScheduler);

    // When: Log group 0 and 1 is full, and checkpoint for log group 0 starts
    EXPECT_CALL(eventScheduler, EnqueueEvent).Times(1);

    releaser.MarkLogGroupFull(0, 10); // Add log group 0, seq num 10
    releaser.MarkLogGroupFull(1, 11); // Add log group 1, seq num 11

    // Then: Notifier should be notified and checkpoint for the next log group should be started
    EXPECT_CALL(notifier, NotifyLogBufferReseted(0)).Times(1);
    EXPECT_CALL(eventScheduler, EnqueueEvent).Times(1);

    // When
    releaser.LogGroupResetCompleted(0);

    // Then: Now flushing log group should be 1
    EXPECT_EQ(releaser.GetFlushingLogGroupId(), 1);
}

TEST(LogGroupReleaser, LogGroupResetCompleted_testIfNextCheckpointIsNotStartedWhenThereIsNoFullLogGroup)
{
    // Given
    NiceMock<MockJournalConfiguration> config;
    NiceMock<MockIContextManager> contextManager;
    NiceMock<MockCheckpointManager> checkpointManager;
    NiceMock<MockLogBufferWriteDoneNotifier> notifier;
    NiceMock<MockEventScheduler> eventScheduler;

    ON_CALL(config, GetNumLogGroups).WillByDefault(Return(2));

    LogGroupReleaser releaser;
    releaser.Init(&config, &notifier, nullptr, &checkpointManager,
        nullptr, &contextManager, &eventScheduler);

    // When: Log group 0 is full and checkpoint started
    EXPECT_CALL(eventScheduler, EnqueueEvent).Times(1);

    releaser.MarkLogGroupFull(0, 10); // Add log group 0, seq num 10

    // Then: Notifier should be notified and checkpoint for the next log group should not be started
    EXPECT_CALL(notifier, NotifyLogBufferReseted(0)).Times(1);
    EXPECT_CALL(eventScheduler, EnqueueEvent).Times(0);

    EXPECT_EQ(releaser.GetFlushingLogGroupId(), 0);

    // When: log group release completed
    releaser.LogGroupResetCompleted(0);

    // Then: Flushing log group id is updated but no full log groups exist
    EXPECT_EQ(releaser.GetFlushingLogGroupId(), 1);
    EXPECT_EQ(releaser.GetFullLogGroups().empty(), true);
}

TEST(LogGroupReleaser, CreateCheckpointSubmissionEvent_testFooterValue)
{
    // Given
    NiceMock<MockJournalConfiguration> config;
    NiceMock<MockIContextManager> contextManager;
    NiceMock<MockCheckpointManager> checkpointManager;
    NiceMock<MockLogBufferWriteDoneNotifier> notifier;
    NiceMock<MockEventScheduler> eventScheduler;

    ON_CALL(config, GetNumLogGroups).WillByDefault(Return(2));

    LogGroupReleaserSpy releaser;
    releaser.Init(&config, &notifier, nullptr, &checkpointManager,
        nullptr, &contextManager, &eventScheduler);

    int logGroupId = 0;
    EXPECT_CALL(contextManager, GetStoredContextVersion(SEGMENT_CTX)).WillOnce(Return(3)).WillOnce(Return(4));

    // When 1. log group 0 is ready to flush with seqNum = 0
    {
        uint32_t seqNum = 0;
        releaser.SetLogGroupReadyToCheckpoint(logGroupId, seqNum);

        LogGroupFooter footer = releaser._CreateLogGroupFooter(logGroupId);
        LogGroupFooter footerAfterFlush = releaser._CreateLogGroupFooterForReset(logGroupId, footer);

        // Then
        EXPECT_EQ(footer.lastCheckpointedSeginfoVersion, 3);
        EXPECT_EQ(footer.resetedSequenceNumber, UINT32_MAX);
        EXPECT_EQ(footer.isReseted, false);

        EXPECT_EQ(footerAfterFlush.lastCheckpointedSeginfoVersion, 3);
        EXPECT_EQ(footerAfterFlush.resetedSequenceNumber, seqNum);
        EXPECT_EQ(footerAfterFlush.isReseted, true);
    }

    // When 2. log group 0 is ready to flush with seqNum = 2
    {
        uint32_t seqNum = 2;
        releaser.SetLogGroupReadyToCheckpoint(logGroupId, seqNum);

        LogGroupFooter footer = releaser._CreateLogGroupFooter(logGroupId);
        LogGroupFooter footerAfterFlush = releaser._CreateLogGroupFooterForReset(logGroupId, footer);

        // Then
        EXPECT_EQ(footer.lastCheckpointedSeginfoVersion, 4);
        EXPECT_EQ(footer.resetedSequenceNumber, 0); // previous sequence number was 0
        EXPECT_EQ(footer.isReseted, false);

        EXPECT_EQ(footerAfterFlush.lastCheckpointedSeginfoVersion, 4);
        EXPECT_EQ(footerAfterFlush.resetedSequenceNumber, seqNum);
        EXPECT_EQ(footerAfterFlush.isReseted, true);
    }
}
} // namespace pos
