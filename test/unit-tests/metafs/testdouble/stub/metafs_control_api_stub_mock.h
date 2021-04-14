#include <gmock/gmock.h>

#include <list>
#include <string>
#include <vector>

#include "src/metafs/testdouble/stub/metafs_control_api_stub.h"

namespace pos
{
class MockMetaFsManagementApi : public MetaFsManagementApi
{
public:
    using MetaFsManagementApi::MetaFsManagementApi;
    MOCK_METHOD(MetaFsReturnCode<POS_EVENT_ID>, Mount, (), (override));
    MOCK_METHOD(MetaFsReturnCode<POS_EVENT_ID>, Unmount, (), (override));
    MOCK_METHOD(MetaFsReturnCode<POS_EVENT_ID>, CreateVolume, (), (override));
};

} // namespace pos
