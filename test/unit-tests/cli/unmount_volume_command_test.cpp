#include "src/cli/unmount_volume_command.h"
#include "src/cli/cli_event_code.h"

#include <gtest/gtest.h>

namespace pos_cli
{
TEST(UnmountVolumeCommand, UnmountVolumeCommand_)
{
}

TEST(UnmountVolumeCommand, Execute_testWhenNoVolumeGiven)
{
    // Given
    Command* cmd = new UnmountVolumeCommand();
    string jsonReq =
    "{\"command\":\"UNMOUNTVOLUME\""
    ",\"rid\":\"fromCLI\""
    ",\"param\":{\"array\":\"TargetArrayName\"}"
    "}";
    json jsonDoc = json::parse(jsonReq);
    string rid = jsonDoc["rid"].get<std::string>();

    // When
    string actual = cmd->Execute(jsonDoc, rid);

    // Then
    JsonFormat jFormat;
    string expected = jFormat.MakeResponse("UNMOUNTVOLUME", rid, BADREQUEST,
        "volume name is not entered", GetPosInfo());

    EXPECT_EQ(expected, actual);
}

} // namespace pos_cli
