#include <gmock/gmock.h>

#include <list>
#include <string>
#include <vector>

#include "src/qos/user_policy_rebuild.h"

namespace pos
{
class MockRebuildUserPolicy : public RebuildUserPolicy
{
public:
    using RebuildUserPolicy::RebuildUserPolicy;
};

} // namespace pos
