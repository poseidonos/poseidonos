#include "src/journal_manager/checkpoint/checkpoint_completion.h"

#include <gtest/gtest.h>

#include "test/unit-tests/journal_manager/checkpoint/checkpoint_manager_mock.h"

using ::testing::NiceMock;

namespace pos
{
TEST(CheckpointCompletion, Execute_testIfExecutedSuccessfully)
{
    // Given
    NiceMock<MockCheckpointManager> cpManager;
    CheckpointCompletion event(&cpManager);

    EXPECT_CALL(cpManager, CheckpointCompleted);

    // When
    bool actual = event.Execute();

    // Then
    EXPECT_EQ(actual, true);
}

} // namespace pos
