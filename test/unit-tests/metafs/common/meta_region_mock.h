#include <gmock/gmock.h>

#include <list>
#include <string>
#include <vector>

#include "src/metafs/common/meta_region.h"

namespace pos
{
template<typename MetaRegionT, typename MetaContentT>
class MockMetaRegion : public MetaRegion<MetaRegionT, MetaContentT>
{
public:
    using MetaRegion::MetaRegion;
};

} // namespace pos
