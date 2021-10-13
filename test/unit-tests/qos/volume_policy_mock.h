#include <gmock/gmock.h>

#include <list>
#include <string>
#include <vector>

#include "src/qos/volume_policy.h"

namespace pos
{
class MockVolumePolicy : public VolumePolicy
{
public:
    using VolumePolicy::VolumePolicy;
    MOCK_METHOD(void, HandlePolicy, (), (override));
};

} // namespace pos
