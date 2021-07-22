#include <gmock/gmock.h>
#include <string>
#include <list>
#include <vector>
#include "src/gc/flow_control/flow_control_configuration.h"

namespace pos
{
class MockFlowControlConfiguration : public FlowControlConfiguration
{
public:
    using FlowControlConfiguration::FlowControlConfiguration;
    MOCK_METHOD(void, ReadConfig, (), (override));
    MOCK_METHOD(bool, GetEnable, (), (override));
    MOCK_METHOD(uint64_t, GetForceResetTimeout, (), (override));
    MOCK_METHOD(uint32_t, GetTotalToken, (), (override));
    MOCK_METHOD(uint32_t, GetTotalTokenInStripe, (), (override));
    MOCK_METHOD(FlowControlStrategy, GetFlowControlStrategy, (), (override));
    MOCK_METHOD(uint32_t, GetTargetPercent, (), (override));
    MOCK_METHOD(uint32_t, GetUrgentPercent, (), (override));
    MOCK_METHOD(uint32_t, GetTargetSegment, (), (override));
    MOCK_METHOD(uint32_t, GetUrgentSegment, (), (override));
};

} // namespace pos
