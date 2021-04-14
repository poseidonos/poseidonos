#include <gmock/gmock.h>

#include <list>
#include <string>
#include <vector>

#include "src/metafs/mvm/volume/region_deque.h"

namespace pos
{
class MockRegionDeque : public RegionDeque
{
public:
    using RegionDeque::RegionDeque;
};

} // namespace pos
