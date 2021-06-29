#include <gmock/gmock.h>

#include <list>
#include <string>
#include <vector>

#include "src/metafs/config/metafs_config.h"

namespace pos
{
class MockMetaFsConfig : public MetaFsConfig
{
public:
    using MetaFsConfig::MetaFsConfig;
};

class MockMetaFsIoConfig : public MetaFsIoConfig
{
public:
    using MetaFsIoConfig::MetaFsIoConfig;
};

} // namespace pos
