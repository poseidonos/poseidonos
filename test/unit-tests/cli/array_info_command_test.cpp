#include "src/cli/array_info_command.h"
#include "src/cli/cli_event_code.h"

#include <gtest/gtest.h>

namespace pos_cli
{
TEST(ArrayInfoCommand, ArrayInfoCommand_)
{
}

TEST(ArrayInfoCommand, Execute_testWhenArrayNameBlank)
{
    // Given
    Command* cmd = new ArrayInfoCommand();
    string jsonReq =
    "{\"command\":\"ARRAYINFO\""
    ",\"rid\":\"fromCLI\""
    ",\"param\":{\"name\":\"\"}"
    "}";
    json jsonDoc = json::parse(jsonReq);
    string rid = jsonDoc["rid"].get<std::string>();

    // When
    string actual = cmd->Execute(jsonDoc, rid);

    // Then
    JsonFormat jFormat;
    int event = EID(CLI_ARRAY_INFO_NO_ARRAY_NAME);
    string expected = jFormat.MakeResponse("ARRAYINFO", rid, event,
            "please type array name", GetPosInfo());

    EXPECT_EQ(expected, actual);
}

TEST(ArrayInfoCommand, Execute_testWhenNoArrayNameGiven)
{
    // Given
    Command* cmd = new ArrayInfoCommand();
    string jsonReq =
    "{\"command\":\"ARRAYINFO\""
    ",\"rid\":\"fromCLI\""
    ",\"param\":{}"
    "}";
    json jsonDoc = json::parse(jsonReq);
    string rid = jsonDoc["rid"].get<std::string>();

    // When
    string actual = cmd->Execute(jsonDoc, rid);

    // Then
    JsonFormat jFormat;
    int event = EID(CLI_ARRAY_INFO_NO_ARRAY_NAME);
    string expected = jFormat.MakeResponse("ARRAYINFO", rid, event,
            "please type array name", GetPosInfo());

    EXPECT_EQ(expected, actual);
}

} // namespace pos_cli
