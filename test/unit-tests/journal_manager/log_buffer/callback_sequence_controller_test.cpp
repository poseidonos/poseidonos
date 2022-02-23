#include "src/journal_manager/log_buffer/callback_sequence_controller.h"

#include <gtest/gtest.h>

namespace pos
{
TEST(CallbackSequenceController, GetCallbackExecutionApproval_testIfCallbackExecutionApproved)
{
    // Given
    CallbackSequenceController test(0, false);

    // When
    test.GetCallbackExecutionApproval();

    // Then: nothing

    // Given
    CallbackSequenceController testWithPendingCallbacks(3, false);

    // When
    test.GetCallbackExecutionApproval();

    // Then: nothing
}

TEST(CallbackSequenceController, NotifyCallbackCompleted_testIfCallbackCountDecreases)
{
    // Given
    int numCallbacks = 10;
    CallbackSequenceController sequenceController(numCallbacks, false);

    // When: All callbacks are completed
    for (int i = 0; i < numCallbacks; i++)
    {
        sequenceController.NotifyCallbackCompleted();
    }

    // Then
    EXPECT_TRUE(sequenceController.GetNumPendingCallbacks() == 0);
}

TEST(CallbackSequenceController, GetCheckpointExecutionApproval_testIfCheckpointExecutionApproved)
{
    // Given
    CallbackSequenceController sequenceController(0, false);

    // When
    sequenceController.GetCheckpointExecutionApproval();

    // Then
    EXPECT_TRUE(sequenceController.IsCheckpointInProgress() == true);
}

TEST(CallbackSequenceController, AllowCallbackExecution_testIfCheckpointInProgressIsCleared)
{
    // Given
    CallbackSequenceController sequenceController(0, true);

    // When
    sequenceController.AllowCallbackExecution();

    // Then
    EXPECT_TRUE(sequenceController.IsCheckpointInProgress() == false);
}

} // namespace pos
