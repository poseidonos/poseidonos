#include <gmock/gmock.h>

#include <list>
#include <string>
#include <vector>

#include "src/spdk_wrapper/event_framework_api.h"
#include "src/spdk_wrapper/caller/spdk_thread_caller.h"

namespace pos
{
class MockEventWrapper : public EventWrapper
{
public:
    using EventWrapper::EventWrapper;
};

class MockEventFrameworkApi : public EventFrameworkApi
{
public:
    using EventFrameworkApi::EventFrameworkApi;
    MOCK_METHOD(bool, SendSpdkEvent, (uint32_t core, EventFuncTwoParams func, void* arg1, void* arg2), (override));
    MOCK_METHOD(bool, SendSpdkEvent, (uint32_t core, EventFuncFourParams func, void* arg1, void* arg2), (override));
    MOCK_METHOD(bool, SendSpdkEvent, (uint32_t core, EventFuncOneParam func, void* arg1), (override));
    MOCK_METHOD(uint32_t, GetFirstReactor, (), (override));
    MOCK_METHOD(uint32_t, GetCurrentReactor, (), (override));
    MOCK_METHOD(uint32_t, GetNextReactor, (), (override));
    MOCK_METHOD(bool, IsReactorNow, (), (override));
    MOCK_METHOD(bool, IsLastReactorNow, (), (override));
    MOCK_METHOD(bool, IsSameReactorNow, (uint32_t reactor), (override));
};

} // namespace pos
