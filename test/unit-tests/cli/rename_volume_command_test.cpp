#include "src/cli/rename_volume_command.h"
#include "src/cli/cli_event_code.h"

#include <gtest/gtest.h>

namespace pos_cli
{
TEST(RenameVolumeCommand, RenameVolumeCommand_)
{
}

TEST(RenameVolumeCommand, Execute_testWhenNoNewNameGiven)
{
    // Given
    Command* cmd = new RenameVolumeCommand();
    string jsonReq =
    "{\"command\":\"RENAMEVOLUME\""
    ",\"rid\":\"fromCLI\""
    ",\"param\":{\"name\":\"TargetArrayName\"}"
    "}";
    json jsonDoc = json::parse(jsonReq);
    string rid = jsonDoc["rid"].get<std::string>();

    // When
    string actual = cmd->Execute(jsonDoc, rid);

    // Then
    JsonFormat jFormat;
    string expected = jFormat.MakeResponse(
            "RENAMEVOLUME", rid, EID(INVALID_PARAM),
            "volume name or newname is not entered", GetPosInfo());

    EXPECT_EQ(expected, actual);
}

} // namespace pos_cli
