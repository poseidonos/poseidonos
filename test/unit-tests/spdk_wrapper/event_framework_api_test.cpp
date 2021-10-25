#include "src/spdk_wrapper/event_framework_api.h"

#include <gtest/gtest.h>
#include <gmock/gmock.h>

#include "test/unit-tests/spdk_wrapper/caller/spdk_thread_caller_mock.h"
#include "test/unit-tests/spdk_wrapper/caller/spdk_env_caller_mock.h"

using namespace std;
using ::testing::_;
using ::testing::NiceMock;
using ::testing::Return;
using ::testing::ReturnRef;

namespace pos
{
TEST(EventFrameworkApi, SendSpdkEvent_WithEventFuncFourParams_WithInvalidCore)
{
    // Given
    uint32_t core = 257;
    EventFuncFourParams func;
    void* arg1 = nullptr;
    void* arg2 = nullptr;
    bool expected = false;

    // When: Try to send event with four params function to invalid core which exceed max reactor count
    bool actual = EventFrameworkApiSingleton::Instance()->SendSpdkEvent(core, func, arg1, arg2);

    // Then: Fail to send event
    EXPECT_EQ(actual, expected);
    EventFrameworkApiSingleton::ResetInstance();
}

void dummyfunction(void* arg1)
{
    return;
}
TEST(EventFrameworkApi, SendSpdkEvent_WithEventFuncOneParams_FailToSendMsg)
{
    // Given
    NiceMock<MockSpdkThreadCaller>* mockSpdkThreadCaller = new NiceMock<MockSpdkThreadCaller>;
    NiceMock<MockSpdkEnvCaller>* mockSpdkEnvCaller = new NiceMock<MockSpdkEnvCaller>;
    uint32_t core = 0;
    EventFuncOneParam func = dummyfunction;
    struct spdk_thread* thread[1];
    thread[0] = reinterpret_cast<struct spdk_thread*>(0x1); // to make it non-null
    bool expected = true;
    int eventCallSuccess = -1;
    EXPECT_CALL(*mockSpdkThreadCaller, GetNvmfThreadFromReactor(_)).WillOnce(Return(thread[0]));
    EXPECT_CALL(*mockSpdkThreadCaller, SpdkThreadSendMsg(_, _, _)).WillOnce(Return(eventCallSuccess));
    EXPECT_CALL(*mockSpdkEnvCaller, SpdkEnvGetCurrentCore()).WillRepeatedly(Return(core));

    // When: Try to send event with one param function
    EventFrameworkApi* eventFrameworkApi = EventFrameworkApiSingleton::Instance(mockSpdkThreadCaller, mockSpdkEnvCaller);
    bool actual = eventFrameworkApi->SendSpdkEvent(core, func, nullptr);

    eventFrameworkApi->CompleteEvents();
    EXPECT_EQ(actual, expected);
    EventFrameworkApiSingleton::ResetInstance();
}

TEST(EventFrameworkApi, GetNextReactor_Success)
{
    // Given
    NiceMock<MockSpdkThreadCaller>* mockSpdkThreadCaller = new NiceMock<MockSpdkThreadCaller>;
    NiceMock<MockSpdkEnvCaller>* mockSpdkEnvCaller = new NiceMock<MockSpdkEnvCaller>;
    uint32_t core = 0;
    uint32_t nextCore = 1;
    EXPECT_CALL(*mockSpdkEnvCaller, SpdkEnvGetCurrentCore()).WillRepeatedly(Return(core));
    EXPECT_CALL(*mockSpdkEnvCaller, SpdkEnvGetNextCore(_)).WillOnce(Return(nextCore));

    // When: Try to get next reactor of current reactor core
    EventFrameworkApi* eventFrameworkApi = EventFrameworkApiSingleton::Instance(mockSpdkThreadCaller, mockSpdkEnvCaller);
    uint32_t actual = eventFrameworkApi->GetNextReactor();

    // Then: Receive correct next core
    EXPECT_EQ(actual, nextCore);
    EventFrameworkApiSingleton::ResetInstance();
}

} // namespace pos
