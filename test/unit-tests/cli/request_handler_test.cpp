#include "src/cli/request_handler.h"
#include "src/cli/cli_event_code.h"

#include <gtest/gtest.h>

namespace pos_cli
{
TEST(RequestHandler, RequestHandler_)
{
}

TEST(RequestHandler, ProcessCommand_testWhenNoRidGiven)
{
    // Given
    RequestHandler* reqHandler = new RequestHandler();
    string jsonReq =
    "{\"command\":\"ADDDEVICE\""
    ",\"param\":{\"array\":\"TargetArrayName\"}"
    "}";

    // When
    string actual = reqHandler->ProcessCommand(jsonReq.c_str());

    // Then
    JsonFormat jFormat;
    string expected = jFormat.MakeResponse("ERROR_RESPONSE", "UNKNOWN", BADREQUEST,
        "wrong parameter", GetPosInfo());

    EXPECT_EQ(expected, actual);
}

TEST(RequestHandler, ProcessCommand_testWhenCommandUnknown)
{
    // Given
    RequestHandler* reqHandler = new RequestHandler();
    string jsonReq =
    "{\"command\":\"UNKNOWNCOMMAND\""
    ",\"rid\":\"fromCLI\""
    ",\"param\":{\"array\":\"TargetArrayName\"}"
    "}";
    json jsonDoc = json::parse(jsonReq);
    string command = jsonDoc["command"].get<std::string>();
    string rid = jsonDoc["rid"].get<std::string>();

    // When
    string actual = reqHandler->ProcessCommand(jsonReq.c_str());

    // Then
    JsonFormat jFormat;
    string expected = jFormat.MakeResponse(command, rid, BADREQUEST,
        "wrong command", GetPosInfo());

    EXPECT_EQ(expected, actual);
}

TEST(RequestHandler, TimedOut_test)
{
    // Given
    RequestHandler* reqHandler = new RequestHandler();
    string jsonReq =
    "{\"command\":\"LISTARRAY\""
    ",\"rid\":\"fromCLI\""
    ",\"param\":{\"array\":\"TargetArrayName\"}"
    "}";
    json jsonDoc = json::parse(jsonReq);
    string command = jsonDoc["command"].get<std::string>();
    string rid = jsonDoc["rid"].get<std::string>();

    // When
    string actual = reqHandler->TimedOut(jsonReq.c_str());

    // Then
    JsonFormat jFormat;
    string expected = jFormat.MakeResponse(command, rid, (int)POS_EVENT_ID::CLI_SERVER_TIMED_OUT, "",
        GetPosInfo());

    EXPECT_EQ(expected, actual);
}

TEST(RequestHandler, PosBusy_test)
{
    // Given
    RequestHandler* reqHandler = new RequestHandler();
    string jsonReq =
    "{\"command\":\"LISTARRAY\""
    ",\"rid\":\"fromCLI\""
    ",\"param\":{\"array\":\"TargetArrayName\"}"
    "}";
    json jsonDoc = json::parse(jsonReq);
    string command = jsonDoc["command"].get<std::string>();
    string rid = jsonDoc["rid"].get<std::string>();

    // When
    string actual = reqHandler->PosBusy(jsonReq.c_str());

    // Then
    JsonFormat jFormat;
    string expected = jFormat.MakeResponse(command, rid, (int)POS_EVENT_ID::CLI_SERVER_BUSY, "",
        GetPosInfo());

    EXPECT_EQ(expected, actual);
}

TEST(RequestHandler, IsExit_test)
{
    // Given
    RequestHandler* reqHandler = new RequestHandler();

    // When
    reqHandler->SetExit(false);
    bool actual = reqHandler->IsExit();

    // Then
    bool expected = false;

    EXPECT_EQ(expected, actual);
}

TEST(RequestHandler, SetExit_)
{
        // Given
    RequestHandler* reqHandler = new RequestHandler();

    // When
    reqHandler->SetExit(false);
    bool actual = reqHandler->IsExit();

    // Then
    bool expected = false;

    EXPECT_EQ(expected, actual);
}

} // namespace pos_cli
