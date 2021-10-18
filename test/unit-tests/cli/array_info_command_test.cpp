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
    string expected = jFormat.MakeResponse("ARRAYINFO", rid, (int)POS_EVENT_ID::ARRAY_WRONG_NAME,
        "Please type array name", GetPosInfo());

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
    string expected = jFormat.MakeResponse("ARRAYINFO", rid, (int)POS_EVENT_ID::ARRAY_WRONG_NAME,
        "Please type array name", GetPosInfo());

    EXPECT_EQ(expected, actual);
}

} // namespace pos_cli
