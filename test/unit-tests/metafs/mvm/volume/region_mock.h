#include <gmock/gmock.h>

#include <list>
#include <string>
#include <vector>

#include "src/metafs/mvm/volume/region.h"

namespace pos
{
class MockRegion : public Region
{
public:
    using Region::Region;
};

} // namespace pos
