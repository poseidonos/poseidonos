#include "src/journal_manager/checkpoint/checkpoint_manager.h"

#include <gtest/gtest.h>

#include "test/unit-tests/event_scheduler/event_mock.h"
#include "test/unit-tests/event_scheduler/event_scheduler_mock.h"
#include "test/unit-tests/journal_manager/checkpoint/checkpoint_handler_mock.h"
#include "test/unit-tests/journal_manager/checkpoint/dirty_map_manager_mock.h"
#include "test/unit-tests/journal_manager/log_buffer/callback_sequence_controller_mock.h"

using ::testing::InSequence;
using ::testing::NiceMock;
using ::testing::Return;

namespace pos
{
TEST(CheckpointManager, Init_testIfInitializedSuccessfully)
{
    // Given
    NiceMock<MockCheckpointHandler>* cpHandler = new NiceMock<MockCheckpointHandler>(0);

    // Then: CheckpointHandler should be initialized
    EXPECT_CALL(*cpHandler, Init).Times(1);

    // When
    CheckpointManager cpManager(cpHandler);
    cpManager.Init(nullptr, nullptr, nullptr, nullptr, nullptr);
}

TEST(CheckpointManager, RequestCheckpoint_testIfCheckpointStarted)
{
    // Given
    NiceMock<MockCheckpointHandler>* cpHandler = new NiceMock<MockCheckpointHandler>(0);

    NiceMock<MockDirtyMapManager> dirtyMapManager;

    CheckpointManager cpManager(cpHandler);
    cpManager.Init(nullptr, nullptr, nullptr, &dirtyMapManager, nullptr);

    // Then
    int logGroupId = 1;
    EXPECT_CALL(dirtyMapManager, GetDirtyList(logGroupId));

    {
        InSequence s;
        EXPECT_CALL(*cpHandler, Start).Times(1);
    }

    // When: Request checkpoint of log group 1
    cpManager.RequestCheckpoint(logGroupId, nullptr);
}

TEST(CheckpointManager, RequestCheckpoint_testIfCheckpointPended)
{
    // Given
    NiceMock<MockCheckpointHandler>* cpHandler = new NiceMock<MockCheckpointHandler>(0);

    NiceMock<MockDirtyMapManager> dirtyMapManager;

    CheckpointManager cpManager(cpHandler);
    cpManager.Init(nullptr, nullptr, nullptr, &dirtyMapManager, nullptr);

    // Then
    int logGroupId = 0;
    EXPECT_CALL(dirtyMapManager, GetDirtyList(logGroupId));

    {
        InSequence s;
        EXPECT_CALL(*cpHandler, Start).Times(1);
    }

    // When: Request checkpoint of log group 1
    cpManager.RequestCheckpoint(0, nullptr);
    EXPECT_TRUE(cpManager.IsCheckpointInProgress() == true);

    cpManager.RequestCheckpoint(1, nullptr);
    EXPECT_EQ(cpManager.GetNumPendingCheckpointRequests(), 1);

    cpManager.RequestCheckpoint(2, nullptr);
    EXPECT_EQ(cpManager.GetNumPendingCheckpointRequests(), 2);
}

TEST(CheckpointManager, RequestCheckpoint_testIfCheckpointStartedWhenLogGroupIdIsMinusOne)
{
    // Given
    NiceMock<MockCheckpointHandler>* cpHandler = new NiceMock<MockCheckpointHandler>(0);

    NiceMock<MockDirtyMapManager> dirtyMapManager;

    CheckpointManager cpManager(cpHandler);
    cpManager.Init(nullptr, nullptr, nullptr, &dirtyMapManager, nullptr);

    // Then
    EXPECT_CALL(dirtyMapManager, GetTotalDirtyList);

    {
        InSequence s;
        EXPECT_CALL(*cpHandler, Start).Times(1);
    }

    // When: Request checkpoint of log group 1
    cpManager.RequestCheckpoint(-1, nullptr);
}

TEST(CheckpointManager, GetStatus_testIfExecutedSuccessfully)
{
    // Given
    NiceMock<MockCheckpointHandler>* cpHandler = new NiceMock<MockCheckpointHandler>(0);

    CheckpointStatus expected = CheckpointStatus::INIT;
    EXPECT_CALL(*cpHandler, GetStatus).WillRepeatedly(Return(expected));

    // When
    CheckpointManager cpManager(cpHandler);
    CheckpointStatus actual = cpManager.GetStatus();

    // Then
    EXPECT_EQ(expected, actual);
}

TEST(CheckpointManager, CheckpointCompleted_testIfCallbackCompleted)
{
    // Given
    NiceMock<MockCheckpointHandler>* cpHandler = new NiceMock<MockCheckpointHandler>(0);

    NiceMock<MockEventScheduler> eventScheduler;
    NiceMock<MockDirtyMapManager> dirtyMapManager;

    EventSmartPtr firstCallback(new NiceMock<MockEvent>);
    EventSmartPtr secondCallback(new NiceMock<MockEvent>);

    CheckpointManager cpManager(cpHandler);
    cpManager.Init(nullptr, nullptr, &eventScheduler, &dirtyMapManager, nullptr);

    // When 1: first checkpoint is requested
    EXPECT_CALL(*cpHandler, Start).Times(1);
    cpManager.RequestCheckpoint(0, firstCallback);
    EXPECT_TRUE(cpManager.IsCheckpointInProgress() == true);

    // When 2: second checkpoint is requested before first completed
    cpManager.RequestCheckpoint(-1, secondCallback);
    EXPECT_TRUE(cpManager.GetNumPendingCheckpointRequests() == 1);

    // Then 1: first checkpoint should be completed and second checkpoint should be started
    EXPECT_CALL(eventScheduler, EnqueueEvent(firstCallback));
    EXPECT_CALL(*cpHandler, Start).Times(1);

    cpManager.CheckpointCompleted();

    EXPECT_CALL(eventScheduler, EnqueueEvent(secondCallback));
    cpManager.CheckpointCompleted();
}

TEST(CheckpointManager, BlockCheckpointAndWaitToBeIdle_testIfCheckpointBlocked)
{
    // Given
    NiceMock<MockCheckpointHandler>* cpHandler = new NiceMock<MockCheckpointHandler>(0);
    CheckpointManager cpManager(cpHandler);

    // When: Checkpoint blocked and another checkpoint is requested
    cpManager.BlockCheckpointAndWaitToBeIdle();
    EXPECT_TRUE(cpManager.IsCheckpointInProgress() == false);

    cpManager.RequestCheckpoint(1, nullptr);

    // Then: Checkpoint should be pended
    EXPECT_TRUE(cpManager.IsCheckpointInProgress() == false);
    EXPECT_EQ(cpManager.GetNumPendingCheckpointRequests(), 1);
}

TEST(CheckpointManager, BlockCheckpointAndWaitToBeIdle_testWaitingToBeIdle)
{
    // Given
    NiceMock<MockCheckpointHandler>* cpHandler = new NiceMock<MockCheckpointHandler>(0);

    NiceMock<MockEventScheduler> eventScheduler;
    NiceMock<MockDirtyMapManager> dirtyMapManager;

    CheckpointManager cpManager(cpHandler);
    cpManager.Init(nullptr, nullptr, &eventScheduler, &dirtyMapManager, nullptr);

    cpManager.RequestCheckpoint(1, nullptr);
    EXPECT_TRUE(cpManager.IsCheckpointInProgress() == true);

    // When: Checkpoint blocked and wait to be idle, and checkpoint completed afterward
    std::thread waitThread(&CheckpointManager::BlockCheckpointAndWaitToBeIdle, &cpManager);
    cpManager.CheckpointCompleted();

    waitThread.join();

    // Then: Checkpoint should be pended
    EXPECT_TRUE(cpManager.IsCheckpointInProgress() == false);
    EXPECT_TRUE(cpManager.IsCheckpointBlocked() == true);
}

TEST(CheckpointManager, UnblockCheckpoint_testIfCheckpointUnblocked)
{
    // Given
    NiceMock<MockCheckpointHandler>* cpHandler = new NiceMock<MockCheckpointHandler>(0);

    NiceMock<MockEventScheduler> eventScheduler;
    NiceMock<MockDirtyMapManager> dirtyMapManager;

    CheckpointManager cpManager(cpHandler);
    cpManager.Init(nullptr, nullptr, &eventScheduler, &dirtyMapManager, nullptr);

    // When 1: Checkpoint blocked and another checkpoint is requested
    cpManager.BlockCheckpointAndWaitToBeIdle();
    cpManager.RequestCheckpoint(1, nullptr);

    EXPECT_TRUE(cpManager.IsCheckpointInProgress() == false);
    EXPECT_EQ(cpManager.GetNumPendingCheckpointRequests(), 1);

    // Then 2: Pended checkpoint should be started
    EXPECT_CALL(*cpHandler, Start).Times(1);

    // When 2: Unblock checkpoint requested
    cpManager.UnblockCheckpoint();

    EXPECT_TRUE(cpManager.IsCheckpointInProgress() == true);
}
} // namespace pos
