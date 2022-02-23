#include <gmock/gmock.h>

#include <list>
#include <string>
#include <vector>

#include "src/metafs/mim/metafs_io_multi_q.h"

namespace pos
{
class MockMetaFsIoMultiQ : public MetaFsIoMultiQ
{
public:
    using MetaFsIoMultiQ::MetaFsIoMultiQ;
};

} // namespace pos
