#include <gmock/gmock.h>

#include <list>
#include <string>
#include <vector>

#include "src/qos/throttle_volume.h"

namespace pos
{
class MockVolumeThrottle : public VolumeThrottle
{
public:
    using VolumeThrottle::VolumeThrottle;
};

} // namespace pos
