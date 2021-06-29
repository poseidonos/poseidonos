#include <gmock/gmock.h>

#include <list>
#include <string>
#include <vector>

#include "src/metafs/mim/mfs_io_range_overlap_chker.h"

namespace pos
{
class MockMetaFsIoRangeOverlapChker : public MetaFsIoRangeOverlapChker
{
public:
    using MetaFsIoRangeOverlapChker::MetaFsIoRangeOverlapChker;
};

} // namespace pos
