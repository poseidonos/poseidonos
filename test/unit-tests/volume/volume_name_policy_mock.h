#include <gmock/gmock.h>

#include <list>
#include <string>
#include <vector>

#include "src/volume/volume_name_policy.h"

namespace pos
{
class MockVolumeNamePolicy : public VolumeNamePolicy
{
public:
    using VolumeNamePolicy::VolumeNamePolicy;
};

} // namespace pos
