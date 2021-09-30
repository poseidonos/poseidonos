#include "src/cli/stop_pos_command.h"

#include <gtest/gtest.h>

namespace pos_cli
{
TEST(StopPosCommand, StopPosCommand_)
{
    // Given : nothing

    // When
    StopPosCommand* exit = new StopPosCommand();

    // Then : nothing
}

TEST(StopPosCommand, Execute_)
{
    // Singleton Issue
}

} // namespace pos_cli
