#include <gmock/gmock.h>

#include <list>
#include <string>
#include <vector>

#include "src/metafs/mai/metafs_io_api.h"

namespace pos
{
class MockMetaFsIoApi : public MetaFsIoApi
{
public:
    using MetaFsIoApi::MetaFsIoApi;
    MOCK_METHOD(MetaFsReturnCode<POS_EVENT_ID>, Read, (FileDescriptorType fd, void* buf), (override));
    MOCK_METHOD(MetaFsReturnCode<POS_EVENT_ID>, Read, (FileDescriptorType fd, FileSizeType byteOffset, FileSizeType byteSize, void* buf), (override));
    MOCK_METHOD(MetaFsReturnCode<POS_EVENT_ID>, Write, (FileDescriptorType fd, void* buf), (override));
    MOCK_METHOD(MetaFsReturnCode<POS_EVENT_ID>, Write, (FileDescriptorType fd, FileSizeType byteOffset, FileSizeType byteSize, void* buf), (override));
    MOCK_METHOD(MetaFsReturnCode<POS_EVENT_ID>, SubmitIO, (MetaFsAioCbCxt * cxt), (override));
};

} // namespace pos
