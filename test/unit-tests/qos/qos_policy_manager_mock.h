#include <gmock/gmock.h>

#include <list>
#include <string>
#include <vector>

#include "src/qos/qos_policy_manager.h"

namespace pos
{
class MockQosPolicyManager : public QosPolicyManager
{
public:
    using QosPolicyManager::QosPolicyManager;
};

class MockQosVolumePolicyManager : public QosVolumePolicyManager
{
public:
    using QosVolumePolicyManager::QosVolumePolicyManager;
};

class MockQosEventPolicyManager : public QosEventPolicyManager
{
public:
    using QosEventPolicyManager::QosEventPolicyManager;
};

} // namespace pos
