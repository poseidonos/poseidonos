#include <gmock/gmock.h>

#include <list>
#include <string>
#include <vector>

#include "src/metafs/testdouble/fake/metafs_io_api_fake.h"

namespace pos
{
class MockMetaFsIoApi : public MetaFsIoApi
{
public:
    using MetaFsIoApi::MetaFsIoApi;
    MOCK_METHOD(MetaFsReturnCode<POS_EVENT_ID>, Read, (uint32_t fd, void* buf), (override));
    MOCK_METHOD(MetaFsReturnCode<POS_EVENT_ID>, Read, (uint32_t fd, uint64_t byteOffset, uint64_t byteSize, void* buf), (override));
    MOCK_METHOD(MetaFsReturnCode<POS_EVENT_ID>, Write, (uint32_t fd, void* buf), (override));
    MOCK_METHOD(MetaFsReturnCode<POS_EVENT_ID>, Write, (uint32_t fd, uint64_t byteOffset, uint64_t byteSize, void* buf), (override));
    MOCK_METHOD(MetaFsReturnCode<POS_EVENT_ID>, SubmitIO, (MetaFsAioCbCxt * cxt), (override));
};

} // namespace pos
