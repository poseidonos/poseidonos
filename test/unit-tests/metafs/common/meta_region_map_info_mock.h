#include <gmock/gmock.h>

#include <list>
#include <string>
#include <vector>

#include "src/metafs/common/meta_region_map_info.h"

namespace pos
{
class MockMetaRegionMap : public MetaRegionMap
{
public:
    using MetaRegionMap::MetaRegionMap;
};

} // namespace pos
