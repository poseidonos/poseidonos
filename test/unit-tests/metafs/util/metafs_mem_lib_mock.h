#include <gmock/gmock.h>

#include <list>
#include <string>
#include <vector>

#include "src/metafs/util/metafs_mem_lib.h"

namespace pos
{
class MockMetaFsMemLib : public MetaFsMemLib
{
public:
    using MetaFsMemLib::MetaFsMemLib;
};

} // namespace pos
