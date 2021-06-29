#include <gmock/gmock.h>

#include <list>
#include <string>
#include <vector>

#include "src/metafs/include/meta_storage_info.h"

namespace pos
{
class MockMetaStorageInfo : public MetaStorageInfo
{
public:
    using MetaStorageInfo::MetaStorageInfo;
};

} // namespace pos
