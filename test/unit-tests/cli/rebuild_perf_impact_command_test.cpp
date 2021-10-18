#include "src/cli/rebuild_perf_impact_command.h"
#include "src/cli/cli_event_code.h"

#include <gtest/gtest.h>

namespace pos_cli
{
TEST(RebuildPerfImpactCommand, RebuildPerfImpactCommand_)
{
}

TEST(RebuildPerfImpactCommand, Execute_testWhenWrongLevelGiven)
{
    // Given
    Command* cmd = new RebuildPerfImpactCommand();
    string jsonReq =
    "{\"command\":\"ADDDEVICE\""
    ",\"rid\":\"fromCLI\""
    ",\"param\":{\"level\":\"unknown_level\"}"
    "}";
    json jsonDoc = json::parse(jsonReq);
    string rid = jsonDoc["rid"].get<std::string>();

    // When
    string actual = cmd->Execute(jsonDoc, rid);

    // Then
    JsonFormat jFormat;
    string expected = jFormat.MakeResponse("REBUILDPERFIMPACT", rid,
        BADREQUEST, "This level is unsupported", GetPosInfo());

    EXPECT_EQ(expected, actual);
}

} // namespace pos_cli
