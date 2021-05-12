#include "src/journal_manager/checkpoint/log_group_releaser.h"

#include <gtest/gtest.h>

#include "test/unit-tests/journal_manager/checkpoint/log_group_releaser_spy.h"
#include "test/unit-tests/journal_manager/checkpoint/checkpoint_handler_mock.h"
#include "test/unit-tests/journal_manager/checkpoint/dirty_map_manager_mock.h"
#include "test/unit-tests/journal_manager/log_buffer/callback_sequence_controller_mock.h"
#include "test/unit-tests/journal_manager/log_buffer/journal_log_buffer_mock.h"

using ::testing::NiceMock;
using ::testing::_;
using ::testing::InSequence;
using ::testing::Return;

namespace pos
{
TEST(LogGroupReleaser, Init_testIfExecutedSuccessfully)
{
    // Given
    NiceMock<MockCheckpointHandler>* checkpointHandler = new NiceMock<MockCheckpointHandler>;
    LogGroupReleaser releaser(checkpointHandler);

    // Then: Checkpoint handler should be initialized
    EXPECT_CALL(*checkpointHandler, Init).Times(1);

    // When
    releaser.Init(nullptr, nullptr, nullptr, nullptr, nullptr, nullptr);
}

TEST(LogGroupReleaser, Reset_testIfResetSuccessfully)
{
    // Given
    LogGroupReleaser releaser(nullptr);

    // When
    releaser.Reset();

    // Then: Full log group should be cleared
    EXPECT_EQ(releaser.GetFullLogGroups().size(), 0);
    EXPECT_EQ(releaser.GetFlushingLogGroupId(), -1);
}

TEST(LogGroupReleaser, AddToFullLogGroup_)
{
}

TEST(LogGroupReleaser, StartCheckpoint_testIfCheckpointStartedSuccessfully)
{
    // Given
    NiceMock<MockDirtyMapManager> dirtyMapManager;
    NiceMock<MockCallbackSequenceController> sequenceController;
    NiceMock<MockCheckpointHandler>* checkpointHandler = new NiceMock<MockCheckpointHandler>;
    LogGroupReleaser logGroupReleaser(checkpointHandler);
    logGroupReleaser.Init(nullptr, nullptr, &dirtyMapManager, &sequenceController, nullptr, nullptr);

    EXPECT_CALL(dirtyMapManager, GetDirtyList);

    // Then: Checkpoint should be started after acquiring approval,
    // and allow callback execution afterwards
    {
        InSequence s;
        EXPECT_CALL(sequenceController, GetCheckpointExecutionApproval);
        EXPECT_CALL(*checkpointHandler, Start).WillOnce(Return(0));
        EXPECT_CALL(sequenceController, AllowCallbackExecution);
    }
    // When
    int result = logGroupReleaser.StartCheckpoint();

    // Then: Excussion result should be 0
    EXPECT_EQ(result, 0);
}

TEST(LogGroupReleaser, CheckpointCompleted_testIfLogBufferReseted)
{
    // Given
    NiceMock<MockJournalLogBuffer> logBuffer;
    LogGroupReleaserSpy releaser;
    releaser.Init(nullptr, &logBuffer, nullptr, nullptr, nullptr, nullptr);

    // Then
    int logGroupId = 0;
    releaser.SetFlushingLogGroupId(logGroupId);
    EXPECT_CALL(logBuffer, AsyncReset(logGroupId, _));

    // When
    releaser.CheckpointCompleted();
}

TEST(LogGroupReleaser, GetFlushingLogGroupId_)
{
}

TEST(LogGroupReleaser, GetFullLogGroups_)
{
}

TEST(LogGroupReleaser, GetStatus_)
{
}

TEST(LogGroupReleaser, _AddToFullLogGroupList_)
{
}

TEST(LogGroupReleaser, _HasFullLogGroup_)
{
}

TEST(LogGroupReleaser, _FlushNextLogGroup_)
{
}

TEST(LogGroupReleaser, _UpdateFlushingLogGroup_)
{
}

TEST(LogGroupReleaser, _PopFullLogGroup_)
{
}

TEST(LogGroupReleaser, _LogGroupResetCompleted_)
{
}

TEST(LogGroupReleaser, _ResetFlushingLogGroup_)
{
}

} // namespace pos
