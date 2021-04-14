#include <gmock/gmock.h>

#include <list>
#include <string>
#include <vector>

#include "src/metafs/mai/metafs_management_api.h"

namespace pos
{
class MockMetaFsManagementApi : public MetaFsManagementApi
{
public:
    using MetaFsManagementApi::MetaFsManagementApi;
    MOCK_METHOD(MetaFsReturnCode<POS_EVENT_ID>, Mount, (), (override));
    MOCK_METHOD(MetaFsReturnCode<POS_EVENT_ID>, Unmount, (), (override));
    MOCK_METHOD(MetaFsReturnCode<POS_EVENT_ID>, Create, (), (override));
    MOCK_METHOD(bool, IsMounted, (), (override));
};

} // namespace pos
