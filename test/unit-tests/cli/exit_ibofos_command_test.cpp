#include "src/cli/exit_ibofos_command.h"

#include <gtest/gtest.h>

namespace pos_cli
{
TEST(ExitIbofosCommand, ExitIbofosCommand_)
{
    // Given : nothing

    // When
    ExitIbofosCommand* exit = new ExitIbofosCommand();

    // Then : nothing
}

TEST(ExitIbofosCommand, Execute_)
{
    // Singleton Issue
}

} // namespace pos_cli
