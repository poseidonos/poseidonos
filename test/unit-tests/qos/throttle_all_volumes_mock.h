#include <gmock/gmock.h>

#include <list>
#include <string>
#include <vector>

#include "src/qos/throttle_all_volumes.h"

namespace pos
{
class MockAllVolumeThrottle : public AllVolumeThrottle
{
public:
    using AllVolumeThrottle::AllVolumeThrottle;
};

} // namespace pos
