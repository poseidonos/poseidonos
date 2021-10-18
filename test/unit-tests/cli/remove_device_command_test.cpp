#include "src/cli/remove_device_command.h"
#include "src/cli/cli_event_code.h"

#include <gtest/gtest.h>

namespace pos_cli
{
TEST(RemoveDeviceCommand, RemoveDeviceCommand_)
{
}

TEST(RemoveDeviceCommand, Execute_testWhenNoSpareGiven)
{
    // Given
    Command* cmd = new RemoveDeviceCommand();
    string jsonReq =
    "{\"command\":\"REMOVEDEVICE\""
    ",\"rid\":\"fromCLI\""
    ",\"param\":{\"array\":\"TargetArrayName\"}"
    "}";
    json jsonDoc = json::parse(jsonReq);
    string rid = jsonDoc["rid"].get<std::string>();

    // When
    string actual = cmd->Execute(jsonDoc, rid);

    // Then
    JsonFormat jFormat;
    string expected = jFormat.MakeResponse("REMOVEDEVICE", rid, BADREQUEST,
        "only spare device can be deleted", GetPosInfo());

    EXPECT_EQ(expected, actual);
}

} // namespace pos_cli
