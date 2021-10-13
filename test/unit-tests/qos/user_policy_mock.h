#include <gmock/gmock.h>

#include <list>
#include <string>
#include <vector>

#include "src/qos/user_policy.h"

namespace pos
{
class MockQosUserPolicy : public QosUserPolicy
{
public:
    using QosUserPolicy::QosUserPolicy;
};

} // namespace pos
