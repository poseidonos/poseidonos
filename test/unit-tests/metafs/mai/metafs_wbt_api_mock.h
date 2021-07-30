#include <gmock/gmock.h>

#include <list>
#include <string>
#include <vector>

#include "src/metafs/mai/metafs_wbt_api.h"

namespace pos
{
class MockMetaFsWBTApi : public MetaFsWBTApi
{
public:
    using MetaFsWBTApi::MetaFsWBTApi;
    MOCK_METHOD(void, SetStatus, (bool isNormal));
};

} // namespace pos
