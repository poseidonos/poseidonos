#include "src/journal_manager/checkpoint/checkpoint_manager.h"

#include <gtest/gtest.h>

#include "test/unit-tests/journal_manager/checkpoint/checkpoint_handler_mock.h"
#include "test/unit-tests/journal_manager/checkpoint/dirty_map_manager_mock.h"
#include "test/unit-tests/journal_manager/log_buffer/callback_sequence_controller_mock.h"

using ::testing::NiceMock;
using ::testing::Return;
using ::testing::InSequence;

namespace pos
{
TEST(CheckpointManager, Init_testIfInitializedSuccessfully)
{
    // Given
    NiceMock<MockCheckpointHandler>* cpHandler = new NiceMock<MockCheckpointHandler>;
    
    // Then: CheckpointHandler should be initialized
    EXPECT_CALL(*cpHandler, Init).Times(1);

    // When
    CheckpointManager cpManager(cpHandler);
    cpManager.Init(nullptr, nullptr, nullptr, nullptr, nullptr);
}

TEST(CheckpointManager, RequestCheckpoint_testIfCheckpointStarted)
{
    // Given
    NiceMock<MockCheckpointHandler>* cpHandler = new NiceMock<MockCheckpointHandler>;

    NiceMock<MockDirtyMapManager> dirtyMapManager;
    NiceMock<MockCallbackSequenceController> seqController;

    CheckpointManager cpManager(cpHandler);
    cpManager.Init(nullptr, nullptr, nullptr, &seqController, &dirtyMapManager);

    // Then
    int logGroupId = 1;
    EXPECT_CALL(dirtyMapManager, GetDirtyList(logGroupId));

    {
        InSequence s;
        EXPECT_CALL(seqController, GetCheckpointExecutionApproval).Times(1);
        EXPECT_CALL(*cpHandler, Start).Times(1);
        EXPECT_CALL(seqController, AllowCallbackExecution).Times(1);
    }    

    // When: Request checkpoint of log group 1
    cpManager.RequestCheckpoint(logGroupId, nullptr);
}

TEST(CheckpointManager, RequestCheckpoint_testIfCheckpointStartedWhenLogGroupIdIsMinusOne)
{
    // Given
    NiceMock<MockCheckpointHandler>* cpHandler = new NiceMock<MockCheckpointHandler>;

    NiceMock<MockDirtyMapManager> dirtyMapManager;
    NiceMock<MockCallbackSequenceController> seqController;

    CheckpointManager cpManager(cpHandler);
    cpManager.Init(nullptr, nullptr, nullptr, &seqController, &dirtyMapManager);

    // Then
    EXPECT_CALL(dirtyMapManager, GetTotalDirtyList);

    {
        InSequence s;
        EXPECT_CALL(seqController, GetCheckpointExecutionApproval).Times(1);
        EXPECT_CALL(*cpHandler, Start).Times(1);
        EXPECT_CALL(seqController, AllowCallbackExecution).Times(1);
    }    

    // When: Request checkpoint of log group 1
    cpManager.RequestCheckpoint(-1, nullptr);
}

TEST(CheckpointManager, GetStatus_testIfExecutedSuccessfully)
{
    // Given
    NiceMock<MockCheckpointHandler>* cpHandler = new NiceMock<MockCheckpointHandler>;

    CheckpointStatus expected = CheckpointStatus::INIT;
    EXPECT_CALL(*cpHandler, GetStatus).WillRepeatedly(Return(expected));

    // When
    CheckpointManager cpManager(cpHandler);
    CheckpointStatus actual = cpManager.GetStatus();

    // Then
    EXPECT_EQ(expected, actual);
}

} // namespace pos
