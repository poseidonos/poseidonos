#include "src/cli/add_device_command.h"
#include "src/cli/cli_event_code.h"
#include "src/cli/command.h"
#include <string>

#include <gtest/gtest.h>

namespace pos_cli
{
TEST(AddDeviceCommand, AddDeviceCommand_)
{
}

TEST(AddDeviceCommand, Execute_testWhenNoSpareDeviceGiven)
{
    // Given
    Command* cmd = new AddDeviceCommand();
    string jsonReq =
    "{\"command\":\"ADDDEVICE\""
    ",\"rid\":\"fromCLI\""
    ",\"param\":{\"array\":\"TargetArrayName\"}"
    "}";
    json jsonDoc = json::parse(jsonReq);
    string rid = jsonDoc["rid"].get<std::string>();

    // When
    string actual = cmd->Execute(jsonDoc, rid);

    // Then
    JsonFormat jFormat;
    int event = EID(CLI_ADD_DEVICE_FAILURE_NO_DEVICE_SPECIFIED);
    string expected = jFormat.MakeResponse("ADDDEVICE", rid, event,
        "failed to add spare device", GetPosInfo());

    EXPECT_EQ(expected, actual);
}

} // namespace pos_cli
