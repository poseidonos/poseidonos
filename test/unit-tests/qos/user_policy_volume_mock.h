#include <gmock/gmock.h>

#include <list>
#include <string>
#include <vector>

#include "src/qos/user_policy_volume.h"

namespace pos
{
class MockVolumeUserPolicy : public VolumeUserPolicy
{
public:
    using VolumeUserPolicy::VolumeUserPolicy;
};

} // namespace pos
