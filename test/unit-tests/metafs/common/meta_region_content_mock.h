#include <gmock/gmock.h>

#include <list>
#include <string>
#include <vector>

#include "src/metafs/common/meta_region_content.h"

namespace pos
{
class MockMetaRegionContent : public MetaRegionContent
{
public:
    using MetaRegionContent::MetaRegionContent;
};

} // namespace pos
