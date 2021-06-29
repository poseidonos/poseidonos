#include <gmock/gmock.h>

#include <list>
#include <string>
#include <vector>

#include "src/metafs/include/mf_property.h"

namespace pos
{
class MockMetaFilePropertySet : public MetaFilePropertySet
{
public:
    using MetaFilePropertySet::MetaFilePropertySet;
};

} // namespace pos
