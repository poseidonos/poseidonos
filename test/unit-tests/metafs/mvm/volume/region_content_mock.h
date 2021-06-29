#include <gmock/gmock.h>

#include <list>
#include <string>
#include <vector>

#include "src/metafs/mvm/volume/region_content.h"

namespace pos
{
class MockRegionContent : public RegionContent
{
public:
    using RegionContent::RegionContent;
};

} // namespace pos
