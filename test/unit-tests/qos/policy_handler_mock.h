#include <gmock/gmock.h>

#include <list>
#include <string>
#include <vector>

#include "src/qos/policy_handler.h"

namespace pos
{
class MockPolicyHandler : public PolicyHandler
{
public:
    using PolicyHandler::PolicyHandler;
    MOCK_METHOD(void, HandlePolicy, (), (override));
};

} // namespace pos
