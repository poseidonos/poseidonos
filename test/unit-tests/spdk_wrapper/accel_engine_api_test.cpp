#include "src/spdk_wrapper/accel_engine_api.h"

#include <gtest/gtest.h>
#include <gmock/gmock.h>
#include "test/unit-tests/spdk_wrapper/event_framework_api_mock.h"

using namespace std;
using ::testing::_;
using ::testing::NiceMock;
using ::testing::Return;

namespace pos
{
TEST(IoatArgument, IoatArgument_Stack)
{
    IoatArgument ioatArgument(nullptr, nullptr, 0, nullptr, nullptr);
}

TEST(IoatArgument, IoatArgument_Heap)
{
    IoatArgument* ioatArgument = new IoatArgument(nullptr, nullptr, 0, nullptr, nullptr);
    delete ioatArgument;
}

} // namespace pos

namespace pos
{
TEST(AccelEngineApi, Finalize_Fail)
{
    NiceMock<MockEventFrameworkApi> mockEventFrameworkApi;
    EXPECT_CALL(mockEventFrameworkApi, GetFirstReactor()).WillOnce(Return(0));
    EXPECT_CALL(mockEventFrameworkApi, SendSpdkEvent(_, _, _)).WillOnce(Return(false));
    AccelEngineApi::Finalize(&mockEventFrameworkApi);
}

TEST(AccelEngineApi, IsIoatEnable_Success)
{
    bool expected = false;
    bool actual = AccelEngineApi::IsIoatEnable();
    EXPECT_EQ(expected, actual);
}

TEST(AccelEngineApi, GetIoatReactorByIndex_InvalidReactor)
{
    int expected = -1;
    int actual = AccelEngineApi::GetIoatReactorByIndex(2);
    EXPECT_EQ(expected, actual);
}

TEST(AccelEngineApi, GetReactorByIndex_InvalidReactor)
{
    int expected = -1;
    int actual = AccelEngineApi::GetReactorByIndex(3);
}

TEST(AccelEngineApi, GetIoatReactorCount_Success)
{
    uint32_t expected = 0;
    uint32_t actual = AccelEngineApi::GetIoatReactorCount();
    EXPECT_EQ(expected, actual);
}

} // namespace pos
