#include "src/qos/parameters_all_events.h"

#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include "src/include/backend_event.h"
#include "test/unit-tests/qos/parameters_event_mock.h"

using namespace std;
using ::testing::_;
using ::testing::NiceMock;

namespace pos
{
TEST(AllEventParameter, AllEventParameter_Constructor_One_Stack)
{
    AllEventParameter allEventParam();
}

TEST(AllEventParameter, AllEventParameter_Constructor_One_Heap)
{
    AllEventParameter* allEventParam = new AllEventParameter();
    delete allEventParam;
}

TEST(AllEventParameter, Check_Reset)
{
    AllEventParameter allEventParam;
    allEventParam.Reset();
}

TEST(AllEventParameter, Test_Insert_EventExists_True)
{
    AllEventParameter allEventParam;
    BackendEvent backendEvent;
    NiceMock<MockEventParameter> eventParam;
    allEventParam.InsertEventParameter(backendEvent, eventParam);
    bool expected = true;
    bool received = allEventParam.EventExists(backendEvent);
    ASSERT_EQ(received, expected);
}

TEST(AllEventParameter, Test_Insert_EventExists_False)
{
    AllEventParameter allEventParam;
    BackendEvent backendEvent;
    bool expected = false;
    bool received = allEventParam.EventExists(backendEvent);
    ASSERT_EQ(received, expected);
}
TEST(AllEventParameter, GetEventParameter_Valid)
{
    AllEventParameter allEventParam;
    BackendEvent backendEvent;
    NiceMock<MockEventParameter> eventParam;
    allEventParam.InsertEventParameter(backendEvent, eventParam);
    EventParameter receivedEventParam = allEventParam.GetEventParameter(backendEvent);
}

TEST(AllEventParameter, GetEventParameter_Invalid)
{
    AllEventParameter allEventParam;
    BackendEvent backendEvent;
    EXPECT_THROW(allEventParam.GetEventParameter(backendEvent), int);
}
} // namespace pos
