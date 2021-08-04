#include <gmock/gmock.h>
#include <string>
#include <list>
#include <vector>
#include "src/gc/flow_control/flow_control.h"

namespace pos
{
class MockFlowControl : public FlowControl
{
public:
    using FlowControl::FlowControl;
    MOCK_METHOD(int, Init, (), (override));
    MOCK_METHOD(void, Dispose, (), (override));
    MOCK_METHOD(void, Shutdown, (), (override));
    MOCK_METHOD(void, Flush, (), (override));
    MOCK_METHOD(int, GetToken, (FlowControlType type, int token), (override));
    MOCK_METHOD(void, ReturnToken, (FlowControlType type, int token), (override));
    MOCK_METHOD(void, InitDistributer, (), (override));
};

} // namespace pos
