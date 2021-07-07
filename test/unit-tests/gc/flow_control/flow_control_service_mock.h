#include <gmock/gmock.h>
#include <string>
#include <list>
#include <vector>
#include "src/gc/flow_control/flow_control_service.h"

namespace pos
{
class MockFlowControlService : public FlowControlService
{
public:
    using FlowControlService::FlowControlService;
    MOCK_METHOD(void, Register, (std::string arrayName, FlowControl* flowControl), (override));
    MOCK_METHOD(void, UnRegister, (std::string arrayName), (override));
    MOCK_METHOD(FlowControl*, GetFlowControl, (std::string arrayName), (override));
};

} // namespace pos
