#include <gmock/gmock.h>

#include <list>
#include <string>
#include <vector>

#include "src/metafs/include/meta_file_extent.h"

namespace pos
{
class MockMetaFileExtent : public MetaFileExtent
{
public:
    using MetaFileExtent::MetaFileExtent;
};

} // namespace pos
