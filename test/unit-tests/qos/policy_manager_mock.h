#include <gmock/gmock.h>

#include <list>
#include <string>
#include <vector>

#include "src/qos/policy_manager.h"

namespace pos
{
class MockQosPolicyManager : public QosPolicyManager
{
public:
    using QosPolicyManager::QosPolicyManager;
    MOCK_METHOD(void, Execute, (), (override));
    MOCK_METHOD(QosInternalManagerType, GetNextManagerType, (), (override));
};

} // namespace pos
