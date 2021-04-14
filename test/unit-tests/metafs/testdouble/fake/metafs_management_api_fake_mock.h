#include <gmock/gmock.h>

#include <list>
#include <string>
#include <vector>

#include "src/metafs/testdouble/fake/metafs_management_api_fake.h"

namespace pos
{
class MockMetaFsManagementApi : public MetaFsManagementApi
{
public:
    using MetaFsManagementApi::MetaFsManagementApi;
    MOCK_METHOD(MetaFsReturnCode<POS_EVENT_ID>, Mount, (), (override));
    MOCK_METHOD(MetaFsReturnCode<POS_EVENT_ID>, Unmount, (), (override));
    MOCK_METHOD(MetaFsReturnCode<POS_EVENT_ID>, Create, (), (override));
    MOCK_METHOD(size_t, GetFileSize, (int fd), (override));
    MOCK_METHOD(size_t, GetAlignedFileIOSize, (int fd), (override));
    MOCK_METHOD(size_t, EstimateAlignedFileIOSize, (MetaFilePropertySet & prop), (override));
    MOCK_METHOD(size_t, GetTheBiggestExtentSize, (MetaFilePropertySet & prop), (override));
};

} // namespace pos
