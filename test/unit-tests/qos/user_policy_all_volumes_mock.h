#include <gmock/gmock.h>

#include <list>
#include <string>
#include <vector>

#include "src/qos/user_policy_all_volumes.h"

namespace pos
{
class MockAllVolumeUserPolicy : public AllVolumeUserPolicy
{
public:
    using AllVolumeUserPolicy::AllVolumeUserPolicy;
    MOCK_METHOD(bool, IsMinPolicyInEffect, ());
};

} // namespace pos
